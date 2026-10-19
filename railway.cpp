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
    Train trains[10];
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

        if (!(file >> count) || count < 0 || count > 10) {
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
};

// Passenger Class
class Passenger : public User {
private:
    RailwaySystem& railway;

public:
    Passenger(string n, RailwaySystem& system)
        : User(n), railway(system) {}

    void displayRole() override {
        cout << "\nWelcome, " << name << "!\n";
    }

    void menu() {
        int choice;

        do {
            cout << "\n===== RAILWAY RESERVATION SYSTEM =====\n";
            cout << "1. Display Trains\n";
            cout << "2. Search Trains\n";
            cout << "3. Book Ticket\n";
            cout << "4. View Reservation\n";
            cout << "5. Cancel Ticket\n";
            cout << "6. View All Reservations\n";
            cout << "0. Exit\n";
            cout << "Enter choice: ";

            if (!(cin >> choice)) {
                cout << "Invalid input!\n";
                break;
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

    Passenger passenger("Guest", railway);

    User* user = &passenger;
    user->displayRole();

    passenger.menu();

    return 0;
}
#endif