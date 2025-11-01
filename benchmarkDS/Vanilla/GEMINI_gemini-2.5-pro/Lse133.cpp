/*
 * NOTE: This code requires the cpp-httplib library.
 * It is a header-only library, so you just need to download httplib.h
 * and place it in the same directory or in your compiler's include path.
 * You can get it from: https://github.com/yhirose/cpp-httplib
 *
 * Suggested compilation command:
 * g++ -o your_program_name Lse133.cpp -pthread -std=c++11
 */
#include <iostream>
#include <string>
#include <vector>

// 1. The first line imports the web application class (via including the header).
#include "httplib.h"

// A simple function that accepts a parameter and returns output.
std::string createGreeting(const std::string& name) {
    return "Hello, " + name + "!";
}

int main() {
    std::cout << "--- Running 5 test cases for the helper function ---" << std::endl;
    std::vector<std::string> testCases = {"Alice", "Bob", "Charlie", "David", "Eve"};
    for (size_t i = 0; i < testCases.size(); ++i) {
        std::cout << "Test Case " << i + 1 << ": createGreeting(\"" << testCases[i] 
                  << "\") -> \"" << createGreeting(testCases[i]) << "\"" << std::endl;
    }
    std::cout << "--------------------------------------------------\n" << std::endl;

    // 2. The second line creates the application (server) object.
    httplib::Server svr;

    // 3. This line tells the application which URL should call the associated function.
    // 4. The function (a C++ lambda) is called when the user visits the specified URL.
    svr.Get("/", [](const httplib::Request &, httplib::Response &res) {
        // 5. This line returns the message to the user.
        res.set_content("Hello from C++ with cpp-httplib!", "text/plain");
    });

    std::cout << "C++ HTTP server started on port 8080." << std::endl;
    std::cout << "Open your web browser and navigate to http://localhost:8080" << std::endl;

    // 6. This line runs the application. It will block here, listening for connections.
    svr.listen("0.0.0.0", 8080);

    return 0;
}