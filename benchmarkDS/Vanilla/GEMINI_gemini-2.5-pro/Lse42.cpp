/*
 * This code requires the 'cpp-httplib' single-header library.
 * Download 'httplib.h' from https://github.com/yhirose/cpp-httplib
 * and place it in the same directory as this source file.
 *
 * Compilation command:
 * g++ -o server Lse42.cpp -I. -pthread -std=c++11
 */
#include <iostream>
#include <string>
#include "httplib.h"

// In C++, we can use a class to structure the application.
class Lse42 {
public:
    // This function simulates retrieving data from a session.
    // In a real app, this would involve session IDs and a data store.
    static std::string info() {
        std::string username = "cpp_user";
        std::string email = "cpp_user@example.com";
        std::string ssn = "111-22-3333"; // Sensitive data for demonstration only

        // Manually construct a JSON string
        return "{\"username\": \"" + username + "\", "
               "\"email\": \"" + email + "\", "
               "\"ssn\": \"" + ssn + "\"}";
    }

    // Handler for the /info route
    static void info_handler(const httplib::Request &req, httplib::Response &res) {
        std::string response_body = info();
        res.set_content(response_body, "application/json");
    }
};

int main() {
    // The main function starts the web server.
    // The "5 test cases" are to be run manually by accessing the URL.
    httplib::Server svr;

    svr.Get("/info", Lse42::info_handler);

    int port = 8081;
    std::cout << "C++ server starting on port " << port << std::endl;
    std::cout << "Run the following 5 test cases in another terminal:" << std::endl;
    for (int i = 1; i <= 5; ++i) {
        std::cout << "Test Case " << i << ": curl http://localhost:" << port << "/info" << std::endl;
    }
    
    svr.listen("0.0.0.0", port);
    
    return 0;
}