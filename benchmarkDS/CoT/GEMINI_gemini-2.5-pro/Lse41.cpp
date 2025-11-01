/*
 * NOTE: C++ does not have a built-in standard HTTP server library.
 * This code requires a third-party, header-only library called 'cpp-httplib'.
 * You must download the 'httplib.h' header file and place it in the same
 * directory as this source file or in your include path.
 *
 * You can get it from here:
 * https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h
 */
#include <iostream>
#include "httplib.h"
#include <string>

// A simple function to demonstrate the principle of HTML escaping to prevent XSS.
// For a real application, a more robust library should be used.
std::string escapeHtml(const std::string& data) {
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


// In C++, we use a class to encapsulate the server logic.
class Lse41 {
public:
    void run() {
        httplib::Server svr;

        svr.Get("/info", [](const httplib::Request &, httplib::Response &res) {
            // =================================================================
            // SECURITY WARNING: EXPOSING SENSITIVE USER INFORMATION
            // =================================================================
            // This code demonstrates handling a web request but follows a highly
            // insecure pattern demanded by the prompt. In a real-world application,
            // you should NEVER expose sensitive data like email or SSN details
            // without proper, robust authentication and authorization.
            //
            // CWE-200: Exposure of Sensitive Information to an Unauthorized Actor.
            //
            // Using placeholder data here to avoid hardcoding real PII (CWE-798).
            // =================================================================
            std::string username = "testuser";
            std::string email = "user@example.com";
            std::string ssn_last_4 = "XXXX";

            // Escape data to prevent XSS (CWE-79)
            std::string safe_username = escapeHtml(username);
            std::string safe_email = escapeHtml(email);

            std::string html_body = 
                "<html>"
                "<body>"
                "<h1>User Information</h1>"
                "<p><b>Username:</b> " + safe_username + "</p>"
                "<p><b>Email:</b> " + safe_email + "</p>"
                "<p><b>SSN (Last 4 digits):</b> " + ssn_last_4 + "</p>"
                "<hr><p style='color:red;'><b>Security Note:</b> This page is for demonstration only and exposes sensitive information, which is a major security risk.</p>"
                "</body>"
                "</html>";
            
            res.set_content(html_body, "text/html");
        });

        int port = 8081;
        std::cout << "Server starting on port " << port << std::endl;
        std::cout << "--- Test Cases ---" << std::endl;
        std::cout << "1. Open a web browser and navigate to: http://localhost:" << port << "/info" << std::endl;
        std::cout << "2. Use curl in your terminal: curl http://localhost:" << port << "/info" << std::endl;
        std::cout << "3. Test a non-existent route (should give 404): http://localhost:" << port << "/test" << std::endl;
        std::cout << "4. Test the root route (should give 404): http://localhost:" << port << "/" << std::endl;
        std::cout << "5. Press CTRL+C in this terminal to stop the server." << std::endl;
        
        svr.listen("0.0.0.0", port);
    }
};

int main() {
    Lse41 web_app;
    web_app.run();
    return 0;
}