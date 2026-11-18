#include <iostream>
#include <string>
#include <fstream>
#include <limits>

using namespace std;

// Abstract base class
class User {
protected:
    string name;
public:
    User(string n) : name(n) {}
    virtual void displayRole() = 0;
    virtual ~User() {}
};

// Train class
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
    Train() : trainNumber(0), totalSeats(0), availableSeats(0), fare(0) {}
    Train(int number, string name, string src, string dest, int seats, int price)
        : trainNumber(number), trainName(name), source(src), destination(dest),
          totalSeats(seats), availableSeats(seats), fare(price) {}

    int getTrainNumber() const { return trainNumber; }
    string getTrainName() const { return trainName; }
    string getSource() const { return source; }
    string getDestination() const { return destination; }
    int getAvailableSeats() const { return availableSeats; }
    int getFare() const { return fare; }
    int getTotalSeats() const { return totalSeats; }

    void setCapacity(int newTotalSeats, int bookedSeats) {
        totalSeats = newTotalSeats;
        availableSeats = totalSeats - bookedSeats;
        if (availableSeats < 0) availableSeats = 0;
    }

    bool bookSeat() {
        if (availableSeats > 0) {
            availableSeats--;
            return true;
        }
        return false;
    }

    void cancelSeat() {
        if (availableSeats < totalSeats) availableSeats++;
    }

    void display(int index = 0) const {
        cout << "\n-------------------------------------------------------------";
        if (index > 0) cout << "\n Option [" << index << "] Train Number : " << trainNumber;
        else cout << "\n Train Number    : " << trainNumber;
        cout << "\n Train Name      : " << trainName;
        cout << "\n Route           : " << source << " -> " << destination;
        cout << "\n Available Seats : " << availableSeats << " / " << totalSeats;
        cout << "\n Ticket Fare     : Rs. " << fare;
        cout << "\n-------------------------------------------------------------\n";
    }

    void save(ofstream& file) const {
        file << trainNumber << '\n' << trainName << '\n' << source << '\n'
             << destination << '\n' << totalSeats << '\n' << availableSeats << '\n'
             << fare << '\n';
    }

    bool load(ifstream& file) {
        if (!(file >> trainNumber)) return false;
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

// Reservation class
class Reservation {
private:
    int pnr;
    string passengerName;
    int trainNumber;
    string trainName;
    int fare;
    bool active;

public:
    Reservation() : pnr(0), trainNumber(0), fare(0), active(false) {}

    void createReservation(int id, string passenger, int number, string name, int price) {
        pnr = id;
        passengerName = passenger;
        trainNumber = number;
        trainName = name;
        fare = price;
        active = true;
    }

    int getPNR() const { return pnr; }
    int getTrainNumber() const { return trainNumber; }
    bool isActive() const { return active; }
    void cancel() { active = false; }
    string getPassengerName() const { return passengerName; }
    string getTrainName() const { return trainName; }
    int getFare() const { return fare; }

    void display() const {
        cout << "\n-------------------------------------------------------------";
        cout << "\n PNR Number      : " << pnr;
        cout << "\n Passenger Name  : " << passengerName;
        cout << "\n Train Details   : " << trainName << " (#" << trainNumber << ")";
        cout << "\n Total Fare      : Rs. " << fare;
        cout << "\n Booking Status  : " << (active ? "CONFIRMED" : "CANCELLED");
        cout << "\n-------------------------------------------------------------\n";
    }

    void save(ofstream& file) const {
        file << pnr << '\n' << passengerName << '\n' << trainNumber << '\n'
             << trainName << '\n' << fare << '\n' << active << '\n';
    }

    bool load(ifstream& file) {
        if (!(file >> pnr)) return false;
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

// Railway system management
class RailwaySystem {
private:
    Train trains[100];
    Reservation reservations[100];
    int trainCount;
    int reservationCount;
    int nextPNR;

    // Find train by number or index
    Train* findTrain(int number) {
        for (int i = 0; i < trainCount; i++) {
            if (trains[i].getTrainNumber() == number) return &trains[i];
        }
        if (number >= 1 && number <= trainCount) return &trains[number - 1];
        return NULL;
    }

    // Save train data
    void saveTrains() {
        ofstream file("trains.txt");
        if (!file) return;
        file << trainCount << '\n';
        for (int i = 0; i < trainCount; i++) trains[i].save(file);
        file.close();
    }

    // Save passenger reservations
    void saveReservations() {
        ofstream file("passengers.txt");
        if (!file) return;
        file << reservationCount << '\n';
        for (int i = 0; i < reservationCount; i++) reservations[i].save(file);
        file.close();
    }

    // Load train data
    void loadTrains() {
        ifstream file("trains.txt");
        if (!file) {
            trainCount = 4;
            trains[0] = Train(12001, "Shatabdi Express", "Delhi", "Agra", 30, 750);
            trains[1] = Train(12002, "Rajdhani Express", "Mumbai", "Delhi", 30, 1800);
            trains[2] = Train(12003, "Intercity Express", "Kanpur", "Lucknow", 40, 350);
            trains[3] = Train(12004, "Vande Bharat Express", "Delhi", "Varanasi", 50, 1250);
            saveTrains();
            return;
        }
        int count = 0;
        if (!(file >> count) || count < 0 || count > 100) {
            file.close();
            trainCount = 4;
            trains[0] = Train(12001, "Shatabdi Express", "Delhi", "Agra", 30, 750);
            trains[1] = Train(12002, "Rajdhani Express", "Mumbai", "Delhi", 30, 1800);
            trains[2] = Train(12003, "Intercity Express", "Kanpur", "Lucknow", 40, 350);
            trains[3] = Train(12004, "Vande Bharat Express", "Delhi", "Varanasi", 50, 1250);
            saveTrains();
            return;
        }
        file.ignore(1000, '\n');
        trainCount = 0;
        for (int i = 0; i < count; i++) {
            if (trains[i].load(file)) trainCount++;
            else break;
        }
        file.close();
    }

    // Load reservation data
    void loadReservations() {
        ifstream file("passengers.txt");
        reservationCount = 0;
        nextPNR = 1001;
        if (!file) return;
        int count = 0;
        if (!(file >> count) || count < 0 || count > 100) {
            file.close();
            return;
        }
        file.ignore(1000, '\n');
        for (int i = 0; i < count; i++) {
            if (reservations[i].load(file)) {
                reservationCount++;
                if (reservations[i].getPNR() >= nextPNR) nextPNR = reservations[i].getPNR() + 1;
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

    // Count booked seats on a train
    int getBookedSeats(int trainNumber) const {
        int count = 0;
        for (int i = 0; i < reservationCount; i++) {
            if (reservations[i].getTrainNumber() == trainNumber && reservations[i].isActive()) count++;
        }
        return count;
    }

    // Show all trains
    void displayTrains() const {
        if (trainCount == 0) {
            cout << "\nNo trains available in system.\n";
            return;
        }
        cout << "\n======================== AVAILABLE TRAINS ========================";
        for (int i = 0; i < trainCount; i++) trains[i].display(i + 1);
    }

    // Search trains by source and destination
    void searchTrains() const {
        string src, dest;
        cout << "\nEnter source station: ";
        cin.ignore(10000, '\n');
        if (!getline(cin, src)) return;
        cout << "Enter destination station: ";
        if (!getline(cin, dest)) return;
        bool found = false;
        cout << "\n======================== SEARCH RESULTS ========================";
        for (int i = 0; i < trainCount; i++) {
            if (trains[i].getSource() == src && trains[i].getDestination() == dest) {
                trains[i].display(i + 1);
                found = true;
            }
        }
        if (!found) cout << "\nNo direct trains found from \"" << src << "\" to \"" << dest << "\".\n";
    }

    // Book ticket and assign PNR
    void bookTicket() {
        if (reservationCount >= 100) {
            cout << "\n[!] Reservation limit reached! Maximum 100 bookings allowed.\n";
            return;
        }
        cout << "\n==================== BOOK A TICKET ====================\n";
        cout << "Available Trains:\n";
        for (int i = 0; i < trainCount; i++) {
            cout << "  [" << (i + 1) << "] " << trains[i].getTrainName()
                 << " (Train #" << trains[i].getTrainNumber() << ")"
                 << " | Route: " << trains[i].getSource() << " -> " << trains[i].getDestination()
                 << " | Available Seats: " << trains[i].getAvailableSeats()
                 << " | Fare: Rs. " << trains[i].getFare() << "\n";
        }
        cout << "-------------------------------------------------------\n";
        cout << "Enter Train Number (e.g. 12001) or Option [1-" << trainCount << "]: ";
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
            cout << "\n[!] Error: No train found matching \"" << number << "\"!\n";
            return;
        }
        if (train->getAvailableSeats() <= 0) {
            cout << "\n[!] Sorry! Train #" << train->getTrainNumber() << " (" << train->getTrainName() << ") has 0 seats available.\n";
            return;
        }
        string passenger = "";
        cout << "Enter Passenger Name: ";
        while (passenger.empty()) {
            if (!getline(cin, passenger)) {
                if (cin.eof()) return;
            }
            size_t first = passenger.find_first_not_of(" \t\r\n");
            if (first != string::npos) {
                size_t last = passenger.find_last_not_of(" \t\r\n");
                passenger = passenger.substr(first, (last - first + 1));
            } else {
                passenger.clear();
            }
        }
        train->bookSeat();
        int bookedPNR = nextPNR;
        reservations[reservationCount].createReservation(
            bookedPNR, passenger, train->getTrainNumber(), train->getTrainName(), train->getFare()
        );
        nextPNR++;
        reservationCount++;
        saveTrains();
        saveReservations();
        cout << "\n======================================================\n";
        cout << "             TICKET BOOKED SUCCESSFULLY!              \n";
        cout << "======================================================\n";
        cout << "  >>> YOUR PNR NUMBER : " << bookedPNR << " <<<\n";
        cout << "  Passenger Name      : " << passenger << "\n";
        cout << "  Train Name          : " << train->getTrainName() << " (#" << train->getTrainNumber() << ")\n";
        cout << "  Journey Route       : " << train->getSource() << " -> " << train->getDestination() << "\n";
        cout << "  Fare Amount         : Rs. " << train->getFare() << "\n";
        cout << "  Booking Status      : CONFIRMED\n";
        cout << "  Seats Remaining     : " << train->getAvailableSeats() << " / " << train->getTotalSeats() << "\n";
        cout << "======================================================\n\n";
    }

    // View reservation by PNR
    void viewReservation() const {
        int pnr;
        cout << "\nEnter PNR to search: ";
        if (!(cin >> pnr)) {
            if (cin.eof()) return;
            cout << "Invalid input! Please enter a numerical PNR.\n";
            cin.clear();
            cin.ignore(10000, '\n');
            return;
        }
        for (int i = 0; i < reservationCount; i++) {
            if (reservations[i].getPNR() == pnr) {
                reservations[i].display();
                return;
            }
        }
        cout << "\n[!] No reservation found with PNR " << pnr << "!\n";
    }

    // Cancel reservation by PNR
    void cancelTicket() {
        int pnr;
        cout << "\nEnter PNR to cancel: ";
        if (!(cin >> pnr)) {
            if (cin.eof()) return;
            cout << "Invalid input! Please enter a numerical PNR.\n";
            cin.clear();
            cin.ignore(10000, '\n');
            return;
        }
        for (int i = 0; i < reservationCount; i++) {
            if (reservations[i].getPNR() == pnr) {
                if (!reservations[i].isActive()) {
                    cout << "\n[!] Notice: Ticket with PNR " << pnr << " is already CANCELLED!\n";
                    return;
                }
                Train* train = findTrain(reservations[i].getTrainNumber());
                if (train != NULL) train->cancelSeat();
                reservations[i].cancel();
                saveTrains();
                saveReservations();
                cout << "\n======================================================\n";
                cout << "            TICKET CANCELLED SUCCESSFULLY!            \n";
                cout << "======================================================\n";
                cout << "  PNR Number     : " << pnr << "\n";
                cout << "  Passenger Name : " << reservations[i].getPassengerName() << "\n";
                cout << "  Status         : CANCELLED\n";
                if (train != NULL) {
                    cout << "  Seat Released  : Available seats on Train #"
                         << train->getTrainNumber() << " is now "
                         << train->getAvailableSeats() << " / " << train->getTotalSeats() << "\n";
                }
                cout << "======================================================\n\n";
                return;
            }
        }
        cout << "\n[!] No reservation found with PNR " << pnr << "!\n";
    }

    // Display all reservations
    void displayAllReservations() const {
        if (reservationCount == 0) {
            cout << "\nNo reservations on record.\n";
            return;
        }
        cout << "\n======================== ALL RESERVATIONS (" << reservationCount << ") ========================";
        for (int i = 0; i < reservationCount; i++) reservations[i].display();
    }

    // Add new train
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

    // Edit train seat capacity
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

// Passenger class
class Passenger : public User {
private:
    RailwaySystem& railway;

public:
    Passenger(string n, RailwaySystem& system) : User(n), railway(system) {}

    void displayRole() override {
        cout << "\n============================================\n";
        cout << "       RAILWAY RESERVATION SYSTEM           \n";
        cout << "   Logged In User: " << name << " (Passenger)\n";
        cout << "============================================\n";
    }

    void menu() {
        int choice;
        do {
            cout << "\n================ MAIN MENU ================\n";
            cout << " 1. Display All Trains\n";
            cout << " 2. Search Trains by Route\n";
            cout << " 3. Book a Ticket\n";
            cout << " 4. View Reservation by PNR\n";
            cout << " 5. Cancel a Ticket\n";
            cout << " 6. View All Reservations\n";
            cout << " 7. Add New Train\n";
            cout << " 8. Edit Train Seat Capacity\n";
            cout << " 0. Exit\n";
            cout << "===========================================\n";
            cout << "Enter your choice (0-8): ";

            if (!(cin >> choice)) {
                if (cin.eof()) break;
                cout << "\nInvalid choice! Please enter a number between 0 and 8.\n";
                cin.clear();
                cin.ignore(10000, '\n');
                continue;
            }

            switch (choice) {
                case 1: railway.displayTrains(); break;
                case 2: railway.searchTrains(); break;
                case 3: railway.bookTicket(); break;
                case 4: railway.viewReservation(); break;
                case 5: railway.cancelTicket(); break;
                case 6: railway.displayAllReservations(); break;
                case 7: railway.addNewTrain(); break;
                case 8: railway.editTrainSeats(); break;
                case 0: cout << "\nThank you for using Railway Reservation System. Goodbye!\n"; break;
                default: cout << "\nInvalid choice! Please select 0 to 8.\n";
            }
        } while (choice != 0);
    }
};

// Main function
int main() {
    RailwaySystem railway;
    Passenger passenger("Guest", railway);
    User* user = &passenger;
    user->displayRole();
    passenger.menu();
    return 0;
}
