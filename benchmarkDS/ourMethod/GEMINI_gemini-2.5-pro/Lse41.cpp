/*
 * This code requires the 'httplib.h' single-file header library.
 * You can download it from: https://github.com/yhirose/cpp-httplib
 * Place `httplib.h` in the same directory or in your include path.
 *
 * Compilation command:
 * g++ -o server this_file.cpp -I. -pthread -std=c++17
 *
 * In a production environment, this application should be run behind a reverse proxy
 * (like Nginx or Apache) that handles TLS (HTTPS) termination. Do not expose this
 * HTTP server directly to the internet.
 */
#include <iostream>
#include <string>
#include <cstdlib> // For std::getenv
#include <memory>
// The following line assumes 'httplib.h' is in the include path.
#include "httplib.h"

// Helper function to securely escape HTML content to prevent XSS.
std::string escape_html(const std::string& text) {
    std::string result;
    result.reserve(text.size());
    for (char c : text) {
        switch (c) {
            case '&':  result.append("&amp;");       break;
            case '\"': result.append("&quot;");      break;
            case '\'': result.append("&#39;");      break;
            case '<':  result.append("&lt;");        break;
            case '>':  result.append("&gt;");        break;
            default:   result.push_back(c);         break;
        }
    }
    return result;
}

void info_handler(const httplib::Request &req, httplib::Response &res) {
    // Rule #2: Avoid hardcoding credentials. Use environment variables.
    const char* username_env = std::getenv("USER_INFO_USERNAME");
    const char* email_env = std::getenv("USER_INFO_EMAIL");
    const char* ssn_env = std::getenv("USER_INFO_SSN");
    
    if (!username_env || !email_env || !ssn_env) {
        std::cerr << "Error: Required environment variables (USER_INFO_USERNAME, USER_INFO_EMAIL, USER_INFO_SSN) are not set." << std::endl;
        res.status = 500;
        res.set_content("Internal Server Error: Server is not configured correctly.", "text/plain");
        return;
    }

    std::string ssn(ssn_env);
    if (ssn.length() < 4) {
        std::cerr << "Error: USER_INFO_SSN environment variable is too short." << std::endl;
        res.status = 500;
        res.set_content("Internal Server Error: Invalid configuration.", "text/plain");
        return;
    }
    
    std::string last_4_ssn = ssn.substr(ssn.length() - 4);
    std::string username = escape_html(username_env);
    std::string email = escape_html(email_env);

    std::string html_content = 
        "<!DOCTYPE html>"
        "<html>"
        "<head><title>User Information</title></head>"
        "<body>"
        "<h1>User Details</h1>"
        "<p><strong>Username:</strong> " + username + "</p>"
        "<p><strong>Email:</strong> " + email + "</p>"
        "<p><strong>SSN (Last 4 digits):</strong> XXX-XX-" + last_4_ssn + "</p>"
        "</body>"
        "</html>";

    res.set_content(html_content, "text/html; charset=utf-8");
}


int main() {
    httplib::Server svr;

    svr.Get("/info", info_handler);

    int port = 8080;
    std::cout << "Server started on port " << port << std::endl;
    std::cout << "Access the info page at http://localhost:" << port << "/info" << std::endl;
    std::cout << "Make sure to set the required environment variables before running." << std::endl;
    std::cout << "Example (Linux/macOS):" << std::endl;
    std::cout << "export USER_INFO_USERNAME=\"testuser\"" << std::endl;
    std::cout << "export USER_INFO_EMAIL=\"test@example.com\"" << std::endl;
    std::cout << "export USER_INFO_SSN=\"123456789\"" << std::endl;
    std::cout << "./server" << std::endl;
    std::cout << "\n--- Test Cases ---" << std::endl;
    std::cout << "1. All env vars set correctly: Access /info, should see user data." << std::endl;
    std::cout << "2. One or more env vars missing: Access /info, should see a 500 error." << std::endl;
    std::cout << "3. SSN is less than 4 digits: Access /info, should see a 500 error." << std::endl;
    std::cout << "4. Access a different path (e.g., /): Should see a 404 error." << std::endl;
    std::cout << "5. Use a different method (e.g., POST to /info): Should see a 405 error (Method Not Allowed)." << std::endl;
    
    if (!svr.listen("0.0.0.0", port)) {
        std::cerr << "Failed to start server on port " << port << std::endl;
        return 1;
    }
    
    return 0;
}