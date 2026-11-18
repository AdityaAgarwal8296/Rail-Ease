# RailEase — Railway Reservation System

> A professional full-stack web application transforming an existing C++ Railway Reservation System into a modern web experience backed by a native C++ REST API and persistent text-file storage.

---

## 1. System Architecture

```
+-------------------------------------------------------------+
|                      Browser Frontend                       |
|         (HTML5, CSS3, Vanilla JS Single Page App)           |
+------------------------------+------------------------------+
                               | HTTP / JSON Requests
                               v
+-------------------------------------------------------------+
|                 C++ Web Server (server.cpp)                 |
|   - cpp-httplib (HTTP routing & static file serving)        |
|   - nlohmann/json (JSON request parsing & serialization)    |
|   - Thread-safe Mutex locking over RailwaySystem            |
+------------------------------+------------------------------+
                               | Direct Method Invocations
                               v
+-------------------------------------------------------------+
|            Existing Railway Core (railway.cpp)              |
|   - Preserved classes: User, Train, Reservation,            |
|     RailwaySystem, Passenger                                |
|   - Zero duplication of booking / reservation logic         |
+------------------------------+------------------------------+
                               | Text-File Persistence
                               v
+-------------------------------------------------------------+
|                 trains.txt  &  passengers.txt               |
+-------------------------------------------------------------+
```

---

## 2. Directory Structure

```
c:\OOPS/
├── include/                     # C++ Header Dependencies
│   ├── httplib.h                # cpp-httplib single-header HTTP server
│   ├── json.hpp                 # nlohmann/json single-header serialization
│   ├── mingw.thread.h           # MinGW threading support header
│   ├── mingw.mutex.h            # MinGW mutex support header
│   ├── mingw.condition_variable.h # MinGW condition variable header
│   ├── mingw.shared_mutex.h     # MinGW shared mutex header
│   ├── mingw.invoke.h           # MinGW invoke helper
│   └── mingw.future.h           # MinGW future helper
├── public/                      # Polished Web Frontend
│   ├── index.html               # Semantic HTML5 SPA with all views & modals
│   ├── css/
│   │   └── style.css            # Modern CSS3 stylesheet (Navy Blue & Red theme)
│   └── js/
│       └── app.js               # Vanilla JavaScript API client & UI controller
├── trains.txt                   # Persistent train fleet records
├── passengers.txt               # Persistent passenger & reservation records
├── railway.cpp                  # Original C++ program with non-intrusive getters
├── railway.exe                  # Compiled standalone console application
├── server.cpp                   # C++ REST API Server & static file host
├── server.exe                   # Compiled Web Server executable
├── build.bat                    # Automated one-click build script
├── run.bat                      # Automated one-click server launch script
├── test_suite.ps1               # Automated end-to-end regression test suite
└── README.md                    # Complete project documentation
```

---

## 3. Preservation of Existing C++ Code (`railway.cpp`)

The existing `railway.cpp` source code is preserved:
- All classes (`User`, `Train`, `Reservation`, `RailwaySystem`, `Passenger`) retain their original members and methods.
- The interactive console application remains fully functional.
- The text file format for `trains.txt` and `passengers.txt` is identical.
- **Minimal, well-documented changes made to `railway.cpp`**:
  1. `Train`: added getter `int getTotalSeats() { return totalSeats; }`.
  2. `Reservation`: added getters `string getPassengerName()`, `string getTrainName()`, and `int getFare()`.
  3. `RailwaySystem`: added programmatic query and booking methods (`bookTicketProgrammatic`, `cancelTicketProgrammatic`, `getTrainByIndex`, `getReservationByIndex`, etc.) which mirror the exact console validation rules.
  4. `main()`: wrapped with `#ifndef RAILWAY_WEB_SERVER` so `railway.cpp` can be compiled as the standalone console application (`railway.exe`) or linked into `server.cpp` without symbol collisions.

---

## 4. REST API Documentation

Base URL: `http://localhost:8080`

### 1. Server Health Check
- **Endpoint**: `GET /api/health`
- **Response** (`200 OK`):
  ```json
  {
    "service": "RailEase API",
    "status": "healthy",
    "version": "1.0.0"
  }
  ```

### 2. List All Trains
- **Endpoint**: `GET /api/trains`
- **Response** (`200 OK`):
  ```json
  {
    "count": 3,
    "success": true,
    "trains": [
      {
        "trainNumber": 12001,
        "trainName": "Shatabdi Express",
        "source": "Delhi",
        "destination": "Agra",
        "totalSeats": 3,
        "availableSeats": 3,
        "fare": 750
      }
    ]
  }
  ```

### 3. Search Trains
- **Endpoint**: `GET /api/trains/search?source=Delhi&destination=Agra`
- **Query Parameters**:
  - `source` (optional): Source station (case-insensitive)
  - `destination` (optional): Destination station (case-insensitive)
- **Response** (`200 OK`):
  ```json
  {
    "success": true,
    "count": 1,
    "searchCriteria": { "source": "Delhi", "destination": "Agra" },
    "trains": [ ... ]
  }
  ```

### 4. Get Train Details by Number
- **Endpoint**: `GET /api/trains/:number`
- **Response** (`200 OK` or `404 Not Found`):
  ```json
  {
    "success": true,
    "train": {
      "trainNumber": 12001,
      "trainName": "Shatabdi Express",
      "source": "Delhi",
      "destination": "Agra",
      "totalSeats": 3,
      "availableSeats": 3,
      "fare": 750
    }
  }
  ```

### 5. List All Reservations
- **Endpoint**: `GET /api/bookings`
- **Response** (`200 OK`):
  ```json
  {
    "success": true,
    "count": 2,
    "bookings": [
      {
        "pnr": 1001,
        "passengerName": "Aditya",
        "trainNumber": 12003,
        "trainName": "Intercity Express",
        "fare": 350,
        "active": true,
        "status": "CONFIRMED"
      }
    ]
  }
  ```

### 6. Query Reservation by PNR
- **Endpoint**: `GET /api/bookings/:pnr`
- **Response** (`200 OK` or `404 Not Found`):
  ```json
  {
    "success": true,
    "booking": {
      "pnr": 1001,
      "passengerName": "Aditya",
      "trainNumber": 12003,
      "trainName": "Intercity Express",
      "fare": 350,
      "active": true,
      "status": "CONFIRMED"
    }
  }
  ```

### 7. Book a Ticket
- **Endpoint**: `POST /api/bookings`
- **Headers**: `Content-Type: application/json`
- **Request Body**:
  ```json
  {
    "trainNumber": 12001,
    "passengerName": "Rahul Sharma"
  }
  ```
- **Response** (`201 Created`):
  ```json
  {
    "success": true,
    "message": "Ticket booked successfully!",
    "pnr": 1002,
    "booking": {
      "pnr": 1002,
      "passengerName": "Rahul Sharma",
      "trainNumber": 12001,
      "trainName": "Shatabdi Express",
      "fare": 750,
      "active": true,
      "status": "CONFIRMED"
    }
  }
  ```
- **Error Responses**:
  - `400 Bad Request`: Missing fields or blank passenger name.
  - `404 Not Found`: Invalid train number.
  - `409 Conflict`: No seats available or reservation limit reached.

### 8. Cancel a Ticket
- **Endpoint**: `POST /api/bookings/:pnr/cancel`
- **Response** (`200 OK`):
  ```json
  {
    "success": true,
    "message": "Ticket cancelled successfully!",
    "pnr": 1002,
    "booking": {
      "pnr": 1002,
      "status": "CANCELLED",
      "active": false
    }
  }
  ```
- **Error Responses**:
  - `404 Not Found`: PNR does not exist.
  - `400 Bad Request`: Ticket is already cancelled.

### 9. Operations & Analytics Dashboard
- **Endpoint**: `GET /api/stats`
- **Response** (`200 OK`):
  ```json
  {
    "success": true,
    "totalTrains": 3,
    "totalSeats": 9,
    "availableSeats": 8,
    "bookedSeats": 1,
    "totalReservations": 2,
    "confirmedBookings": 1,
    "cancelledBookings": 1,
    "recentReservations": [ ... ]
  }
  ```

### 10. Add New Express Train (Administrator)
- **Endpoint**: `POST /api/trains`
- **Headers**: `Content-Type: application/json`
- **Request Body**:
  ```json
  {
    "trainNumber": 12004,
    "trainName": "Vande Bharat Express",
    "source": "Delhi",
    "destination": "Varanasi",
    "totalSeats": 10,
    "fare": 1250
  }
  ```
- **Response** (`201 Created`):
  ```json
  {
    "success": true,
    "message": "Train added successfully!",
    "train": {
      "trainNumber": 12004,
      "trainName": "Vande Bharat Express",
      "source": "Delhi",
      "destination": "Varanasi",
      "totalSeats": 10,
      "availableSeats": 10,
      "bookedSeats": 0,
      "fare": 1250
    }
  }
  ```
- **Error Responses**:
  - `400 Bad Request`: Invalid/non-positive train number, blank train name, empty stations, identical source and destination, non-positive total seats, or non-positive fare.
  - `409 Conflict`: Train number already exists in fleet or maximum fleet capacity reached.

### 11. Modify Train Seat Capacity (Administrator)
- **Endpoint**: `PATCH /api/trains/:number/capacity` (also supports `POST /api/trains/:number/capacity`)
- **Headers**: `Content-Type: application/json`
- **Request Body**:
  ```json
  {
    "totalSeats": 15
  }
  ```
- **Business Rules Enforced**:
  1. Total seat capacity must be a positive integer (`> 0`).
  2. Total seat capacity cannot be reduced below the count of active reservations on that train.
  3. Active reservations are never deleted or modified merely to reduce capacity.
  4. Cancelled reservations are excluded from booked seat counts.
  5. Available seats are recalculated automatically: `availableSeats = newTotalSeats - bookedSeats`.
  6. Capacity changes are persisted to `trains.txt` and reloaded across server restarts.
- **Response** (`200 OK`):
  ```json
  {
    "success": true,
    "message": "Train seat capacity updated successfully!",
    "train": {
      "trainNumber": 12004,
      "trainName": "Vande Bharat Express",
      "totalSeats": 15,
      "availableSeats": 15,
      "bookedSeats": 0,
      "fare": 1250
    }
  }
  ```
- **Error Responses**:
  - `400 Bad Request`: Invalid or non-positive capacity.
  - `404 Not Found`: Train number does not exist.
  - `409 Conflict`: Requested capacity is less than the number of active bookings.

---

## 5. Build and Run Instructions (Windows MinGW)

### Prerequisites
- GCC / MinGW (`g++`) with C++17 support.
- Windows Winsock library (`-lws2_32`).

### Quick One-Click Build & Run
Run from Command Prompt or PowerShell:
```cmd
build.bat
run.bat
```

### Manual Compilation Commands

1. **Compile Original Console Application**:
   ```cmd
   g++ -std=c++17 railway.cpp -o railway.exe
   ```
   Run in console:
   ```cmd
   .\railway.exe
   ```

2. **Compile Web Server**:
   ```cmd
   g++ -std=c++17 -Iinclude server.cpp -lws2_32 -o server.exe
   ```
   Run the web server:
   ```cmd
   .\server.exe 8080
   ```
   Open your browser at `http://localhost:8080`.

---

## 6. Dependencies Setup

All third-party header libraries are self-contained in the `include/` directory:
- **cpp-httplib** (`include/httplib.h`):
  Obtained from [yhirose/cpp-httplib](https://github.com/yhirose/cpp-httplib).
- **nlohmann/json** (`include/json.hpp`):
  Obtained from [nlohmann/json](https://github.com/nlohmann/json).
- **mingw-std-threads** (`include/mingw.*.h`):
  Obtained from [meganz/mingw-std-threads](https://github.com/meganz/mingw-std-threads) for seamless Windows MinGW threading support.

To re-download or update headers:
```powershell
curl.exe -L "https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp" -o "include/json.hpp"
curl.exe -L "https://raw.githubusercontent.com/yhirose/cpp-httplib/v0.14.0/httplib.h" -o "include/httplib.h"
```

---

## 7. Automated Testing Suite

To run the automated regression test suite:
```powershell
powershell -ExecutionPolicy Bypass -File .\test_suite.ps1
```
The suite covers **18 comprehensive test scenarios** with **78 automated assertions**:
1. Server health check
2. Train listing & baseline counts
3. Adding a new valid express train (`POST /api/trains`)
4. Retrieving the newly added train by number (`GET /api/trains/:number`)
5. Searching for the newly added train by route (`GET /api/trains/search`)
6. Duplicate train number rejection (`409 Conflict`)
7. Invalid train data validation (negative number, blank name, empty stations, same source and destination, invalid seats, negative fare -> `400 Bad Request`)
8. Increasing capacity on a train with no bookings & checking available seats
9. Booking tickets on new train & increasing capacity with active bookings (recalculating available seats without affecting booked count)
10. Decreasing seat capacity down to exact active bookings count (available becomes 0)
11. Rejecting seat capacity decrease below active bookings (`409 Conflict`)
12. Verifying cancelled reservations do not prevent capacity reduction
13. Verifying passenger bookings remain intact and valid after capacity adjustments
14. Non-existent train capacity change (`404 Not Found`) & invalid values (`400 Bad Request`)
15. Capacity expansion, ticket booking, and cancellation on new train with automatic seat restoration
16. Full server restart persistence (train added, updated capacity, and active bookings reloaded from disk)
17. Original baseline endpoint verification (PNR lookups, route searches, invalid PNRs)
18. Live operations dashboard statistics integrity
