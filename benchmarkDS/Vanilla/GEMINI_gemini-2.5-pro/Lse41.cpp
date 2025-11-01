/*
 * INSTRUCTIONS TO COMPILE AND RUN:
 * 1. This code requires the 'cpp-httplib' library. It's a header-only library.
 *    Download the 'httplib.h' file from the official repository:
 *    https://github.com/yhirose/cpp-httplib
 * 2. Place 'httplib.h' in the same directory as this source file.
 * 3. Compile the code using a C++11 compliant compiler (or newer):
 *    g++ -std=c++11 -o Lse41 Lse41.cpp -pthread
 * 4. Run the executable:
 *    ./Lse41
 */
#include <iostream>
#include "httplib.h" // Assumes httplib.h is in the same directory

// This function simulates the web application logic
void serveInfoPage(const httplib::Request &req, httplib::Response &res) {
    // Hardcoded user data
    std::string username = "johndoe";
    std::string email = "j.doe@example.com";
    std::string ssn_last_4 = "5678";
    
    std::string html_content =
        "<html>"
        "<head><title>User Information</title></head>"
        "<body>"
        "<h1>User Information</h1>"
        "<ul>"
        "<li><strong>Username:</strong> " + username + "</li>"
        "<li><strong>Email:</strong> " + email + "</li>"
        "<li><strong>SSN (Last 4 digits):</strong> " + ssn_last_4 + "</li>"
        "</ul>"
        "</body>"
        "</html>";

    res.set_content(html_content, "text/html");
}

int main(void) {
    // Create a server instance
    httplib::Server svr;

    // Register a handler for the "/info" route
    svr.Get("/info", serveInfoPage);

    int port = 1234;
    std::cout << "C++ server starting on port " << port << "..." << std::endl;
    std::cout << "Visit http://localhost:" << port << "/info to see the user information." << std::endl;
    
    // Test cases (instructions for manual testing)
    std::cout << "\n--- How to Test ---" << std::endl;
    std::cout << "Test Case 1: Open a web browser and navigate to http://localhost:1234/info" << std::endl;
    std::cout << "Test Case 2: Use curl in your terminal: curl http://localhost:1234/info" << std::endl;
    std::cout << "Test Case 3: Use a tool like Postman to send a GET request to http://localhost:1234/info" << std::endl;
    std::cout << "Test Case 4: Use PowerShell Invoke-WebRequest: Invoke-WebRequest -Uri http://localhost:1234/info" << std::endl;
    std::cout << "Test Case 5: Use Python requests library: python -c \"import requests; print(requests.get('http://localhost:1234/info').text)\"" << std::endl;

    // Start listening for connections
    if (!svr.listen("0.0.0.0", port)) {
        std::cerr << "Error: Server could not start on port " << port << std::endl;
        return 1;
    }

    return 0;
}