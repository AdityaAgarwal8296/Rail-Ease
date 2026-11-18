#ifdef _WIN32
#define WINVER 0x0501
#define _WIN32_WINNT 0x0501

#include <io.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#ifndef _access_s
#define _access_s(p, m) _access(p, m)
#endif

#ifndef IPV6_V6ONLY
#define IPV6_V6ONLY 27
#endif
#else
#include <unistd.h>
#endif

#include <string.h>
#include <strings.h>

#ifndef _stricmp
#define _stricmp strcasecmp
#endif

#if defined(__MINGW32__) && !defined(_GLIBCXX_HAS_GTHREADS)
#include "mingw.thread.h"
#include "mingw.mutex.h"
#include "mingw.condition_variable.h"
#endif

#define RAILWAY_WEB_SERVER
#include "railway.cpp"

#include "httplib.h"
#include "json.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <cstdlib>

using json = nlohmann::json;

// Global railway system instance protected by a mutex for thread safety
static RailwaySystem g_railway;
static std::mutex g_systemMutex;

// Helper function to trim strings
static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

// Case-insensitive string comparison
static bool iequals(const std::string& a, const std::string& b) {
    if (a.size() != b.size())
        return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (tolower(static_cast<unsigned char>(a[i])) != tolower(static_cast<unsigned char>(b[i])))
            return false;
    }
    return true;
}

// Convert a Train object to JSON
static json trainToJson(Train* train) {
    if (!train) return json::object();
    int booked = g_railway.getBookedSeats(train->getTrainNumber());
    return json{
        {"trainNumber", train->getTrainNumber()},
        {"trainName", train->getTrainName()},
        {"source", train->getSource()},
        {"destination", train->getDestination()},
        {"totalSeats", train->getTotalSeats()},
        {"availableSeats", train->getAvailableSeats()},
        {"bookedSeats", booked},
        {"fare", train->getFare()}
    };
}

// Convert a Reservation object to JSON
static json reservationToJson(Reservation* res) {
    if (!res) return json::object();
    return json{
        {"pnr", res->getPNR()},
        {"passengerName", res->getPassengerName()},
        {"trainNumber", res->getTrainNumber()},
        {"trainName", res->getTrainName()},
        {"fare", res->getFare()},
        {"active", res->isActive()},
        {"status", res->isActive() ? "CONFIRMED" : "CANCELLED"}
    };
}

// Helper to set CORS and JSON headers
static void setJson(httplib::Response& res, const json& j, int statusCode = 200) {
    res.status = statusCode;
    res.set_content(j.dump(2), "application/json");
}

int main(int argc, char* argv[]) {
    int port = 8080;
    const char* envPort = std::getenv("PORT");
    if (envPort) {
        try {
            port = std::stoi(envPort);
        } catch (...) {
            port = 8080;
        }
    } else if (argc > 1) {
        try {
            port = std::stoi(argv[1]);
        } catch (...) {
            port = 8080;
        }
    }

    httplib::Server svr;

    // Enable CORS for API routes
    svr.set_default_headers({
        {"Access-Control-Allow-Origin", "*"},
        {"Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS, PATCH"},
        {"Access-Control-Allow-Headers", "Content-Type, Authorization, Accept, X-User-Role"}
    });

    // Handle OPTIONS preflight requests
    svr.Options(".*", [](const httplib::Request&, httplib::Response& res) {
        res.status = 200;
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS, PATCH");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization, Accept, X-User-Role");
    });

    // Serve static files from ./public directory
    svr.set_mount_point("/", "./public");

    // -------------------------------------------------------------
    // API: GET /api/health
    // -------------------------------------------------------------
    svr.Get("/api/health", [](const httplib::Request&, httplib::Response& res) {
        json j = {
            {"status", "healthy"},
            {"service", "RailEase API"},
            {"version", "1.0.0"}
        };
        setJson(res, j, 200);
    });

    // -------------------------------------------------------------
    // API: GET /api/trains
    // -------------------------------------------------------------
    svr.Get("/api/trains", [](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_systemMutex);

        json trainList = json::array();
        int count = g_railway.getTrainCount();
        for (int i = 0; i < count; i++) {
            Train* t = g_railway.getTrainByIndex(i);
            if (t) {
                trainList.push_back(trainToJson(t));
            }
        }

        json response = {
            {"success", true},
            {"count", count},
            {"trains", trainList}
        };
        setJson(res, response, 200);
    });

    // -------------------------------------------------------------
    // API: GET /api/trains/search
    // Parameters: source, destination
    // -------------------------------------------------------------
    svr.Get("/api/trains/search", [](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_systemMutex);

        std::string src = "";
        std::string dest = "";
        if (req.has_param("source"))
            src = trim(req.get_param_value("source"));
        if (req.has_param("destination"))
            dest = trim(req.get_param_value("destination"));

        json matched = json::array();
        int count = g_railway.getTrainCount();

        for (int i = 0; i < count; i++) {
            Train* t = g_railway.getTrainByIndex(i);
            if (!t) continue;

            bool matchSrc = src.empty() || iequals(t->getSource(), src);
            bool matchDest = dest.empty() || iequals(t->getDestination(), dest);

            if (matchSrc && matchDest) {
                matched.push_back(trainToJson(t));
            }
        }

        json response = {
            {"success", true},
            {"searchCriteria", {
                {"source", src},
                {"destination", dest}
            }},
            {"count", matched.size()},
            {"trains", matched}
        };
        setJson(res, response, 200);
    });

    // -------------------------------------------------------------
    // API: GET /api/trains/:number
    // -------------------------------------------------------------
    svr.Get(R"(/api/trains/(\d+))", [](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_systemMutex);

        std::string numStr = req.matches[1];
        int trainNumber = 0;
        try {
            trainNumber = std::stoi(numStr);
        } catch (...) {
            setJson(res, {{"success", false}, {"error", "Invalid train number format"}}, 400);
            return;
        }

        Train* train = g_railway.findTrainByNumber(trainNumber);
        if (!train) {
            setJson(res, {{"success", false}, {"error", "Train number " + numStr + " not found"}}, 404);
            return;
        }

        json response = {
            {"success", true},
            {"train", trainToJson(train)}
        };
        setJson(res, response, 200);
    });

    // -------------------------------------------------------------
    // API: POST /api/trains
    // Creates a new train in the fleet
    // -------------------------------------------------------------
    svr.Post("/api/trains", [](const httplib::Request& req, httplib::Response& res) {
        // Role-based Access Control: Adding trains is restricted to Head
        if (req.has_header("X-User-Role")) {
            std::string role = req.get_header_value("X-User-Role");
            if (iequals(role, "passenger")) {
                setJson(res, {
                    {"success", false},
                    {"error", "Access Denied: The 'Add Trains' option is disabled for Passengers and is ONLY available to the Head."}
                }, 403);
                return;
            }
        }

        if (req.body.empty()) {
            setJson(res, {{"success", false}, {"error", "Request body cannot be empty"}}, 400);
            return;
        }

        json body;
        try {
            body = json::parse(req.body);
        } catch (const std::exception& e) {
            setJson(res, {{"success", false}, {"error", std::string("Malformed JSON body: ") + e.what()}}, 400);
            return;
        }

        // Required fields check
        if (!body.contains("trainNumber") || !body.contains("trainName") ||
            !body.contains("source") || !body.contains("destination") ||
            !body.contains("totalSeats") || !body.contains("fare")) {
            setJson(res, {{"success", false}, {"error", "Missing required fields: 'trainNumber', 'trainName', 'source', 'destination', 'totalSeats', and 'fare'"}}, 400);
            return;
        }

        int trainNumber = 0;
        try {
            if (body["trainNumber"].is_number()) {
                trainNumber = body["trainNumber"].get<int>();
            } else if (body["trainNumber"].is_string()) {
                trainNumber = std::stoi(body["trainNumber"].get<std::string>());
            } else {
                setJson(res, {{"success", false}, {"error", "'trainNumber' must be a valid integer"}}, 400);
                return;
            }
        } catch (...) {
            setJson(res, {{"success", false}, {"error", "'trainNumber' must be a valid integer"}}, 400);
            return;
        }

        if (trainNumber <= 0) {
            setJson(res, {{"success", false}, {"error", "Train number must be a positive integer"}}, 400);
            return;
        }

        if (!body["trainName"].is_string() || !body["source"].is_string() || !body["destination"].is_string()) {
            setJson(res, {{"success", false}, {"error", "'trainName', 'source', and 'destination' must be strings"}}, 400);
            return;
        }

        std::string trainName = trim(body["trainName"].get<std::string>());
        std::string source = trim(body["source"].get<std::string>());
        std::string destination = trim(body["destination"].get<std::string>());

        if (trainName.empty()) {
            setJson(res, {{"success", false}, {"error", "Train name cannot be blank"}}, 400);
            return;
        }
        if (source.empty()) {
            setJson(res, {{"success", false}, {"error", "Source station cannot be blank"}}, 400);
            return;
        }
        if (destination.empty()) {
            setJson(res, {{"success", false}, {"error", "Destination station cannot be blank"}}, 400);
            return;
        }
        if (iequals(source, destination)) {
            setJson(res, {{"success", false}, {"error", "Source and destination stations cannot be the same"}}, 400);
            return;
        }

        int totalSeats = 0;
        try {
            if (body["totalSeats"].is_number()) {
                totalSeats = body["totalSeats"].get<int>();
            } else if (body["totalSeats"].is_string()) {
                totalSeats = std::stoi(body["totalSeats"].get<std::string>());
            } else {
                setJson(res, {{"success", false}, {"error", "'totalSeats' must be a valid positive integer"}}, 400);
                return;
            }
        } catch (...) {
            setJson(res, {{"success", false}, {"error", "'totalSeats' must be a valid positive integer"}}, 400);
            return;
        }

        if (totalSeats <= 0) {
            setJson(res, {{"success", false}, {"error", "Total seats must be a positive integer"}}, 400);
            return;
        }

        int fare = 0;
        try {
            if (body["fare"].is_number()) {
                fare = body["fare"].get<int>();
            } else if (body["fare"].is_string()) {
                fare = std::stoi(body["fare"].get<std::string>());
            } else {
                setJson(res, {{"success", false}, {"error", "'fare' must be a valid positive amount"}}, 400);
                return;
            }
        } catch (...) {
            setJson(res, {{"success", false}, {"error", "'fare' must be a valid positive amount"}}, 400);
            return;
        }

        if (fare <= 0) {
            setJson(res, {{"success", false}, {"error", "Fare must be a positive amount"}}, 400);
            return;
        }

        std::lock_guard<std::mutex> lock(g_systemMutex);

        std::string errorMsg;
        bool ok = g_railway.addTrainProgrammatic(trainNumber, trainName, source, destination, totalSeats, fare, errorMsg);
        if (!ok) {
            int status = 400;
            if (errorMsg.find("already exists") != std::string::npos || errorMsg.find("Duplicate") != std::string::npos) {
                status = 409;
            } else if (errorMsg.find("capacity reached") != std::string::npos) {
                status = 409;
            }
            setJson(res, {{"success", false}, {"error", errorMsg}}, status);
            return;
        }

        Train* newTrain = g_railway.findTrainByNumber(trainNumber);
        json response = {
            {"success", true},
            {"message", "Train added successfully!"},
            {"train", trainToJson(newTrain)}
        };
        setJson(res, response, 201);
    });

    // -------------------------------------------------------------
    // API: PATCH & POST /api/trains/:number/capacity
    // Updates seat capacity for a train
    // -------------------------------------------------------------
    auto handleCapacityUpdate = [](const httplib::Request& req, httplib::Response& res) {
        // Role-based Access Control: Modifying capacity is restricted to Head
        if (req.has_header("X-User-Role")) {
            std::string role = req.get_header_value("X-User-Role");
            if (iequals(role, "passenger")) {
                setJson(res, {
                    {"success", false},
                    {"error", "Access Denied: Modifying train seat capacity is disabled for Passengers and is ONLY available to the Head."}
                }, 403);
                return;
            }
        }

        std::string numStr = req.matches[1];
        int trainNumber = 0;
        try {
            trainNumber = std::stoi(numStr);
        } catch (...) {
            setJson(res, {{"success", false}, {"error", "Invalid train number format"}}, 400);
            return;
        }

        if (req.body.empty()) {
            setJson(res, {{"success", false}, {"error", "Request body cannot be empty"}}, 400);
            return;
        }

        json body;
        try {
            body = json::parse(req.body);
        } catch (const std::exception& e) {
            setJson(res, {{"success", false}, {"error", std::string("Malformed JSON body: ") + e.what()}}, 400);
            return;
        }

        std::string key = "";
        if (body.contains("totalSeats")) key = "totalSeats";
        else if (body.contains("seatCapacity")) key = "seatCapacity";
        else if (body.contains("capacity")) key = "capacity";
        else {
            setJson(res, {{"success", false}, {"error", "Missing required field: 'totalSeats'"}}, 400);
            return;
        }

        int newCapacity = 0;
        try {
            if (body[key].is_number()) {
                newCapacity = body[key].get<int>();
            } else if (body[key].is_string()) {
                newCapacity = std::stoi(body[key].get<std::string>());
            } else {
                setJson(res, {{"success", false}, {"error", "'totalSeats' must be a valid positive integer"}}, 400);
                return;
            }
        } catch (...) {
            setJson(res, {{"success", false}, {"error", "'totalSeats' must be a valid positive integer"}}, 400);
            return;
        }

        if (newCapacity <= 0) {
            setJson(res, {{"success", false}, {"error", "Seat capacity must be a positive integer"}}, 400);
            return;
        }

        std::lock_guard<std::mutex> lock(g_systemMutex);

        Train* train = g_railway.findTrainByNumber(trainNumber);
        if (!train) {
            setJson(res, {{"success", false}, {"error", "Train number " + numStr + " not found"}}, 404);
            return;
        }

        int booked = g_railway.getBookedSeats(trainNumber);
        if (newCapacity < booked) {
            setJson(res, {
                {"success", false},
                {"error", "Cannot reduce seat capacity below active bookings (" + std::to_string(booked) + " active seats currently booked)"},
                {"activeBookings", booked},
                {"requestedCapacity", newCapacity}
            }, 409);
            return;
        }

        std::string errorMsg;
        bool ok = g_railway.updateTrainCapacityProgrammatic(trainNumber, newCapacity, errorMsg);
        if (!ok) {
            int status = 400;
            if (errorMsg.find("Cannot reduce") != std::string::npos) status = 409;
            setJson(res, {{"success", false}, {"error", errorMsg}}, status);
            return;
        }

        json response = {
            {"success", true},
            {"message", "Train seat capacity updated successfully!"},
            {"train", trainToJson(train)}
        };
        setJson(res, response, 200);
    };

    svr.Patch(R"(/api/trains/(\d+)/capacity)", handleCapacityUpdate);
    svr.Post(R"(/api/trains/(\d+)/capacity)", handleCapacityUpdate);

    // -------------------------------------------------------------
    // API: GET /api/bookings
    // -------------------------------------------------------------
    svr.Get("/api/bookings", [](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_systemMutex);

        json bookingList = json::array();
        int count = g_railway.getReservationCount();
        for (int i = 0; i < count; i++) {
            Reservation* r = g_railway.getReservationByIndex(i);
            if (r) {
                bookingList.push_back(reservationToJson(r));
            }
        }

        json response = {
            {"success", true},
            {"count", count},
            {"bookings", bookingList}
        };
        setJson(res, response, 200);
    });

    // -------------------------------------------------------------
    // API: GET /api/bookings/:pnr
    // -------------------------------------------------------------
    svr.Get(R"(/api/bookings/(\d+))", [](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_systemMutex);

        std::string pnrStr = req.matches[1];
        int pnr = 0;
        try {
            pnr = std::stoi(pnrStr);
        } catch (...) {
            setJson(res, {{"success", false}, {"error", "Invalid PNR format"}}, 400);
            return;
        }

        Reservation* r = g_railway.findReservationByPNR(pnr);
        if (!r) {
            setJson(res, {{"success", false}, {"error", "No reservation found with PNR " + pnrStr}}, 404);
            return;
        }

        json response = {
            {"success", true},
            {"booking", reservationToJson(r)}
        };
        setJson(res, response, 200);
    });

    // -------------------------------------------------------------
    // API: POST /api/bookings
    // Request body: { "trainNumber": 12001, "passengerName": "..." }
    // -------------------------------------------------------------
    svr.Post("/api/bookings", [](const httplib::Request& req, httplib::Response& res) {
        if (req.body.empty()) {
            setJson(res, {{"success", false}, {"error", "Request body cannot be empty"}}, 400);
            return;
        }

        json body;
        try {
            body = json::parse(req.body);
        } catch (const std::exception& e) {
            setJson(res, {{"success", false}, {"error", std::string("Malformed JSON body: ") + e.what()}}, 400);
            return;
        }

        if (!body.contains("trainNumber") || !body.contains("passengerName")) {
            setJson(res, {{"success", false}, {"error", "Missing required fields: 'trainNumber' and 'passengerName'"}}, 400);
            return;
        }

        int trainNumber = 0;
        try {
            if (body["trainNumber"].is_number()) {
                trainNumber = body["trainNumber"].get<int>();
            } else if (body["trainNumber"].is_string()) {
                trainNumber = std::stoi(body["trainNumber"].get<std::string>());
            } else {
                setJson(res, {{"success", false}, {"error", "'trainNumber' must be a valid integer"}}, 400);
                return;
            }
        } catch (...) {
            setJson(res, {{"success", false}, {"error", "'trainNumber' must be a valid integer"}}, 400);
            return;
        }

        if (!body["passengerName"].is_string()) {
            setJson(res, {{"success", false}, {"error", "'passengerName' must be a string"}}, 400);
            return;
        }

        std::string passengerName = trim(body["passengerName"].get<std::string>());
        if (passengerName.empty()) {
            setJson(res, {{"success", false}, {"error", "Passenger name cannot be blank"}}, 400);
            return;
        }

        std::lock_guard<std::mutex> lock(g_systemMutex);

        int outPNR = 0;
        std::string errorMsg;
        bool ok = g_railway.bookTicketProgrammatic(trainNumber, passengerName, outPNR, errorMsg);

        if (!ok) {
            int status = 400;
            if (errorMsg.find("not exist") != std::string::npos) status = 404;
            else if (errorMsg.find("No seats") != std::string::npos || errorMsg.find("limit reached") != std::string::npos) status = 409;
            setJson(res, {{"success", false}, {"error", errorMsg}}, status);
            return;
        }

        Reservation* newRes = g_railway.findReservationByPNR(outPNR);
        json response = {
            {"success", true},
            {"message", "Ticket booked successfully!"},
            {"pnr", outPNR},
            {"booking", reservationToJson(newRes)}
        };
        setJson(res, response, 201);
    });

    // -------------------------------------------------------------
    // API: POST /api/bookings/:pnr/cancel
    // -------------------------------------------------------------
    svr.Post(R"(/api/bookings/(\d+)/cancel)", [](const httplib::Request& req, httplib::Response& res) {
        std::string pnrStr = req.matches[1];
        int pnr = 0;
        try {
            pnr = std::stoi(pnrStr);
        } catch (...) {
            setJson(res, {{"success", false}, {"error", "Invalid PNR format"}}, 400);
            return;
        }

        std::lock_guard<std::mutex> lock(g_systemMutex);

        std::string errorMsg;
        bool ok = g_railway.cancelTicketProgrammatic(pnr, errorMsg);

        if (!ok) {
            int status = 400;
            if (errorMsg.find("not found") != std::string::npos) status = 404;
            setJson(res, {{"success", false}, {"error", errorMsg}}, status);
            return;
        }

        Reservation* cancelledRes = g_railway.findReservationByPNR(pnr);
        json response = {
            {"success", true},
            {"message", "Ticket cancelled successfully!"},
            {"pnr", pnr},
            {"booking", reservationToJson(cancelledRes)}
        };
        setJson(res, response, 200);
    });

    // -------------------------------------------------------------
    // API: GET /api/stats
    // Returns dashboard analytics calculated directly from backend data
    // -------------------------------------------------------------
    svr.Get("/api/stats", [](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_systemMutex);

        int totalTrains = g_railway.getTrainCount();
        int totalSeats = 0;
        int availableSeats = 0;

        for (int i = 0; i < totalTrains; i++) {
            Train* t = g_railway.getTrainByIndex(i);
            if (t) {
                totalSeats += t->getTotalSeats();
                availableSeats += t->getAvailableSeats();
            }
        }

        int totalReservations = g_railway.getReservationCount();
        int confirmedBookings = 0;
        int cancelledBookings = 0;
        json recent = json::array();

        for (int i = 0; i < totalReservations; i++) {
            Reservation* r = g_railway.getReservationByIndex(i);
            if (r) {
                if (r->isActive()) {
                    confirmedBookings++;
                } else {
                    cancelledBookings++;
                }
            }
        }

        // Return up to 5 most recent reservations (in reverse chronological order)
        int startIdx = totalReservations - 1;
        int collected = 0;
        while (startIdx >= 0 && collected < 5) {
            Reservation* r = g_railway.getReservationByIndex(startIdx);
            if (r) {
                recent.push_back(reservationToJson(r));
                collected++;
            }
            startIdx--;
        }

        json stats = {
            {"success", true},
            {"totalTrains", totalTrains},
            {"totalSeats", totalSeats},
            {"availableSeats", availableSeats},
            {"bookedSeats", totalSeats - availableSeats},
            {"totalReservations", totalReservations},
            {"confirmedBookings", confirmedBookings},
            {"cancelledBookings", cancelledBookings},
            {"recentReservations", recent}
        };

        setJson(res, stats, 200);
    });

    // -------------------------------------------------------------
    // Fallback for undefined API routes
    // -------------------------------------------------------------
    svr.Get(R"(/api/.*)", [](const httplib::Request& req, httplib::Response& res) {
        setJson(res, {
            {"success", false},
            {"error", "API endpoint not found: " + req.path}
        }, 404);
    });

    svr.Post(R"(/api/.*)", [](const httplib::Request& req, httplib::Response& res) {
        setJson(res, {
            {"success", false},
            {"error", "API endpoint not found: " + req.path}
        }, 404);
    });

    std::cout << "\n=======================================================\n";
    std::cout << "             RAILEASE RAILWAY RESERVATION SYSTEM        \n";
    std::cout << "                 C++ Backend REST Server               \n";
    std::cout << "=======================================================\n";
    std::cout << " [+] Server listening at : http://localhost:" << port << "\n";
    std::cout << " [+] Web Frontend root   : ./public/\n";
    std::cout << " [+] API Base URL        : http://localhost:" << port << "/api/\n";
    std::cout << " [+] Data Storage        : trains.txt, passengers.txt\n";
    std::cout << "=======================================================\n";
    std::cout << "Press Ctrl+C in console to terminate server.\n\n" << std::flush;

    if (!svr.listen("0.0.0.0", port)) {
        std::cerr << "[-] Error: Failed to bind to port " << port << ". Check if another instance is already running.\n";
        return 1;
    }

    return 0;
}
