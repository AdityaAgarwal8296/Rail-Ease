#include <iostream>
#include <string>
#include <fstream>
using namespace std;

// Abstract Base Class
class User {
protected:
    string name;

public:
    User(string n) {
        name = n;
    }

    virtual void displayRole() = 0;

    virtual ~User() {}
};

// Train Class
class Train {
private:
    int trainNumber;
    string trainName;
    string source;
    string destination;
    int totalSeats;
    int availableSeats;
    int fare;

public:
    Train() {
        trainNumber = 0;
        totalSeats = 0;
        availableSeats = 0;
        fare = 0;
    }

    Train(int number, string name, string src,
          string dest, int seats, int price) {
        trainNumber = number;
        trainName = name;
        source = src;
        destination = dest;
        totalSeats = seats;
        availableSeats = seats;
        fare = price;
    }

    int getTrainNumber() {
        return trainNumber;
    }

    string getTrainName() {
        return trainName;
    }

    string getSource() {
        return source;
    }

    string getDestination() {
        return destination;
    }

    int getAvailableSeats() {
        return availableSeats;
    }

    int getFare() {
        return fare;
    }

    int getTotalSeats() {
        return totalSeats;
    }

    void setCapacity(int newTotalSeats, int bookedSeats) {
        totalSeats = newTotalSeats;
        availableSeats = totalSeats - bookedSeats;
        if (availableSeats < 0) {
            availableSeats = 0;
        }
    }

    bool bookSeat() {
        if (availableSeats > 0) {
            availableSeats--;
            return true;
        }
        return false;
    }

    void cancelSeat() {
        if (availableSeats < totalSeats) {
            availableSeats++;
        }
    }

    void display() {
        cout << "\nTrain Number: " << trainNumber;
        cout << "\nTrain Name: " << trainName;
        cout << "\nSource: " << source;
        cout << "\nDestination: " << destination;
        cout << "\nAvailable Seats: " << availableSeats;
        cout << "\nFare: Rs. " << fare << "\n";
    }

    void save(ofstream& file) {
        file << trainNumber << '\n'
             << trainName << '\n'
             << source << '\n'
             << destination << '\n'
             << totalSeats << '\n'
             << availableSeats << '\n'
             << fare << '\n';
    }

    bool load(ifstream& file) {
        if (!(file >> trainNumber))
            return false;

        file.ignore(1000, '\n');
        getline(file, trainName);
        getline(file, source);
        getline(file, destination);

        file >> totalSeats;
        file >> availableSeats;
        file >> fare;
        file.ignore(1000, '\n');

        return true;
    }
};

// Reservation Class
class Reservation {
private:
    int pnr;
    string passengerName;
    int trainNumber;
    string trainName;
    int fare;
    bool active;

public:
    Reservation() {
        pnr = 0;
        trainNumber = 0;
        fare = 0;
        active = false;
    }

    void createReservation(int id, string passenger,
                           int number, string name, int price) {
        pnr = id;
        passengerName = passenger;
        trainNumber = number;
        trainName = name;
        fare = price;
        active = true;
    }

    int getPNR() {
        return pnr;
    }

    int getTrainNumber() {
        return trainNumber;
    }

    bool isActive() {
        return active;
    }

    void cancel() {
        active = false;
    }

    string getPassengerName() {
        return passengerName;
    }

    string getTrainName() {
        return trainName;
    }

    int getFare() {
        return fare;
    }

    void display() {
        cout << "\nPNR: " << pnr;
        cout << "\nPassenger Name: " << passengerName;
        cout << "\nTrain Number: " << trainNumber;
        cout << "\nTrain Name: " << trainName;
        cout << "\nFare: Rs. " << fare;
        cout << "\nStatus: "
             << (active ? "CONFIRMED" : "CANCELLED")
             << "\n";
    }

    void save(ofstream& file) {
        file << pnr << '\n'
             << passengerName << '\n'
             << trainNumber << '\n'
             << trainName << '\n'
             << fare << '\n'
             << active << '\n';
    }

    bool load(ifstream& file) {
        if (!(file >> pnr))
            return false;

        file.ignore(1000, '\n');
        getline(file, passengerName);

        file >> trainNumber;
        file.ignore(1000, '\n');
        getline(file, trainName);

        file >> fare;
        file >> active;
        file.ignore(1000, '\n');

        return true;
    }
};

// Railway System
class RailwaySystem {
private:
    Train trains[100];
    Reservation reservations[100];

    int trainCount;
    int reservationCount;
    int nextPNR;

    Train* findTrain(int number) {
        for (int i = 0; i < trainCount; i++) {
            if (trains[i].getTrainNumber() == number)
                return &trains[i];
        }
        return NULL;
    }

    void saveTrains() {
        ofstream file("trains.txt");

        if (!file) {
            cout << "Error saving train details!\n";
            return;
        }

        file << trainCount << '\n';

        for (int i = 0; i < trainCount; i++)
            trains[i].save(file);

        file.close();
    }

    void saveReservations() {
        ofstream file("passengers.txt");

        if (!file) {
            cout << "Error saving passenger details!\n";
            return;
        }

        file << reservationCount << '\n';

        for (int i = 0; i < reservationCount; i++)
            reservations[i].save(file);

        file.close();
    }

    void loadTrains() {
        ifstream file("trains.txt");

        if (!file) {
            trainCount = 3;

            trains[0] = Train(12001, "Shatabdi Express",
                              "Delhi", "Agra", 3, 750);

            trains[1] = Train(12002, "Rajdhani Express",
                              "Mumbai", "Delhi", 2, 1800);

            trains[2] = Train(12003, "Intercity Express",
                              "Kanpur", "Lucknow", 4, 350);

            saveTrains();
            return;
        }

        int count;

        if (!(file >> count) || count < 0 || count > 100) {
            file.close();
            cout << "Invalid train file. Loading defaults.\n";

            trainCount = 3;

            trains[0] = Train(12001, "Shatabdi Express",
                              "Delhi", "Agra", 3, 750);

            trains[1] = Train(12002, "Rajdhani Express",
                              "Mumbai", "Delhi", 2, 1800);

            trains[2] = Train(12003, "Intercity Express",
                              "Kanpur", "Lucknow", 4, 350);

            saveTrains();
            return;
        }

        file.ignore(1000, '\n');
        trainCount = 0;

        for (int i = 0; i < count; i++) {
            if (trains[i].load(file))
                trainCount++;
            else
                break;
        }

        file.close();
    }

    void loadReservations() {
        ifstream file("passengers.txt");

        reservationCount = 0;
        nextPNR = 1001;

        if (!file)
            return;

        int count;

        if (!(file >> count) || count < 0 || count > 100) {
            file.close();
            cout << "Invalid passenger file.\n";
            return;
        }

        file.ignore(1000, '\n');

        for (int i = 0; i < count; i++) {
            if (reservations[i].load(file)) {
                reservationCount++;

                if (reservations[i].getPNR() >= nextPNR)
                    nextPNR = reservations[i].getPNR() + 1;
            } else {
                break;
            }
        }

        file.close();
    }

public:
    RailwaySystem() {
        trainCount = 0;
        reservationCount = 0;
        nextPNR = 1001;

        loadTrains();
        loadReservations();
    }

    void displayTrains() {
        for (int i = 0; i < trainCount; i++)
            trains[i].display();
    }

    void searchTrains() {
        string src, dest;
        bool found = false;

        cout << "Enter source: ";
        getline(cin >> ws, src);

        cout << "Enter destination: ";
        getline(cin >> ws, dest);

        for (int i = 0; i < trainCount; i++) {
            if (trains[i].getSource() == src &&
                trains[i].getDestination() == dest) {
                trains[i].display();
                found = true;
            }
        }

        if (!found)
            cout << "No trains found.\n";
    }

    void bookTicket() {
        int number;
        string passenger;

        if (reservationCount >= 100) {
            cout << "Reservation limit reached!\n";
            return;
        }

        cout << "Enter train number: ";
        cin >> number;

        Train* train = findTrain(number);

        if (train == NULL) {
            cout << "Invalid train number!\n";
            return;
        }

        if (train->getAvailableSeats() == 0) {
            cout << "No seats available!\n";
            return;
        }

        cout << "Enter passenger name: ";
        getline(cin >> ws, passenger);

        if (passenger.empty()) {
            cout << "Invalid passenger name!\n";
            return;
        }

        train->bookSeat();

        reservations[reservationCount].createReservation(
            nextPNR,
            passenger,
            train->getTrainNumber(),
            train->getTrainName(),
            train->getFare()
        );

        cout << "\nTicket booked successfully!\n";
        cout << "PNR: " << nextPNR << '\n';

        nextPNR++;
        reservationCount++;

        saveTrains();
        saveReservations();
    }

    void viewReservation() {
        int pnr;

        cout << "Enter PNR: ";
        cin >> pnr;

        for (int i = 0; i < reservationCount; i++) {
            if (reservations[i].getPNR() == pnr) {
                reservations[i].display();
                return;
            }
        }

        cout << "Reservation not found!\n";
    }

    void cancelTicket() {
        int pnr;

        cout << "Enter PNR to cancel: ";
        cin >> pnr;

        for (int i = 0; i < reservationCount; i++) {
            if (reservations[i].getPNR() == pnr) {
                if (!reservations[i].isActive()) {
                    cout << "Ticket already cancelled!\n";
                    return;
                }

                Train* train =
                    findTrain(reservations[i].getTrainNumber());

                if (train != NULL)
                    train->cancelSeat();

                reservations[i].cancel();

                saveTrains();
                saveReservations();

                cout << "Ticket cancelled successfully!\n";
                return;
            }
        }

        cout << "Reservation not found!\n";
    }

    void displayAllReservations() {
        if (reservationCount == 0) {
            cout << "No reservations found.\n";
            return;
        }

        for (int i = 0; i < reservationCount; i++)
            reservations[i].display();
    }

    // Web API & Programmatic Access Helpers
    int getTrainCount() const {
        return trainCount;
    }

    Train* getTrainByIndex(int index) {
        if (index >= 0 && index < trainCount)
            return &trains[index];
        return NULL;
    }

    Train* findTrainByNumber(int number) {
        return findTrain(number);
    }

    int getReservationCount() const {
        return reservationCount;
    }

    Reservation* getReservationByIndex(int index) {
        if (index >= 0 && index < reservationCount)
            return &reservations[index];
        return NULL;
    }

    Reservation* findReservationByPNR(int pnr) {
        for (int i = 0; i < reservationCount; i++) {
            if (reservations[i].getPNR() == pnr)
                return &reservations[i];
        }
        return NULL;
    }

    bool bookTicketProgrammatic(int number, const string& passenger, int& outPNR, string& errorMsg) {
        if (reservationCount >= 100) {
            errorMsg = "Reservation limit reached! Maximum capacity is 100 bookings.";
            return false;
        }

        if (passenger.empty()) {
            errorMsg = "Invalid passenger name! Name cannot be blank.";
            return false;
        }

        Train* train = findTrain(number);
        if (train == NULL) {
            errorMsg = "Invalid train number! Train does not exist.";
            return false;
        }

        if (train->getAvailableSeats() <= 0) {
            errorMsg = "No seats available on this train!";
            return false;
        }

        train->bookSeat();

        reservations[reservationCount].createReservation(
            nextPNR,
            passenger,
            train->getTrainNumber(),
            train->getTrainName(),
            train->getFare()
        );

        outPNR = nextPNR;
        nextPNR++;
        reservationCount++;

        saveTrains();
        saveReservations();
        return true;
    }

    bool cancelTicketProgrammatic(int pnr, string& errorMsg) {
        for (int i = 0; i < reservationCount; i++) {
            if (reservations[i].getPNR() == pnr) {
                if (!reservations[i].isActive()) {
                    errorMsg = "Ticket already cancelled!";
                    return false;
                }

                Train* train = findTrain(reservations[i].getTrainNumber());
                if (train != NULL)
                    train->cancelSeat();

                reservations[i].cancel();

                saveTrains();
                saveReservations();
                return true;
            }
        }

        errorMsg = "Reservation not found! Please check the PNR number.";
        return false;
    }

    int getBookedSeats(int trainNumber) {
        int count = 0;
        for (int i = 0; i < reservationCount; i++) {
            if (reservations[i].getTrainNumber() == trainNumber && reservations[i].isActive()) {
                count++;
            }
        }
        return count;
    }

    bool addTrainProgrammatic(int number, const string& name, const string& src,
                              const string& dest, int seats, int price, string& errorMsg) {
        if (trainCount >= 100) {
            errorMsg = "Train fleet capacity reached! Maximum 100 trains allowed.";
            return false;
        }

        if (number <= 0) {
            errorMsg = "Train number must be a positive integer!";
            return false;
        }

        if (findTrain(number) != NULL) {
            errorMsg = "Train number already exists! Duplicate train numbers are not allowed.";
            return false;
        }

        if (name.empty()) {
            errorMsg = "Train name cannot be blank!";
            return false;
        }

        if (src.empty()) {
            errorMsg = "Source station cannot be blank!";
            return false;
        }

        if (dest.empty()) {
            errorMsg = "Destination station cannot be blank!";
            return false;
        }

        string sLower = src;
        string dLower = dest;
        for (size_t i = 0; i < sLower.size(); ++i) sLower[i] = tolower(static_cast<unsigned char>(sLower[i]));
        for (size_t i = 0; i < dLower.size(); ++i) dLower[i] = tolower(static_cast<unsigned char>(dLower[i]));
        if (sLower == dLower) {
            errorMsg = "Source and destination stations cannot be the same!";
            return false;
        }

        if (seats <= 0) {
            errorMsg = "Total seats must be a positive integer!";
            return false;
        }

        if (price <= 0) {
            errorMsg = "Fare must be a positive amount!";
            return false;
        }

        trains[trainCount] = Train(number, name, src, dest, seats, price);
        trainCount++;

        saveTrains();
        return true;
    }

    bool updateTrainCapacityProgrammatic(int number, int newCapacity, string& errorMsg) {
        if (newCapacity <= 0) {
            errorMsg = "Seat capacity must be a positive integer!";
            return false;
        }

        Train* train = findTrain(number);
        if (train == NULL) {
            errorMsg = "Train not found!";
            return false;
        }

        int booked = getBookedSeats(number);
        if (newCapacity < booked) {
            errorMsg = "Cannot reduce seat capacity to " + to_string(newCapacity) +
                       ". There are currently " + to_string(booked) + " active bookings on this train.";
            return false;
        }

        train->setCapacity(newCapacity, booked);
        saveTrains();
        return true;
    }

    // Add new train (Console)
    void addNewTrain() {
        if (trainCount >= 100) {
            cout << "\n[!] Train fleet capacity reached (Maximum 100 trains allowed)!\n";
            return;
        }
        cout << "\n==================== ADD NEW TRAIN ====================\n";
        int number;
        cout << "Enter Train Number (e.g. 12005): ";
        if (!(cin >> number)) {
            if (cin.eof()) return;
            cout << "\n[!] Invalid input! Train number must be a number.\n";
            cin.clear();
            cin.ignore(10000, '\n');
            return;
        }
        if (number <= 0) {
            cout << "\n[!] Train number must be a positive integer!\n";
            cin.ignore(10000, '\n');
            return;
        }
        for (int i = 0; i < trainCount; i++) {
            if (trains[i].getTrainNumber() == number) {
                cout << "\n[!] Error: Train #" << number << " already exists in the system!\n";
                cin.ignore(10000, '\n');
                return;
            }
        }
        cin.ignore(10000, '\n');
        string name = "";
        cout << "Enter Train Name (e.g. Tejas Express): ";
        while (name.empty()) {
            if (!getline(cin, name)) {
                if (cin.eof()) return;
            }
            size_t first = name.find_first_not_of(" \t\r\n");
            if (first != string::npos) {
                size_t last = name.find_last_not_of(" \t\r\n");
                name = name.substr(first, (last - first + 1));
            } else {
                name.clear();
            }
        }
        string source = "";
        cout << "Enter Source Station (e.g. Mumbai): ";
        while (source.empty()) {
            if (!getline(cin, source)) {
                if (cin.eof()) return;
            }
            size_t first = source.find_first_not_of(" \t\r\n");
            if (first != string::npos) {
                size_t last = source.find_last_not_of(" \t\r\n");
                source = source.substr(first, (last - first + 1));
            } else {
                source.clear();
            }
        }
        string dest = "";
        cout << "Enter Destination Station (e.g. Goa): ";
        while (dest.empty()) {
            if (!getline(cin, dest)) {
                if (cin.eof()) return;
            }
            size_t first = dest.find_first_not_of(" \t\r\n");
            if (first != string::npos) {
                size_t last = dest.find_last_not_of(" \t\r\n");
                dest = dest.substr(first, (last - first + 1));
            } else {
                dest.clear();
            }
        }
        if (source == dest) {
            cout << "\n[!] Source and destination stations cannot be the same!\n";
            return;
        }
        int seats;
        cout << "Enter Total Seat Capacity (e.g. 50): ";
        if (!(cin >> seats) || seats <= 0) {
            if (cin.eof()) return;
            cout << "\n[!] Invalid input! Total seats must be a positive integer.\n";
            cin.clear();
            cin.ignore(10000, '\n');
            return;
        }
        int price;
        cout << "Enter Ticket Fare in Rs. (e.g. 950): ";
        if (!(cin >> price) || price <= 0) {
            if (cin.eof()) return;
            cout << "\n[!] Invalid input! Fare must be a positive amount.\n";
            cin.clear();
            cin.ignore(10000, '\n');
            return;
        }
        cin.ignore(10000, '\n');
        trains[trainCount] = Train(number, name, source, dest, seats, price);
        trainCount++;
        saveTrains();
        cout << "\n======================================================\n";
        cout << "               TRAIN ADDED SUCCESSFULLY!              \n";
        cout << "======================================================\n";
        cout << "  Train Number   : " << number << "\n";
        cout << "  Train Name     : " << name << "\n";
        cout << "  Route          : " << source << " -> " << dest << "\n";
        cout << "  Seat Capacity  : " << seats << "\n";
        cout << "  Ticket Fare    : Rs. " << price << "\n";
        cout << "======================================================\n\n";
    }

    // Edit train seat capacity (Console)
    void editTrainSeats() {
        if (trainCount == 0) {
            cout << "\n[!] No trains available to edit.\n";
            return;
        }
        cout << "\n================= EDIT TRAIN SEATS =================\n";
        cout << "Current Train Fleet:\n";
        for (int i = 0; i < trainCount; i++) {
            int booked = getBookedSeats(trains[i].getTrainNumber());
            cout << "  [" << (i + 1) << "] " << trains[i].getTrainName()
                 << " (Train #" << trains[i].getTrainNumber() << ")"
                 << " | Capacity: " << trains[i].getTotalSeats()
                 << " | Booked: " << booked
                 << " | Available: " << trains[i].getAvailableSeats() << "\n";
        }
        cout << "----------------------------------------------------\n";
        cout << "Enter Train Number or Option [1-" << trainCount << "] to edit: ";
        int number;
        if (!(cin >> number)) {
            if (cin.eof()) return;
            cout << "\n[!] Invalid input! Please enter a number.\n";
            cin.clear();
            cin.ignore(10000, '\n');
            return;
        }
        cin.ignore(10000, '\n');
        Train* train = findTrain(number);
        if (train == NULL) {
            cout << "\n[!] Error: Train not found matching \"" << number << "\"!\n";
            return;
        }
        int booked = getBookedSeats(train->getTrainNumber());
        cout << "\nSelected: Train #" << train->getTrainNumber() << " (" << train->getTrainName() << ")\n";
        cout << "  Current Total Capacity : " << train->getTotalSeats() << " seats\n";
        cout << "  Active Confirmed Bookings: " << booked << " seat(s)\n";
        cout << "  Current Available Seats  : " << train->getAvailableSeats() << " seat(s)\n";
        cout << "----------------------------------------------------\n";
        cout << "Enter New Total Seat Capacity (minimum " << booked << "): ";
        int newCapacity;
        if (!(cin >> newCapacity) || newCapacity <= 0) {
            if (cin.eof()) return;
            cout << "\n[!] Seat capacity must be a positive integer!\n";
            cin.clear();
            cin.ignore(10000, '\n');
            return;
        }
        cin.ignore(10000, '\n');
        if (newCapacity < booked) {
            cout << "\n[!] Cannot reduce seat capacity to " << newCapacity << "!\n";
            cout << "    There are currently " << booked << " active bookings on this train.\n";
            return;
        }
        train->setCapacity(newCapacity, booked);
        saveTrains();
        cout << "\n======================================================\n";
        cout << "          SEAT CAPACITY UPDATED SUCCESSFULLY!         \n";
        cout << "======================================================\n";
        cout << "  Train Number     : " << train->getTrainNumber() << "\n";
        cout << "  Train Name       : " << train->getTrainName() << "\n";
        cout << "  New Capacity     : " << train->getTotalSeats() << " seats\n";
        cout << "  Active Bookings  : " << booked << " seats\n";
        cout << "  Available Now    : " << train->getAvailableSeats() << " seats\n";
        cout << "======================================================\n\n";
    }
};

// Passenger Class
class Passenger : public User {
private:
    RailwaySystem& railway;

public:
    Passenger(string n, RailwaySystem& system)
        : User(n), railway(system) {}

    void displayRole() override {
        cout << "\n============================================\n";
        cout << "       RAILWAY RESERVATION SYSTEM           \n";
        cout << "   Logged In User: " << name << " (Passenger)\n";
        cout << "   Note: 'Add Trains' option is DISABLED.   \n";
        cout << "============================================\n";
    }

    void menu() {
        int choice;

        do {
            cout << "\n============ PASSENGER MAIN MENU ============\n";
            cout << "1. Display All Trains\n";
            cout << "2. Search Trains by Route\n";
            cout << "3. Book a Ticket\n";
            cout << "4. View Reservation by PNR\n";
            cout << "5. Cancel a Ticket\n";
            cout << "6. View All Reservations\n";
            cout << "7. [Disabled] Add New Train (Head Only)\n";
            cout << "8. [Disabled] Edit Train Seat Capacity (Head Only)\n";
            cout << "0. Exit\n";
            cout << "=============================================\n";
            cout << "Enter your choice (0-6, or 7/8 for status): ";

            if (!(cin >> choice)) {
                if (cin.eof()) break;
                cout << "Invalid input!\n";
                cin.clear();
                cin.ignore(10000, '\n');
                continue;
            }

            switch (choice) {
                case 1:
                    railway.displayTrains();
                    break;

                case 2:
                    railway.searchTrains();
                    break;

                case 3:
                    railway.bookTicket();
                    break;

                case 4:
                    railway.viewReservation();
                    break;

                case 5:
                    railway.cancelTicket();
                    break;

                case 6:
                    railway.displayAllReservations();
                    break;

                case 7:
                    cout << "\n======================================================\n";
                    cout << " [!] ACCESS DENIED: Add Trains is restricted to HEAD. \n";
                    cout << "     Passengers cannot add trains to the system.      \n";
                    cout << "======================================================\n";
                    break;

                case 8:
                    cout << "\n======================================================\n";
                    cout << " [!] ACCESS DENIED: Edit Seats is restricted to HEAD. \n";
                    cout << "     Passengers cannot modify train seat capacity.    \n";
                    cout << "======================================================\n";
                    break;

                case 0:
                    cout << "Goodbye!\n";
                    break;

                default:
                    cout << "Invalid choice!\n";
            }
        } while (choice != 0);
    }
};

// Head (Administrator) Class
class Head : public User {
private:
    RailwaySystem& railway;

public:
    Head(string n, RailwaySystem& system)
        : User(n), railway(system) {}

    void displayRole() override {
        cout << "\n============================================\n";
        cout << "       RAILWAY RESERVATION SYSTEM           \n";
        cout << "   Logged In User: " << name << " (Head Administrator)\n";
        cout << "   Status: 'Add Trains' option is ENABLED.  \n";
        cout << "============================================\n";
    }

    void menu() {
        int choice;

        do {
            cout << "\n============== HEAD ADMIN MENU ==============\n";
            cout << "1. Display All Trains\n";
            cout << "2. Search Trains by Route\n";
            cout << "3. Book a Ticket\n";
            cout << "4. View Reservation by PNR\n";
            cout << "5. Cancel a Ticket\n";
            cout << "6. View All Reservations\n";
            cout << "7. Add New Train (Enabled)\n";
            cout << "8. Edit Train Seat Capacity (Enabled)\n";
            cout << "0. Exit\n";
            cout << "=============================================\n";
            cout << "Enter your choice (0-8): ";

            if (!(cin >> choice)) {
                if (cin.eof()) break;
                cout << "Invalid input!\n";
                cin.clear();
                cin.ignore(10000, '\n');
                continue;
            }

            switch (choice) {
                case 1:
                    railway.displayTrains();
                    break;

                case 2:
                    railway.searchTrains();
                    break;

                case 3:
                    railway.bookTicket();
                    break;

                case 4:
                    railway.viewReservation();
                    break;

                case 5:
                    railway.cancelTicket();
                    break;

                case 6:
                    railway.displayAllReservations();
                    break;

                case 7:
                    railway.addNewTrain();
                    break;

                case 8:
                    railway.editTrainSeats();
                    break;

                case 0:
                    cout << "Goodbye!\n";
                    break;

                default:
                    cout << "Invalid choice!\n";
            }
        } while (choice != 0);
    }
};

#ifndef RAILWAY_WEB_SERVER
int main() {
    RailwaySystem railway;

    while (true) {
        cout << "\n======================================================\n";
        cout << "     WELCOME TO RAILWAY RESERVATION SYSTEM (RailEase) \n";
        cout << "======================================================\n";
        cout << "  Please Select Your Role:\n";
        cout << "    [1] Passenger (Search, Book & View Tickets)\n";
        cout << "    [2] Head      (Full Admin: Add Trains & Manage Fleet)\n";
        cout << "    [0] Exit Application\n";
        cout << "======================================================\n";
        cout << "Enter choice (1, 2, or 0): ";

        int roleChoice;
        if (!(cin >> roleChoice)) {
            if (cin.eof()) break;
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "\n[!] Invalid input! Please enter 1 for Passenger, 2 for Head, or 0 to Exit.\n";
            continue;
        }

        if (roleChoice == 0) {
            cout << "\nThank you for visiting RailEase. Have a safe journey!\n";
            break;
        } else if (roleChoice == 1) {
            Passenger passenger("Passenger", railway);
            User* user = &passenger;
            user->displayRole();
            passenger.menu();
            break;
        } else if (roleChoice == 2) {
            Head head("Head Administrator", railway);
            User* user = &head;
            user->displayRole();
            head.menu();
            break;
        } else {
            cout << "\n[!] Invalid choice! Please select 1 for Passenger or 2 for Head.\n";
        }
    }

    return 0;
}
#endif