/*
NOTE: A secure, single-file C++ web application is not practical without external libraries.
This code depends on the 'cpp-httplib' single-header library.
You must download the 'httplib.h' file and place it in the same directory as this source file.
You can get it from: https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h

To compile:
g++ -o Lse41 Lse41.cpp -std=c++17 -pthread

This example demonstrates security principles like data masking and output encoding, but for a
production environment, you would use a more robust framework and ensure the application is
served over HTTPS.
*/
#include <iostream>
#include <string>
#include <memory>

// This line assumes httplib.h is in the same directory.
#include "httplib.h"

// A simple class to represent user data.
// In a real application, this would be retrieved from a secure database.
class User {
public:
    User(std::string username, std::string email, std::string ssn)
        : username_(std::move(username)), email_(std::move(email)), socialSecurityNumber_(std::move(ssn)) {}

    const std::string& getUsername() const { return username_; }
    const std::string& getEmail() const { return email_; }

    // Method to get only the last 4 digits of the SSN.
    std::string getLast4Ssn() const {
        if (socialSecurityNumber_.length() >= 4) {
            return socialSecurityNumber_.substr(socialSecurityNumber_.length() - 4);
        }
        return "****"; // Return a masked value if SSN is invalid.
    }

private:
    std::string username_;
    std::string email_;
    std::string socialSecurityNumber_; // Store the full SSN securely in a real app
};

// Utility function to escape HTML to prevent XSS (Cross-Site Scripting) attacks.
std::string escapeHtml(const std::string& text) {
    std::string result;
    result.reserve(text.size());
    for (char c : text) {
        switch (c) {
            case '&':  result += "&amp;";       break;
            case '\"': result += "&quot;";      break;
            case '\'': result += "&#x27;";      break;
            case '<':  result += "&lt;";        break;
            case '>':  result += "&gt;";        break;
            default:   result += c;             break;
        }
    }
    return result;
}

// Function to handle requests to the /info endpoint
void handleInfoRequest(const httplib::Request& req, httplib::Response& res, const User& currentUser) {
    // In a real application, you would check for user authentication and authorization here.
    
    // Securely escape user data before including it in the HTML response.
    std::string safeUsername = escapeHtml(currentUser.getUsername());
    std::string safeEmail = escapeHtml(currentUser.getEmail());
    std::string last4Ssn = currentUser.getLast4Ssn(); // Already safe as it's just digits

    std::string body = "<html>"
                       "<head><title>User Information</title></head>"
                       "<body>"
                       "<h1>User Information</h1>"
                       "<p><strong>Username:</strong> " + safeUsername + "</p>"
                       "<p><strong>Email:</strong> " + safeEmail + "</p>"
                       "<p><strong>Last 4 of SSN:</strong> ****-**-" + last4Ssn + "</p>"
                       "</body>"
                       "</html>";

    // Set secure headers. In a real application, you'd add more, like CSP.
    res.set_header("X-Content-Type-Options", "nosniff");
    res.set_content(body, "text/html; charset=UTF-8");
}


int main() {
    // --- Test Cases ---
    // Create shared pointers for user objects to manage their lifetime
    auto user1 = std::make_shared<User>("chris.smith", "chris.smith@example.com", "555-44-3333");
    auto user2 = std::make_shared<User>("<img src=x onerror=alert('xss')>", "badactor@example.com", "999-88-7777");
    auto user3 = std::make_shared<User>("test.user", "test@example.com", "123"); // Invalid SSN
    auto user4 = std::make_shared<User>("", "", ""); // Empty data
    auto user5 = std::make_shared<User>("final.user", "final@example.com", "000-11-2222");

    // --- Server Setup ---
    // Choose which user context to run the server with.
    // To test others, change `user1` to `user2`, `user3`, etc.
    auto currentUserForServer = user1;

    httplib::Server svr;

    // Define the handler for the /info route
    svr.Get("/info", [currentUserForServer](const httplib::Request& req, httplib::Response& res) {
        handleInfoRequest(req, res, *currentUserForServer);
    });
    
    // httplib automatically provides a 404 Not Found response for other routes.

    int port = 8080;
    std::cout << "Server starting on port " << port << std::endl;
    std::cout << "--- Testing Instructions ---" << std::endl;
    std::cout << "1. Open a web browser." << std::endl;
    std::cout << "2. Visit: http://localhost:" << port << "/info" << std::endl;
    std::cout << "   -> Expected: Page with info for user: " << currentUserForServer->getUsername() << std::endl;
    std::cout << "3. Visit: http://localhost:" << port << "/otherpage" << std::endl;
    std::cout << "   -> Expected: 404 Not Found" << std::endl;
    std::cout << "\n--- Important Security Note ---" << std::endl;
    std::cout << "This is an HTTP server. For production, use a reverse proxy (e.g., Nginx)" << std::endl;
    std::cout << "to handle HTTPS/TLS encryption." << std::endl;


    if (!svr.listen("0.0.0.0", port)) {
        std::cerr << "Failed to start server on port " << port << std::endl;
        return 1;
    }

    return 0;
}