/**
 * RailEase — Railway Reservation System
 * Vanilla JavaScript Frontend Architecture
 * Interacts directly with the C++ REST backend
 */

// ============================================================================
// API CLIENT
// ============================================================================
const API = {
<<<<<<< HEAD
  baseUrl: window.BACKEND_API_URL || window.location.origin,
=======
  baseUrl: window.location.origin,
>>>>>>> 3d86135348a41c454c33a698403f178999cee826

  async request(endpoint, options = {}) {
    const url = `${this.baseUrl}${endpoint}`;
    const defaultHeaders = {
      "Accept": "application/json",
      "Content-Type": "application/json"
    };

    options.headers = { ...defaultHeaders, ...options.headers };

    try {
      const response = await fetch(url, options);
      const data = await response.json();
      return { ok: response.ok, status: response.status, data };
    } catch (err) {
      console.error(`API Error on ${endpoint}:`, err);
      return {
        ok: false,
        status: 0,
        data: { error: "Unable to connect to C++ backend server. Please verify the server is running." }
      };
    }
  },

  async getHealth() {
    return this.request("/api/health");
  },

  async getTrains() {
    return this.request("/api/trains");
  },

  async searchTrains(source, destination) {
    const params = new URLSearchParams();
    if (source) params.append("source", source);
    if (destination) params.append("destination", destination);
    return this.request(`/api/trains/search?${params.toString()}`);
  },

  async getTrain(trainNumber) {
    return this.request(`/api/trains/${trainNumber}`);
  },

  async getBookings() {
    return this.request("/api/bookings");
  },

  async getBooking(pnr) {
    return this.request(`/api/bookings/${pnr}`);
  },

  async bookTicket(trainNumber, passengerName) {
    return this.request("/api/bookings", {
      method: "POST",
      body: JSON.stringify({ trainNumber, passengerName })
    });
  },

  async cancelBooking(pnr) {
    return this.request(`/api/bookings/${pnr}/cancel`, {
      method: "POST"
    });
  },

  async getStats() {
    return this.request("/api/stats");
  }
};

// ============================================================================
// MAIN APPLICATION CONTROLLER
// ============================================================================
const app = {
  state: {
    currentView: "home",
    trains: [],
    bookings: [],
    stats: null,
    selectedTrainForBooking: null,
    pendingCancelPnr: null,
    bookingFilter: "ALL"
  },

  init() {
    console.log("Initializing RailEase Application...");

    // Setup default journey date to tomorrow
    const tomorrow = new Date();
    tomorrow.setDate(tomorrow.getDate() + 1);
    const dateInput = document.getElementById("home-date");
    if (dateInput) {
      dateInput.value = tomorrow.toISOString().split("T")[0];
      dateInput.min = new Date().toISOString().split("T")[0];
    }

    // Handle initial hash routing
    window.addEventListener("hashchange", () => this.handleHashChange());
    this.handleHashChange();

    // Check backend health & load initial data
    this.checkBackendHealth();
    this.loadInitialData();

    // Setup periodic health heartbeat (every 15s)
    setInterval(() => this.checkBackendHealth(), 15000);
  },

  async checkBackendHealth() {
    const statusPill = document.getElementById("server-status");
    const res = await API.getHealth();
    if (res.ok && res.data.status === "healthy") {
      statusPill.innerHTML = `
        <div class="pulse-dot"></div>
        <span>C++ Backend Online</span>
      `;
      statusPill.style.backgroundColor = "rgba(16, 185, 129, 0.15)";
      statusPill.style.color = "#34d399";
    } else {
      statusPill.innerHTML = `
        <div class="pulse-dot" style="background:#ef4444; box-shadow:0 0 8px #ef4444;"></div>
        <span>Backend Offline</span>
      `;
      statusPill.style.backgroundColor = "rgba(239, 68, 68, 0.15)";
      statusPill.style.color = "#fca5a5";
    }
  },

  async loadInitialData() {
    await this.fetchTrains();
    this.renderPopularRoutes();
    this.loadDashboard();
  },

  handleHashChange() {
    const hash = window.location.hash.replace("#", "") || "home";
    const validViews = ["home", "search", "pnr", "bookings", "dashboard"];
    if (validViews.includes(hash)) {
      this.navigate(hash, false);
    } else {
      this.navigate("home", false);
    }
  },

  navigate(viewName, updateHash = true) {
    this.state.currentView = viewName;

    // Update active nav links
    const navButtons = document.querySelectorAll(".nav-item button");
    navButtons.forEach(btn => btn.classList.remove("active"));
    const activeBtn = document.getElementById(`nav-${viewName}`);
    if (activeBtn) activeBtn.classList.add("active");

    // Toggle views
    const viewSections = document.querySelectorAll(".view-section");
    viewSections.forEach(sec => sec.style.display = "none");
    const targetSection = document.getElementById(`view-${viewName}`);
    if (targetSection) targetSection.style.display = "block";

    if (updateHash) {
      window.location.hash = `#${viewName}`;
    }

    window.scrollTo({ top: 0, behavior: "smooth" });

    // Refresh view-specific data
    if (viewName === "search") {
      this.fetchTrains();
    } else if (viewName === "bookings") {
      this.refreshBookings();
    } else if (viewName === "dashboard") {
      this.loadDashboard();
    }
  },

  // ==========================================================================
  // TRAIN MANAGEMENT & SEARCH
  // ==========================================================================
  async fetchTrains() {
    const container = document.getElementById("train-list-container");
    const res = await API.getTrains();

    if (res.ok && res.data.trains) {
      this.state.trains = res.data.trains;
      this.renderTrainList(this.state.trains);
    } else {
      if (container) {
        container.innerHTML = `
          <div class="empty-state">
            <div class="empty-icon" style="color:var(--accent-red); background:var(--accent-red-light);">
              <svg width="28" height="28" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="10"></circle><line x1="12" y1="8" x2="12" y2="12"></line><line x1="12" y1="16" x2="12.01" y2="16"></line></svg>
            </div>
            <h3 class="empty-title">Failed to load trains</h3>
            <p class="empty-desc">${res.data.error || "Could not retrieve train data from C++ server."}</p>
            <button class="btn btn-navy" onclick="app.fetchTrains()">Retry</button>
          </div>
        `;
      }
    }
  },

  renderPopularRoutes() {
    const container = document.getElementById("popular-routes-list");
    if (!container || !this.state.trains.length) return;

    container.innerHTML = this.state.trains.map(train => {
      const isAvailable = train.availableSeats > 0;
      return `
        <div class="route-card">
          <div>
            <div class="route-header">
              <span class="route-train-number">EXP #${train.trainNumber}</span>
              <span class="seats-badge ${isAvailable ? (train.availableSeats <= 2 ? 'seats-low' : 'seats-available') : 'seats-soldout'}">
                ${isAvailable ? `${train.availableSeats} Available` : 'Sold Out'}
              </span>
            </div>
            <h3 class="route-train-name">${this.escapeHtml(train.trainName)}</h3>
            
            <div class="route-path-visual">
              <div class="station-node">
                <div class="station-name">${this.escapeHtml(train.source)}</div>
                <div class="station-tag-label">Origin</div>
              </div>
              <div class="route-arrow">
                <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5"><line x1="5" y1="12" x2="19" y2="12"></line><polyline points="12 5 19 12 12 19"></polyline></svg>
              </div>
              <div class="station-node">
                <div class="station-name">${this.escapeHtml(train.destination)}</div>
                <div class="station-tag-label">Destination</div>
              </div>
            </div>
          </div>

          <div class="route-footer">
            <div>
              <div class="route-price-label">Starting Fare</div>
              <div class="route-price">Rs. ${train.fare}</div>
            </div>
            <button class="btn btn-primary" onclick="app.openBookingModal(${train.trainNumber})" ${!isAvailable ? 'disabled' : ''}>
              ${isAvailable ? 'Book Ticket' : 'Sold Out'}
            </button>
          </div>
        </div>
      `;
    }).join("");
  },

  renderTrainList(trains) {
    const container = document.getElementById("train-list-container");
    if (!container) return;

    if (!trains || trains.length === 0) {
      container.innerHTML = `
        <div class="empty-state">
          <div class="empty-icon">
            <svg width="28" height="28" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="11" cy="11" r="8"></circle><path d="m21 21-4.3-4.3"></path></svg>
          </div>
          <h3 class="empty-title">No trains found</h3>
          <p class="empty-desc">We couldn't find any trains matching your search stations. Try resetting filters to view all express connections.</p>
          <button class="btn btn-navy" onclick="app.resetTrainSearch()">Show All Trains</button>
        </div>
      `;
      return;
    }

    container.innerHTML = trains.map(train => {
      const isAvailable = train.availableSeats > 0;
      let badgeClass = "seats-available";
      let badgeText = `${train.availableSeats} of ${train.totalSeats} Seats Available`;

      if (train.availableSeats === 0) {
        badgeClass = "seats-soldout";
        badgeText = "No Seats Available (Sold Out)";
      } else if (train.availableSeats <= 2) {
        badgeClass = "seats-low";
        badgeText = `Hurry! Only ${train.availableSeats} Seats Left`;
      }

      return `
        <div class="train-card">
          <div class="train-main-info">
            <span class="train-meta-pill">TRAIN #${train.trainNumber}</span>
            <h3 class="train-title">${this.escapeHtml(train.trainName)}</h3>
            <span style="font-size:0.85rem; color:var(--text-muted);">Daily Superfast Express</span>
          </div>

          <div class="train-schedule">
            <div class="station-time-group">
              <span class="station-city">${this.escapeHtml(train.source)}</span>
              <span class="station-time">Departure</span>
            </div>
            <div class="journey-divider">
              <span>Direct</span>
              <div class="divider-line"></div>
              <span>Express</span>
            </div>
            <div class="station-time-group">
              <span class="station-city">${this.escapeHtml(train.destination)}</span>
              <span class="station-time">Arrival</span>
            </div>
          </div>

          <div class="train-seats-info">
            <span class="seats-badge ${badgeClass}">${badgeText}</span>
            <span style="font-size:0.75rem; color:var(--text-muted); margin-top:2px;">Capacity: ${train.totalSeats} Berths</span>
          </div>

          <div class="fare-block">
            <span class="fare-label">Ticket Fare</span>
            <div class="fare-amount">Rs. ${train.fare}</div>
            <button class="btn btn-primary" style="margin-top: 6px;" onclick="app.openBookingModal(${train.trainNumber})" ${!isAvailable ? 'disabled' : ''}>
              ${isAvailable ? 'Book Ticket ➔' : 'Sold Out'}
            </button>
          </div>
        </div>
      `;
    }).join("");
  },

  handleHomeSearch(e) {
    e.preventDefault();
    const src = document.getElementById("home-source").value.trim();
    const dest = document.getElementById("home-destination").value.trim();

    this.navigate("search");

    document.getElementById("filter-source").value = src;
    document.getElementById("filter-destination").value = dest;

    this.filterTrainsLive();
  },

  swapHomeStations() {
    const srcInput = document.getElementById("home-source");
    const destInput = document.getElementById("home-destination");
    const temp = srcInput.value;
    srcInput.value = destInput.value;
    destInput.value = temp;
  },

  async filterTrainsLive() {
    const src = document.getElementById("filter-source").value.trim();
    const dest = document.getElementById("filter-destination").value.trim();

    if (!src && !dest) {
      this.renderTrainList(this.state.trains);
      return;
    }

    const res = await API.searchTrains(src, dest);
    if (res.ok && res.data.trains) {
      this.renderTrainList(res.data.trains);
    }
  },

  resetTrainSearch() {
    document.getElementById("filter-source").value = "";
    document.getElementById("filter-destination").value = "";
    this.renderTrainList(this.state.trains);
  },

  // ==========================================================================
  // BOOKING MODAL & TICKET SUBMISSION
  // ==========================================================================
  openBookingModal(trainNumber) {
    const train = this.state.trains.find(t => t.trainNumber === trainNumber);
    if (!train) {
      this.showToast("Error", "Train details not found", "error");
      return;
    }

    if (train.availableSeats <= 0) {
      this.showToast("Unavailable", "No seats remaining on this train", "warning");
      return;
    }

    this.state.selectedTrainForBooking = train;

    document.getElementById("modal-train-number").textContent = `TRAIN #${train.trainNumber}`;
    document.getElementById("modal-train-name").textContent = train.trainName;
    document.getElementById("modal-train-route").textContent = `${train.source} ➔ ${train.destination}`;
    document.getElementById("modal-train-fare").textContent = `Rs. ${train.fare}`;
    document.getElementById("modal-train-seats").textContent = `${train.availableSeats} of ${train.totalSeats} Remaining`;

    const nameInput = document.getElementById("booking-passenger-name");
    nameInput.value = "";
    document.getElementById("booking-error-msg").style.display = "none";

    document.getElementById("btn-confirm-booking").disabled = false;
    document.getElementById("btn-booking-text").style.display = "inline";
    document.getElementById("btn-booking-spinner").style.display = "none";

    document.getElementById("booking-modal").classList.add("active");
    setTimeout(() => nameInput.focus(), 100);
  },

  closeBookingModal() {
    document.getElementById("booking-modal").classList.remove("active");
    this.state.selectedTrainForBooking = null;
  },

  async handleBookingSubmit(e) {
    e.preventDefault();
    if (!this.state.selectedTrainForBooking) return;

    const passengerName = document.getElementById("booking-passenger-name").value.trim();
    const errorEl = document.getElementById("booking-error-msg");
    const submitBtn = document.getElementById("btn-confirm-booking");
    const btnText = document.getElementById("btn-booking-text");
    const btnSpinner = document.getElementById("btn-booking-spinner");

    // Client-side validation
    if (!passengerName || passengerName.length < 2) {
      errorEl.textContent = "Please enter a valid passenger name (minimum 2 characters).";
      errorEl.style.display = "block";
      return;
    }
    errorEl.style.display = "none";

    // Prevent duplicate submissions: disable button & show spinner
    submitBtn.disabled = true;
    btnText.style.display = "none";
    btnSpinner.style.display = "inline-block";

    const res = await API.bookTicket(this.state.selectedTrainForBooking.trainNumber, passengerName);

    submitBtn.disabled = false;
    btnText.style.display = "inline";
    btnSpinner.style.display = "none";

    if (res.ok && res.data.success) {
      this.closeBookingModal();
      this.showToast("Booking Confirmed!", `PNR: ${res.data.pnr} booked for ${passengerName}`, "success");

      // Refresh train counts and dashboard
      await this.fetchTrains();
      this.renderPopularRoutes();
      this.loadDashboard();

      // Show Voucher Modal
      this.showVoucherModal(res.data.booking);
    } else {
      errorEl.textContent = res.data.error || "Booking failed. Please try again.";
      errorEl.style.display = "block";
      this.showToast("Booking Failed", res.data.error || "Could not complete booking", "error");
    }
  },

  showVoucherModal(booking) {
    const slot = document.getElementById("voucher-ticket-slot");
    if (!slot) return;

    slot.innerHTML = `
      <div class="ticket-voucher" style="box-shadow: none; border: 1.5px solid var(--border-light);">
        <div class="ticket-header">
          <div class="ticket-pnr-box">
            <span class="ticket-pnr-label">Booking Reference</span>
            <span class="ticket-pnr-number">PNR ${booking.pnr}</span>
          </div>
          <span class="status-badge status-confirmed">CONFIRMED</span>
        </div>

        <div class="ticket-body">
          <div class="ticket-grid">
            <div class="ticket-item">
              <span class="ticket-label">Passenger Legal Name</span>
              <span class="ticket-value">${this.escapeHtml(booking.passengerName)}</span>
            </div>
            <div class="ticket-item">
              <span class="ticket-label">Express Train Details</span>
              <span class="ticket-value">${this.escapeHtml(booking.trainName)} (#${booking.trainNumber})</span>
            </div>
            <div class="ticket-item">
              <span class="ticket-label">Fare Charged</span>
              <span class="ticket-value" style="color:var(--accent-red);">Rs. ${booking.fare}</span>
            </div>
            <div class="ticket-item">
              <span class="ticket-label">Booking Status</span>
              <span class="ticket-value" style="color:var(--success-green); font-weight:800;">ACTIVE & VALID</span>
            </div>
          </div>
        </div>

        <div class="ticket-perforation">
          <div class="ticket-cutout-left"></div>
          <div class="ticket-dashed-line"></div>
          <div class="ticket-cutout-right"></div>
        </div>

        <div class="ticket-footer">
          <span style="font-size:0.8rem; color:var(--text-muted);">
            Stored in passengers.txt &bull; Verified by C++ Engine
          </span>
          <span style="font-size:0.85rem; font-weight:700; color:var(--primary-navy);">
            RailEase Express
          </span>
        </div>
      </div>
    `;

    document.getElementById("voucher-modal").classList.add("active");
  },

  closeVoucherModal() {
    document.getElementById("voucher-modal").classList.remove("active");
  },

  // ==========================================================================
  // PNR STATUS QUERY
  // ==========================================================================
  async handlePnrSearch(e) {
    e.preventDefault();
    const pnr = document.getElementById("pnr-input").value.trim();
    if (!pnr) return;

    const btn = document.getElementById("btn-search-pnr");
    btn.disabled = true;
    btn.textContent = "Checking...";

    const res = await API.getBooking(pnr);

    btn.disabled = false;
    btn.textContent = "Get Status";

    const container = document.getElementById("pnr-result-container");
    if (!container) return;

    if (res.ok && res.data.booking) {
      const b = res.data.booking;
      const isConfirmed = b.status === "CONFIRMED";

      container.innerHTML = `
        <div class="ticket-voucher">
          <div class="ticket-header">
            <div class="ticket-pnr-box">
              <span class="ticket-pnr-label">Reference Number</span>
              <span class="ticket-pnr-number">PNR: ${b.pnr}</span>
            </div>
            <span class="status-badge ${isConfirmed ? 'status-confirmed' : 'status-cancelled'}">
              ${b.status}
            </span>
          </div>

          <div class="ticket-body">
            <div class="ticket-grid">
              <div class="ticket-item">
                <span class="ticket-label">Passenger Name</span>
                <span class="ticket-value">${this.escapeHtml(b.passengerName)}</span>
              </div>
              <div class="ticket-item">
                <span class="ticket-label">Train Service</span>
                <span class="ticket-value">${this.escapeHtml(b.trainName)}</span>
              </div>
              <div class="ticket-item">
                <span class="ticket-label">Train Number</span>
                <span class="ticket-value">#${b.trainNumber}</span>
              </div>
              <div class="ticket-item">
                <span class="ticket-label">Total Fare</span>
                <span class="ticket-value" style="color:var(--accent-red);">Rs. ${b.fare}</span>
              </div>
            </div>
          </div>

          <div class="ticket-perforation">
            <div class="ticket-cutout-left"></div>
            <div class="ticket-dashed-line"></div>
            <div class="ticket-cutout-right"></div>
          </div>

          <div class="ticket-footer">
            <span style="font-size:0.8rem; color:var(--text-muted);">
              ${isConfirmed ? 'Ticket is confirmed for journey' : 'This reservation was cancelled and seat refunded'}
            </span>
            ${isConfirmed ? `
              <button class="btn btn-danger-outline" onclick="app.openCancelModal(${b.pnr}, '${this.escapeHtml(b.passengerName)}', '${this.escapeHtml(b.trainName)}')">
                Cancel Ticket
              </button>
            ` : `
              <button class="btn btn-navy" onclick="app.navigate('search')">
                Book Again
              </button>
            `}
          </div>
        </div>
      `;
    } else {
      container.innerHTML = `
        <div class="empty-state" style="max-width: 600px; margin: 0 auto; border-color: var(--accent-red-border);">
          <div class="empty-icon" style="background:var(--accent-red-light); color:var(--accent-red);">
            <svg width="28" height="28" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="10"></circle><line x1="15" y1="9" x2="9" y2="15"></line><line x1="9" y1="9" x2="15" y2="15"></line></svg>
          </div>
          <h3 class="empty-title">PNR Not Found</h3>
          <p class="empty-desc">${res.data.error || `No booking found for PNR reference ${pnr}. Please check the number and try again.`}</p>
          <button class="btn btn-navy" onclick="document.getElementById('pnr-input').focus()">Try Another PNR</button>
        </div>
      `;
    }
  },

  // ==========================================================================
  // MY BOOKINGS & CANCELLATION
  // ==========================================================================
  async refreshBookings() {
    const res = await API.getBookings();
    if (res.ok && res.data.bookings) {
      this.state.bookings = res.data.bookings;
      this.renderBookingsTable();
    }
  },

  filterBookingsTab(filterType) {
    this.state.bookingFilter = filterType;
    document.querySelectorAll(".active-tab-btn, #filter-booking-all, #filter-booking-active, #filter-booking-cancelled").forEach(btn => {
      btn.className = "btn btn-outline";
    });

    if (filterType === "ALL") {
      document.getElementById("filter-booking-all").className = "btn btn-navy active-tab-btn";
    } else if (filterType === "CONFIRMED") {
      document.getElementById("filter-booking-active").className = "btn btn-navy active-tab-btn";
    } else {
      document.getElementById("filter-booking-cancelled").className = "btn btn-navy active-tab-btn";
    }

    this.renderBookingsTable();
  },

  filterBookingsTable() {
    this.renderBookingsTable();
  },

  renderBookingsTable() {
    const tbody = document.getElementById("bookings-table-body");
    if (!tbody) return;

    const searchTerm = (document.getElementById("bookings-search-box")?.value || "").toLowerCase().trim();

    let list = this.state.bookings;

    // Apply status filter
    if (this.state.bookingFilter !== "ALL") {
      list = list.filter(b => b.status === this.state.bookingFilter);
    }

    // Apply search filter
    if (searchTerm) {
      list = list.filter(b => 
        String(b.pnr).includes(searchTerm) ||
        b.passengerName.toLowerCase().includes(searchTerm) ||
        b.trainName.toLowerCase().includes(searchTerm)
      );
    }

    if (!list.length) {
      tbody.innerHTML = `
        <tr>
          <td colspan="7" style="text-align: center; padding: 3rem 1rem;">
            <div style="color:var(--text-muted); font-size: 1rem;">No matching reservations found.</div>
            <button class="btn btn-navy" style="margin-top: 1rem;" onclick="app.navigate('search')">Find & Book Trains</button>
          </td>
        </tr>
      `;
      return;
    }

    tbody.innerHTML = list.map(b => {
      const isConfirmed = b.status === "CONFIRMED";
      return `
        <tr>
          <td><strong style="letter-spacing: 0.5px;">#${b.pnr}</strong></td>
          <td><strong style="color:var(--primary-navy);">${this.escapeHtml(b.passengerName)}</strong></td>
          <td>#${b.trainNumber}</td>
          <td>${this.escapeHtml(b.trainName)}</td>
          <td><strong style="color:var(--accent-red);">Rs. ${b.fare}</strong></td>
          <td>
            <span class="status-badge ${isConfirmed ? 'status-confirmed' : 'status-cancelled'}" style="font-size:0.75rem; padding:3px 10px;">
              ${b.status}
            </span>
          </td>
          <td style="text-align: right;">
            ${isConfirmed ? `
              <button class="btn btn-danger-outline" style="padding: 6px 14px; font-size: 0.85rem;" onclick="app.openCancelModal(${b.pnr}, '${this.escapeHtml(b.passengerName)}', '${this.escapeHtml(b.trainName)}')">
                Cancel
              </button>
            ` : `
              <span style="font-size:0.85rem; color:var(--text-muted);">Seat Restored</span>
            `}
          </td>
        </tr>
      `;
    }).join("");
  },

  openCancelModal(pnr, passengerName, trainName) {
    this.state.pendingCancelPnr = pnr;
    document.getElementById("cancel-modal-pnr").textContent = `PNR: ${pnr}`;
    document.getElementById("cancel-modal-passenger").textContent = `Passenger: ${passengerName}`;
    document.getElementById("cancel-modal-train").textContent = `Train: ${trainName}`;

    document.getElementById("btn-confirm-cancel").disabled = false;
    document.getElementById("btn-cancel-text").style.display = "inline";
    document.getElementById("btn-cancel-spinner").style.display = "none";

    document.getElementById("cancel-modal").classList.add("active");
  },

  closeCancelModal() {
    document.getElementById("cancel-modal").classList.remove("active");
    this.state.pendingCancelPnr = null;
  },

  async executeCancel() {
    const pnr = this.state.pendingCancelPnr;
    if (!pnr) return;

    const btn = document.getElementById("btn-confirm-cancel");
    const btnText = document.getElementById("btn-cancel-text");
    const btnSpinner = document.getElementById("btn-cancel-spinner");

    btn.disabled = true;
    btnText.style.display = "none";
    btnSpinner.style.display = "inline-block";

    const res = await API.cancelBooking(pnr);

    btn.disabled = false;
    btnText.style.display = "inline";
    btnSpinner.style.display = "none";

    if (res.ok && res.data.success) {
      this.closeCancelModal();
      this.showToast("Cancellation Successful", `PNR ${pnr} cancelled. Available seats restored.`, "success");

      // Refresh all views
      await this.fetchTrains();
      this.renderPopularRoutes();
      this.refreshBookings();
      this.loadDashboard();

      // If on PNR status view, refresh the card
      if (this.state.currentView === "pnr") {
        document.getElementById("pnr-input").value = pnr;
        this.handlePnrSearch(new Event("submit"));
      }
    } else {
      this.showToast("Cancellation Failed", res.data.error || "Could not cancel reservation.", "error");
    }
  },

  // ==========================================================================
  // DASHBOARD ANALYTICS
  // ==========================================================================
  async loadDashboard() {
    const res = await API.getStats();
    if (res.ok && res.data) {
      const s = res.data;
      this.state.stats = s;

      document.getElementById("stat-total-trains").textContent = s.totalTrains;
      document.getElementById("stat-available-seats").textContent = s.availableSeats;
      document.getElementById("stat-total-seats-desc").textContent = `Out of ${s.totalSeats} total seats`;
      document.getElementById("stat-total-reservations").textContent = s.totalReservations;
      document.getElementById("stat-confirmed-bookings").textContent = s.confirmedBookings;
      document.getElementById("stat-cancelled-bookings").textContent = s.cancelledBookings;

      // Render Fleet Table
      const fleetBody = document.getElementById("dashboard-fleet-body");
      if (fleetBody && this.state.trains.length) {
        fleetBody.innerHTML = this.state.trains.map(t => {
          const booked = t.totalSeats - t.availableSeats;
          const percent = t.totalSeats > 0 ? Math.round((booked / t.totalSeats) * 100) : 0;
          return `
            <tr>
              <td><strong>#${t.trainNumber}</strong></td>
              <td><strong>${this.escapeHtml(t.trainName)}</strong></td>
              <td>${this.escapeHtml(t.source)} ➔ ${this.escapeHtml(t.destination)}</td>
              <td>${t.availableSeats} / ${t.totalSeats} Available</td>
              <td>
                <div style="display:flex; align-items:center; gap:8px;">
                  <div style="flex:1; max-width:120px; height:8px; background:var(--bg-subtle); border-radius:4px; overflow:hidden;">
                    <div style="width:${percent}%; height:100%; background:${percent >= 100 ? 'var(--accent-red)' : (percent > 60 ? 'var(--warning-amber)' : 'var(--primary-blue)')};"></div>
                  </div>
                  <span style="font-size:0.8rem; font-weight:700;">${percent}%</span>
                </div>
              </td>
              <td><strong style="color:var(--accent-red);">Rs. ${t.fare}</strong></td>
              <td>
                <button class="btn btn-outline" style="padding:4px 10px; font-size:0.8rem;" onclick="app.openBookingModal(${t.trainNumber})" ${t.availableSeats <= 0 ? 'disabled' : ''}>
                  Book
                </button>
              </td>
            </tr>
          `;
        }).join("");
      }

      // Render Recent Reservations Table
      const recentBody = document.getElementById("dashboard-recent-body");
      if (recentBody) {
        if (!s.recentReservations || !s.recentReservations.length) {
          recentBody.innerHTML = `<tr><td colspan="5" style="text-align:center; color:var(--text-muted); padding:1.5rem;">No recent bookings recorded.</td></tr>`;
        } else {
          recentBody.innerHTML = s.recentReservations.map(r => `
            <tr>
              <td><strong>#${r.pnr}</strong></td>
              <td>${this.escapeHtml(r.passengerName)}</td>
              <td>${this.escapeHtml(r.trainName)} (#${r.trainNumber})</td>
              <td><strong style="color:var(--accent-red);">Rs. ${r.fare}</strong></td>
              <td>
                <span class="status-badge ${r.status === 'CONFIRMED' ? 'status-confirmed' : 'status-cancelled'}" style="font-size:0.75rem; padding:3px 8px;">
                  ${r.status}
                </span>
              </td>
            </tr>
          `).join("");
        }
      }
    }
  },

  // ==========================================================================
  // TOAST NOTIFICATIONS & UTILITIES
  // ==========================================================================
  showToast(title, message, type = "info") {
    const container = document.getElementById("toast-container");
    if (!container) return;

    const toast = document.createElement("div");
    toast.className = `toast toast-${type}`;
    toast.innerHTML = `
      <div class="toast-content">
        <div class="toast-title">${this.escapeHtml(title)}</div>
        <div class="toast-message">${this.escapeHtml(message)}</div>
      </div>
    `;

    container.appendChild(toast);

    setTimeout(() => {
      toast.style.opacity = "0";
      toast.style.transform = "translateX(100%)";
      setTimeout(() => toast.remove(), 250);
    }, 4000);
  },

  escapeHtml(str) {
    if (!str) return "";
    return String(str)
      .replace(/&/g, "&amp;")
      .replace(/</g, "&lt;")
      .replace(/>/g, "&gt;")
      .replace(/"/g, "&quot;")
      .replace(/'/g, "&#039;");
  }
};

// Start application when DOM is ready
document.addEventListener("DOMContentLoaded", () => app.init());
