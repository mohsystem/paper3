/*
 * IMPORTANT: This code requires the 'cpp-httplib' single-header library.
 * Download 'httplib.h' from https://github.com/yhirose/cpp-httplib
 * and place it in the same directory as this source file.
 *
 * Compilation command:
 * g++ -std=c++17 Lse40.cpp -o Lse40 -pthread
 * or
 * clang++ -std=c++17 Lse40.cpp -o Lse40 -pthread
 */
#define CPPHTTPLIB_OPENSSL_SUPPORT // Comment out if you don't have OpenSSL
#include "httplib.h"
#include <iostream>
#include <map>
#include <string>
#include <mutex>
#include <random>
#include <thread>
#include <chrono>

// In-memory store for session data.
// Key: sessionID, Value: user data map
std::map<std::string, std::map<std::string, std::string>> sessions;
std::mutex session_mutex;

std::string generate_session_id() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<unsigned long long> dis;
    return std::to_string(dis(gen));
}

void info(const httplib::Request& req, httplib::Response& res) {
    std::string session_id;
    if (req.has_header("Cookie")) {
        std::string cookie_header = req.get_header_value("Cookie");
        // Simple parsing for "session_id=..."
        size_t pos = cookie_header.find("session_id=");
        if (pos != std::string::npos) {
            session_id = cookie_header.substr(pos + 11);
        }
    }

    std::lock_guard<std::mutex> lock(session_mutex);
    if (!session_id.empty() && sessions.count(session_id)) {
        const auto& user_data = sessions[session_id];
        std::string username = user_data.at("username");
        std::string email = user_data.at("email");
        std::string ssn = user_data.at("ssn");
        std::string last4_ssn = ssn.substr(ssn.length() - 4);

        char buffer[512];
        snprintf(buffer, sizeof(buffer),
            "<html><body>"
            "<h1>User Information</h1>"
            "<p>Username: %s</p>"
            "<p>Email: %s</p>"
            "<p>Last 4 digits of SSN: %s</p>"
            "</body></html>",
            username.c_str(), email.c_str(), last4_ssn.c_str());
        res.set_content(buffer, "text/html");
    } else {
        res.status = 401; // Unauthorized
        res.set_content(
            "<html><body><h1>Access Denied</h1><p>Please <a href='/login'>login</a> first.</p></body></html>",
            "text/html");
    }
}

// Helper route to set session data for testing
void login(const httplib::Request& req, httplib::Response& res) {
    std::string session_id = generate_session_id();
    
    {
        std::lock_guard<std::mutex> lock(session_mutex);
        sessions[session_id] = {
            {"username", "cpp_user"},
            {"email", "cpp.user@example.com"},
            {"ssn", "555-444-3322"}
        };
    }

    res.set_header("Set-Cookie", "session_id=" + session_id + "; Path=/");
    res.set_content(
        "<html><body><h1>Login Successful</h1><p>Go to <a href='/info'>/info</a>.</p></body></html>",
        "text/html");
}

void run_tests(int port) {
    std::cout << "Running 5 test cases..." << std::endl;
    httplib::Client cli("localhost", port);
    cli.set_connection_timeout(5, 0); // 5 seconds

    // Test Case 1: Access /info without a session
    std::cout << "\n--- Test Case 1: Access /info without session ---" << std::endl;
    auto res1 = cli.Get("/info");
    if (res1 && res1->status == 401) {
        std::cout << "OK: Received 401 Access Denied." << std::endl;
    } else {
        std::cerr << "FAIL: Did not get 401. Status: " << (res1 ? res1->status : -1) << std::endl;
    }

    // Test Case 2: Access /login to get a session cookie
    std::cout << "\n--- Test Case 2: Access /login to create session ---" << std::endl;
    auto res2 = cli.Get("/login");
    std::string cookie;
    if (res2 && res2->has_header("Set-Cookie")) {
        cookie = res2->get_header_value("Set-Cookie");
        // The client needs only the "key=value" part.
        cookie = cookie.substr(0, cookie.find(';')); 
        std::cout << "OK: Logged in and received cookie." << std::endl;
    } else {
        std::cerr << "FAIL: Did not receive cookie." << std::endl;
    }

    // Test Case 3: Access /info with the session cookie
    std::cout << "\n--- Test Case 3: Access /info with session ---" << std::endl;
    httplib::Headers headers = {{ "Cookie", cookie }};
    auto res3 = cli.Get("/info", headers);
    if (res3 && res3->status == 200 && res3->body.find("cpp_user") != std::string::npos) {
        std::cout << "OK: Displayed user's info." << std::endl;
    } else {
        std::cerr << "FAIL: Could not retrieve user info." << std::endl;
    }

    // Test Case 4: Simulate clearing session (like a server restart)
    std::cout << "\n--- Test Case 4: Test with an invalid/expired session ---" << std::endl;
    {
        std::lock_guard<std::mutex> lock(session_mutex);
        sessions.clear();
    }
    auto res4 = cli.Get("/info", headers); // Use the same old cookie
    if (res4 && res4->status == 401) {
        std::cout << "OK: Received 401 for expired session." << std::endl;
    } else {
        std::cerr << "FAIL: Did not get 401 for expired session." << std::endl;
    }

    // Test Case 5: Log in again and verify access
    std::cout << "\n--- Test Case 5: Re-login and verify access ---" << std::endl;
    auto res5_login = cli.Get("/login");
    std::string cookie2 = res5_login->get_header_value("Set-Cookie");
    cookie2 = cookie2.substr(0, cookie2.find(';'));
    httplib::Headers headers2 = {{ "Cookie", cookie2 }};
    auto res5_info = cli.Get("/info", headers2);
    if (res5_info && res5_info->status == 200) {
        std::cout << "OK: Successfully re-logged in and accessed info." << std::endl;
    } else {
        std::cerr << "FAIL: Failed to access info after re-login." << std::endl;
    }
}

int main() {
    httplib::Server svr;

    svr.Get("/info", info);
    svr.Get("/login", login);

    int port = 8080;
    
    // Start server in a background thread
    std::thread server_thread([&]() {
        std::cout << "Server started on port " << port << std::endl;
        if (!svr.listen("localhost", port)) {
            std::cerr << "Error starting server." << std::endl;
        }
    });

    // Give the server a moment to start up
    std::this_thread::sleep_for(std::chrono::seconds(1));

    run_tests(port);

    std::cout << "\nTests finished. Stopping server..." << std::endl;
    svr.stop();
    server_thread.join();
    std::cout << "Server stopped." << std::endl;

    return 0;
}