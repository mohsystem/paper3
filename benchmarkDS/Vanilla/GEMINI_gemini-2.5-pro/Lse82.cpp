/*
 * Instructions:
 * 1. This code requires two single-header libraries:
 *    - cpp-httplib: Download 'httplib.h' from https://github.com/yhirose/cpp-httplib
 *    - nlohmann/json: Download 'json.hpp' from https://github.com/nlohmann/json
 * 2. Place both `httplib.h` and `json.hpp` in the same directory as this source file.
 * 3. Compile with a C++11 compliant compiler (or newer). Enable threads.
 *    Example using g++:
 *    g++ -std=c++17 -pthread -o web_app Lse82.cpp
 * 4. Run the compiled executable:
 *    ./web_app
*/
#include <iostream>
#include "httplib.h"
#include "json.hpp"

// for convenience
using json = nlohmann::json;

class Lse82 {
public:
    static void handleApiRequest(const httplib::Request& req, httplib::Response& res) {
        // It gets the payload parameter from the URL.
        if (!req.has_param("payload")) {
            res.status = 400;
            res.set_content("Payload parameter is missing.", "text/plain");
            return;
        }

        std::string payload_str = req.get_param_value("payload");
        
        // It loads the payload into a dictionary.
        json data;
        try {
            data = json::parse(payload_str);
        } catch (json::parse_error& e) {
            res.status = 400;
            res.set_content("Invalid JSON in payload parameter.", "text/plain");
            return;
        }

        // It returns the name value from the dictionary.
        if (data.is_object() && data.contains("name") && data["name"].is_string()) {
            std::string name = data["name"];
            res.set_content(name, "text/plain");
        } else {
            res.status = 400;
            res.set_content("'name' key (as a string) not found in payload.", "text/plain");
        }
    }
};

int main() {
    httplib::Server svr;

    svr.Get("/api", Lse82::handleApiRequest);

    int port = 8080;
    std::cout << "Server started on port " << port << std::endl;
    std::cout << "Use Ctrl+C to stop the server." << std::endl;
    std::cout << "\n--- Test Cases ---" << std::endl;
    std::cout << "Run the following commands in your terminal:" << std::endl;
    std::cout << "1) Test Case 1 (Standard):" << std::endl;
    std::cout << "   curl \"http://localhost:8080/api?payload=%7B%22name%22%3A%22JohnDoe%22%2C%22age%22%3A30%7D\"" << std::endl;
    std::cout << "   Expected output: JohnDoe\n" << std::endl;
    std::cout << "2) Test Case 2 (Different Name):" << std::endl;
    std::cout << "   curl \"http://localhost:8080/api?payload=%7B%22name%22%3A%22JaneSmith%22%7D\"" << std::endl;
    std::cout << "   Expected output: JaneSmith\n" << std::endl;
    std::cout << "3) Test Case 3 (No payload parameter):" << std::endl;
    std::cout << "   curl \"http://localhost:8080/api\"" << std::endl;
    std::cout << "   Expected output: Payload parameter is missing.\n" << std::endl;
    std::cout << "4) Test Case 4 (Payload without name key):" << std::endl;
    std::cout << "   curl \"http://localhost:8080/api?payload=%7B%22city%22%3A%22NewYork%22%7D\"" << std::endl;
    std::cout << "   Expected output: 'name' key (as a string) not found in payload.\n" << std::endl;
    std::cout << "5) Test Case 5 (Malformed payload):" << std::endl;
    std::cout << "   curl \"http://localhost:8080/api?payload=thisisnotjson\"" << std::endl;
    std::cout << "   Expected output: Invalid JSON in payload parameter.\n" << std::endl;
    
    svr.listen("0.0.0.0", port);

    return 0;
}