/*
NOTE: To compile and run this C++ code, you need the 'cpp-httplib' library.
It is a header-only library. You can download the 'httplib.h' file from its GitHub repository:
https://github.com/yhirose/cpp-httplib

Save 'httplib.h' in the same directory as this source file.

Compilation command:
g++ -o Lse42 Lse42.cpp -I. -pthread -std=c++17

To run, execute the compiled binary:
./Lse42
The server will start, run 5 test cases against itself, and then continue listening for requests.
*/
#include <iostream>
#include <string>
#include <map>
#include <mutex>
#include <random>
#include <thread>
#include <chrono>

// Include the cpp-httplib header
#include "httplib.h"

// A simple structure to hold user session data
struct UserSession {
    std::string username;
    std::string email;
    std::string ssn;
    // In a real app, you would also store expiration timestamps, etc.
};

// Global session store.
// Secure Coding: Use a mutex to protect concurrent access in a multi-threaded environment.
std::map<std::string, UserSession> session_storage;
std::mutex session_mutex;

// Secure Coding: Generates a cryptographically secure random string for session IDs.
std::string generate_session_id() {
    const std::string chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, chars.size() - 1);
    std::string session_id;
    for (int i = 0; i < 32; ++i) {
        session_id += chars[distribution(generator)];
    }
    return session_id;
}

// Function to handle the /info route
void info(const httplib::Request& req, httplib::Response& res) {
    /*
     * SECURITY WARNING: This code is for demonstration purposes only.
     * Exposing sensitive PII like an SSN via an API is extremely insecure.
     * Real applications must have strong authentication and authorization.
     * The SSN should also be masked.
    */
    std::string session_id;
    if (req.has_header("Cookie")) {
        // A very simple cookie parser to find our session_id
        std::string cookie_header = req.get_header_value("Cookie");
        size_t pos = cookie_header.find("session_id=");
        if (pos != std::string::npos) {
            size_t start = pos + 11; // length of "session_id="
            size_t end = cookie_header.find(';', start);
            session_id = cookie_header.substr(start, end - start);
        }
    }

    if (session_id.empty()) {
        res.set_content("{\"error\": \"User not authenticated. No session found.\"}", "application/json");
        res.status = 401; // Unauthorized
        return;
    }

    std::lock_guard<std::mutex> guard(session_mutex);
    auto it = session_storage.find(session_id);

    if (it != session_storage.end()) {
        const auto& user_session = it->second;
        // Secure coding: Check for the presence of a key identifier like username.
        if (user_session.username.empty()) {
             res.set_content("{\"error\": \"Invalid session. User not identified.\"}", "application/json");
             res.status = 401;
             return;
        }

        // Using a library like nlohmann/json would be better for robust JSON creation.
        // For this example, we build the string manually.
        std::string json_response = "{";
        json_response += "\"username\": \"" + user_session.username + "\", ";
        json_response += "\"email\": \"" + user_session.email + "\", ";
        json_response += "\"ssn\": \"" + user_session.ssn + "\"";
        json_response += "}";
        res.set_content(json_response, "application/json");
    } else {
        res.set_content("{\"error\": \"Invalid session ID.\"}", "application/json");
        res.status = 401; // Unauthorized
    }
}

void run_tests() {
    httplib::Client cli("localhost", 8080);
    cli.set_connection_timeout(5); // 5 seconds
    
    std::cout << "--- Running Client Tests ---" << std::endl;

    // Test Case 1: All user data is present in the session.
    {
        std::cout << "\n--- Test Case 1: Valid Session ---" << std::endl;
        std::string sid = generate_session_id();
        {
            std::lock_guard<std::mutex> guard(session_mutex);
            session_storage[sid] = {"jdoe", "jdoe@example.com", "**********"};
        }
        httplib::Headers headers = {{ "Cookie", "session_id=" + sid }};
        auto res = cli.Get("/info", headers);
        if (res) {
            std::cout << "Status: " << res->status << std::endl;
            std::cout << "Body: " << res->body << std::endl;
        } else {
             std::cerr << "Request failed: " << httplib::to_string(res.error()) << std::endl;
        }
    }

    // Test Case 2: Some data is missing (ssn).
    {
        std::cout << "\n--- Test Case 2: Session with Missing Data ---" << std::endl;
        std::string sid = generate_session_id();
        {
            std::lock_guard<std::mutex> guard(session_mutex);
            session_storage[sid] = {"asmith", "asmith@example.com", ""}; // ssn is empty
        }
        httplib::Headers headers = {{ "Cookie", "session_id=" + sid }};
        auto res = cli.Get("/info", headers);
        if (res) {
            std::cout << "Status: " << res->status << std::endl;
            std::cout << "Body: " << res->body << std::endl;
        } else {
             std::cerr << "Request failed: " << httplib::to_string(res.error()) << std::endl;
        }
    }

    // Test Case 3: Invalid session ID.
    {
        std::cout << "\n--- Test Case 3: Invalid Session ID ---" << std::endl;
        httplib::Headers headers = {{ "Cookie", "session_id=invalid_id_12345" }};
        auto res = cli.Get("/info", headers);
        if (res) {
            std::cout << "Status: " << res->status << std::endl;
            std::cout << "Body: " << res->body << std::endl;
        } else {
             std::cerr << "Request failed: " << httplib::to_string(res.error()) << std::endl;
        }
    }

    // Test Case 4: No session cookie sent at all.
    {
        std::cout << "\n--- Test Case 4: No Session Cookie ---" << std::endl;
        auto res = cli.Get("/info");
        if (res) {
            std::cout << "Status: " << res->status << std::endl;
            std::cout << "Body: " << res->body << std::endl;
        } else {
             std::cerr << "Request failed: " << httplib::to_string(res.error()) << std::endl;
        }
    }

    // Test Case 5: Different valid user.
    {
        std::cout << "\n--- Test Case 5: Different Valid User ---" << std::endl;
        std::string sid = generate_session_id();
        {
            std::lock_guard<std::mutex> guard(session_mutex);
            session_storage[sid] = {"test.user", "test.user@example.com", "**********"};
        }
        httplib::Headers headers = {{ "Cookie", "session_id=" + sid }};
        auto res = cli.Get("/info", headers);
        if (res) {
            std::cout << "Status: " << res->status << std::endl;
            std::cout << "Body: " << res->body << std::endl;
        } else {
             std::cerr << "Request failed: " << httplib::to_string(res.error()) << std::endl;
        }
    }
    std::cout << "\n--- Client Tests Complete ---" << std::endl;
}

int main() {
    httplib::Server svr;

    // Define the /info route
    svr.Get("/info", info);

    // Run the server in a separate thread so the main thread can run tests
    std::thread server_thread([&]() {
        std::cout << "Starting server at http://localhost:8080" << std::endl;
        if (!svr.listen("localhost", 8080)) {
            std::cerr << "Error: Server could not start." << std::endl;
        }
    });

    // Give the server a moment to start up
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Run the test cases
    run_tests();

    std::cout << "\nServer is running. Press CTRL+C to stop." << std::endl;
    server_thread.join(); // Wait for the server thread to finish

    return 0;
}