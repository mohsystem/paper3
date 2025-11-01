/*
 * This code requires the cpp-httplib library.
 * It's a header-only library. You can get it from:
 * https://github.com/yhirose/cpp-httplib
 * Just download the 'httplib.h' file and place it in the same directory
 * as this source file.
 *
 * Compilation command:
 * g++ -std=c++17 -o server Lse40.cpp -pthread
 *
 * To run:
 * ./server
 */
#include <iostream>
#include <string>
#include <unordered_map>
#include <mutex>
#include <random>
#include <sstream>
#include <iomanip>
#include <thread>
#include <chrono>

#define CPPHTTPLIB_OPENSSL_SUPPORT // Uncomment for HTTPS support if OpenSSL is installed
#include "httplib.h"

// Structure to hold user data
struct UserData {
    std::string username;
    std::string email;
    std::string ssn;
};

// Thread-safe session store
std::unordered_map<std::string, UserData> session_store;
std::mutex session_mutex;

// Function to generate a cryptographically secure random session ID
std::string generate_session_id() {
    std::stringstream ss;
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;

    for (int i = 0; i < 4; ++i) {
        ss << std::hex << std::setw(16) << std::setfill('0') << dis(gen);
    }
    return ss.str();
}

// Simple HTML escaping function to prevent XSS
std::string html_escape(const std::string& data) {
    std::string buffer;
    buffer.reserve(data.size());
    for(size_t pos = 0; pos != data.size(); ++pos) {
        switch(data[pos]) {
            case '&':  buffer.append("&amp;");       break;
            case '\"': buffer.append("&quot;");      break;
            case '\'': buffer.append("&apos;");      break;
            case '<':  buffer.append("&lt;");        break;
            case '>':  buffer.append("&gt;");        break;
            default:   buffer.append(&data[pos], 1); break;
        }
    }
    return buffer;
}


void setup_server(httplib::Server& svr) {
    // A dummy login route to create a session
    svr.Get("/login", [](const httplib::Request&, httplib::Response& res) {
        std::string session_id = generate_session_id();
        {
            std::lock_guard<std::mutex> lock(session_mutex);
            session_store[session_id] = {"testuser", "test@example.com", "123-456-7890"};
        }
        res.set_header("Set-Cookie", "session_id=" + session_id + "; Path=/; HttpOnly");
        res.set_redirect("/info");
    });

    // The main /info route
    svr.Get("/info", [](const httplib::Request& req, httplib::Response& res) {
        if (!req.has_header("Cookie")) {
            res.set_content("<h1>Not Logged In</h1><p>Please <a href=\"/login\">login</a> first.</p>", "text/html");
            res.status = 401;
            return;
        }

        auto val = req.get_header_value("Cookie");
        auto pos = val.find("session_id=");
        if (pos == std::string::npos) {
            res.set_content("<h1>Not Logged In</h1><p>No session ID found. Please <a href=\"/login\">login</a>.</p>", "text/html");
            res.status = 401;
            return;
        }

        std::string session_id = val.substr(pos + 11);
        auto semi_pos = session_id.find(';');
        if (semi_pos != std::string::npos) {
            session_id = session_id.substr(0, semi_pos);
        }

        std::string html_content;
        {
            std::lock_guard<std::mutex> lock(session_mutex);
            if (session_store.count(session_id)) {
                const auto& user_data = session_store.at(session_id);
                std::string ssn = user_data.ssn;
                std::string last4 = ssn.length() >= 4 ? ssn.substr(ssn.length() - 4) : ssn;

                html_content = "<html><body>"
                               "<h1>User Information</h1>"
                               "<p>Username: " + html_escape(user_data.username) + "</p>"
                               "<p>Email: " + html_escape(user_data.email) + "</p>"
                               "<p>SSN (last 4 digits): ****-**-" + html_escape(last4) + "</p>"
                               "</body></html>";
                res.set_content(html_content, "text/html");
            } else {
                html_content = "<h1>Invalid Session</h1><p>Please <a href=\"/login\">login</a> again.</p>";
                res.set_content(html_content, "text/html");
                res.status = 401;
            }
        }
    });
}

void run_tests() {
    httplib::Client cli("localhost", 8080);
    cli.set_connection_timeout(5);

    std::cout << "\n--- Test Case 1: Access /info without session ---" << std::endl;
    auto res1 = cli.Get("/info");
    if (res1 && res1->status == 401) {
        std::cout << "Result: OK (Status 401 as expected)" << std::endl;
    } else {
        std::cout << "Result: FAILED" << std::endl;
    }

    std::cout << "\n--- Test Case 2: Access /login to establish session ---" << std::endl;
    auto res2 = cli.Get("/login");
    std::string session_cookie;
    if (res2 && res2->status == 302) {
        std::cout << "Result: OK (Redirected after login)" << std::endl;
        session_cookie = res2->get_header_value("Set-Cookie");
        // Extract just the name=value part for subsequent requests
        auto semi_pos = session_cookie.find(';');
        if (semi_pos != std::string::npos) {
            session_cookie = session_cookie.substr(0, semi_pos);
        }
    } else {
        std::cout << "Result: FAILED" << std::endl;
    }

    httplib::Headers headers = { {"Cookie", session_cookie} };

    std::cout << "\n--- Test Case 3: Access /info with session ---" << std::endl;
    auto res3 = cli.Get("/info", headers);
    if (res3 && res3->status == 200 && res3->body.find("testuser") != std::string::npos) {
        std::cout << "Result: OK (Successfully fetched user info)" << std::endl;
    } else {
        std::cout << "Result: FAILED" << std::endl;
    }

    std::cout << "\n--- Test Case 4: Verify SSN masking ---" << std::endl;
    if (res3 && res3->body.find("****-**-7890") != std::string::npos && res3->body.find("123-456-7890") == std::string::npos) {
        std::cout << "Result: OK (SSN is masked correctly)" << std::endl;
    } else {
        std::cout << "Result: FAILED" << std::endl;
    }
    
    std::cout << "\n--- Test Case 5: Access /info with invalid session ---" << std::endl;
    httplib::Headers bad_headers = { {"Cookie", "session_id=invalid123"} };
    auto res5 = cli.Get("/info", bad_headers);
    if (res5 && res5->status == 401) {
        std::cout << "Result: OK (Rejected invalid session)" << std::endl;
    } else {
        std::cout << "Result: FAILED" << std::endl;
    }
}

int main() {
    httplib::Server svr;
    setup_server(svr);

    std::thread server_thread([&]() {
        std::cout << "Server starting on http://localhost:8080" << std::endl;
        if (!svr.listen("localhost", 8080)) {
            std::cerr << "Error starting server" << std::endl;
        }
    });
    
    // Give the server a moment to start up
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "Running tests..." << std::endl;
    run_tests();
    std::cout << "\nTests finished." << std::endl;

    svr.stop();
    server_thread.join();
    
    return 0;
}