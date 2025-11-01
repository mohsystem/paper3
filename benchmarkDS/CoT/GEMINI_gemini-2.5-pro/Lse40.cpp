/*
NOTE: This code requires the 'cpp-httplib' library.
It is a header-only library. To compile, you must download 'httplib.h'
and place it in the same directory or in your include path.
You can get it here: https://github.com/yhirose/cpp-httplib
Then, compile with a C++11 compliant compiler, like:
g++ -std=c++11 -o server Lse40.cpp -pthread
*/
#include <iostream>
#include <map>
#include <string>
#include <mutex>
#include <random>
#define CPPHTTPLIB_OPENSSL_SUPPORT // Uncomment for HTTPS support if needed
#include "httplib.h"

// --- In-memory Session Storage (for demonstration purposes) ---
// In a real application, use a proper session management system.
static std::map<std::string, std::map<std::string, std::string>> sessions;
static std::mutex session_mutex;

// Utility to generate a random session ID
std::string generate_session_id() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<unsigned long long> dis;
    return std::to_string(dis(gen));
}

// Utility to escape HTML to prevent XSS
std::string escape_html(const std::string& data) {
    std::string buffer;
    buffer.reserve(data.size());
    for(char c : data) {
        switch(c) {
            case '&':  buffer.append("&amp;");       break;
            case '\"': buffer.append("&quot;");      break;
            case '\'': buffer.append("&#x27;");      break;
            case '<':  buffer.append("&lt;");        break;
            case '>':  buffer.append("&gt;");        break;
            default:   buffer.push_back(c);         break;
        }
    }
    return buffer;
}

// The main logic function for the /info route
std::string get_info_page(const std::map<std::string, std::string>& session_data) {
    std::string username = session_data.count("username") ? session_data.at("username") : "N/A";
    std::string email = session_data.count("email") ? session_data.at("email") : "N/A";
    std::string ssn = session_data.count("ssn") ? session_data.at("ssn") : "0000";

    // SECURITY: Never store full SSN in session in a real application.
    // This is done here only to adhere to the prompt's requirements.
    std::string last4_ssn = (ssn.length() >= 4) ? ssn.substr(ssn.length() - 4) : ssn;
    
    // SECURITY: Escape user-provided data to prevent XSS attacks.
    std::string escaped_username = escape_html(username);
    std::string escaped_email = escape_html(email);

    std::string html =
        "<!DOCTYPE html>"
        "<html><head><title>User Information</title></head>"
        "<body>"
        "<h1>User Information</h1>"
        "<p><strong>Username:</strong> " + escaped_username + "</p>"
        "<p><strong>Email:</strong> " + escaped_email + "</p>"
        "<p><strong>Last 4 Digits of SSN:</strong> " + last4_ssn + "</p>"
        "<a href=\"/test\">Back to Test Cases</a>"
        "</body></html>";

    return html;
}

// In C++, the main class can be represented by the overall file/program structure.
// This is Lse40.cpp.
int main() {
    httplib::Server svr;

    // Route to simulate login and create a session
    svr.Get("/login", [](const httplib::Request& req, httplib::Response& res) {
        std::string user = req.has_param("user") ? req.get_param_value("user") : "";
        std::string email = req.has_param("email") ? req.get_param_value("email") : "";
        std::string ssn = req.has_param("ssn") ? req.get_param_value("ssn") : "";
        
        std::string session_id = generate_session_id();
        
        {
            std::lock_guard<std::mutex> lock(session_mutex);
            sessions[session_id] = {
                {"username", user},
                {"email", email},
                {"ssn", ssn}
            };
        }
        
        res.set_header("Set-Cookie", "sessionId=" + session_id + "; Path=/");
        res.set_redirect("/info");
    });

    // The main /info route
    svr.Get("/info", [](const httplib::Request& req, httplib::Response& res) {
        std::string session_id = req.get_header_value("Cookie");
        if (session_id.rfind("sessionId=", 0) == 0) {
            session_id.erase(0, 10); // Remove "sessionId="
        } else {
            session_id = "";
        }

        std::string html_content;
        int status_code = 401;

        std::lock_guard<std::mutex> lock(session_mutex);
        if (!session_id.empty() && sessions.count(session_id)) {
            html_content = get_info_page(sessions.at(session_id));
            status_code = 200;
        } else {
            html_content = "<h1>Unauthorized</h1><p>Please <a href='/test'>login</a> first.</p>";
        }
        
        res.set_content(html_content, "text/html");
        res.status = status_code;
    });

    // Route for testing
    svr.Get("/test", [](const httplib::Request& req, httplib::Response& res) {
        std::string html =
            "<!DOCTYPE html><html><head><title>Test Cases</title></head><body>"
            "<h1>Click a user to log in</h1><ul>"
            "<li><a href='/login?user=alice&email=alice@example.com&ssn=111223344'>Login as Alice</a></li>"
            "<li><a href='/login?user=bob&email=bob@example.com&ssn=555667788'>Login as Bob</a></li>"
            "<li><a href='/login?user=charlie<script>alert(1)</script>&email=charlie@bad.com&ssn=999887766'>Login as Charlie (XSS Test)</a></li>"
            "<li><a href='/login?user=dave&email=dave@example.com&ssn=123456789'>Login as Dave</a></li>"
            "<li><a href='/login?user=eve&email=eve@example.com&ssn=000000001'>Login as Eve</a></li>"
            "</ul></body></html>";
        res.set_content(html, "text/html");
    });
    
    int port = 8081;
    std::cout << "Server started on port " << port << std::endl;
    std::cout << "--- Test Cases ---" << std::endl;
    std::cout << "Visit http://localhost:" << port << "/test to run test cases." << std::endl;
    svr.listen("0.0.0.0", port);

    return 0;
}