/*
 * This C++ code creates a simple web server.
 * It uses two popular header-only libraries:
 * 1. cpp-httplib for the HTTP server.
 * 2. nlohmann/json for JSON parsing.
 *
 * How to compile and run:
 * 1. Make sure you have a C++ compiler (like g++).
 * 2. Download 'httplib.h' from: https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h
 * 3. Download 'json.hpp' from: https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp
 * 4. Place both header files in the same directory as this source file.
 * 5. Compile the code:
 *    g++ -std=c++11 -o server this_file.cpp -pthread
 * 6. Run the executable:
 *    ./server
 * 7. The server will start on http://localhost:8080.
 */
#include <iostream>
#include <string>
#include "httplib.h"
#include "json.hpp"

using json = nlohmann::json;

void handle_api_request(const httplib::Request& req, httplib::Response& res) {
    if (!req.has_param("payload")) {
        res.status = 400;
        res.set_content("'payload' parameter is missing", "text/plain");
        return;
    }
    
    std::string payload_str = req.get_param_value("payload");
    
    try {
        json data = json::parse(payload_str);
        
        if (!data.is_object()) {
            res.status = 400;
            res.set_content("Payload must be a JSON object.", "text/plain");
            return;
        }

        if (!data.contains("name") || !data["name"].is_string()) {
            res.status = 400;
            res.set_content("'name' key is missing or not a string in JSON payload", "text/plain");
            return;
        }
        
        std::string name = data["name"].get<std::string>();
        res.status = 200;
        res.set_content(name, "text/plain; charset=utf-8");

    } catch (const json::parse_error& e) {
        res.status = 400;
        res.set_content("Invalid JSON format in 'payload'", "text/plain");
    } catch (const std::exception& e) {
        res.status = 500;
        res.set_content("An internal server error occurred", "text/plain");
    }
}

int main() {
    httplib::Server svr;
    const int port = 8080;

    svr.Get("/api", handle_api_request);

    std::cout << "Server starting on http://localhost:" << port << std::endl;
    std::cout << "Use curl to test the API endpoint:" << std::endl;
    
    // Test cases (to be executed from a separate terminal):
    // 1. Valid request:
    std::cout << "curl 'http://localhost:" << port << "/api?payload=%7B%22name%22%3A%22world%22%7D'" << std::endl;
    // 2. Missing 'payload' parameter:
    std::cout << "curl 'http://localhost:" << port << "/api'" << std::endl;
    // 3. Invalid JSON in 'payload':
    std::cout << "curl 'http://localhost:" << port << "/api?payload=invalid-json'" << std::endl;
    // 4. 'name' key missing in JSON:
    std::cout << "curl 'http://localhost:" << port << "/api?payload=%7B%22other_key%22%3A%22value%22%7D'" << std::endl;
    // 5. 'name' value is not a string:
    std::cout << "curl 'http://localhost:" << port << "/api?payload=%7B%22name%22%3A123%7D'" << std::endl;
    
    if (!svr.listen("0.0.0.0", port)) {
        std::cerr << "Failed to start server on port " << port << std::endl;
        return 1;
    }

    return 0;
}