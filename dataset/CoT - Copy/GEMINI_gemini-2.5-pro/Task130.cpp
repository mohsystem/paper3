/*
 * NOTE: This code requires the 'cpp-httplib' single-file header library.
 * Download 'httplib.h' from https://github.com/yhirose/cpp-httplib
 * and place it in the same directory or in your include path.
 *
 * COMPILE with: g++ -o task130 task130.cpp -I. -pthread -std=c++17
 */
#include <iostream>
#include <set>
#include <string>
#include "httplib.h"

// Whitelist of allowed origins for CORS
const std::set<std::string> ALLOWED_ORIGINS = {
    "http://localhost:3000",
    "https://safe.example.com"
};

// Middleware-like function to handle CORS headers
void set_cors_headers(const httplib::Request& req, httplib::Response& res) {
    if (req.has_header("Origin")) {
        const std::string& origin = req.get_header_value("Origin");
        // Secure CORS: Check if the origin is in the whitelist
        if (ALLOWED_ORIGINS.count(origin)) {
            res.set_header("Access-Control-Allow-Origin", origin);
            res.set_header("Access-Control-Allow-Methods", "GET, OPTIONS");
            res.set_header("Access-Control-Allow-Headers", "Content-Type, X-Requested-With");
        }
    }
}

int main() {
    httplib::Server svr;
    int port = 8082;

    // Handler for preflight OPTIONS request
    svr.Options("/api/data", [](const httplib::Request& req, httplib::Response& res) {
        set_cors_headers(req, res);
        res.status = 204; // No Content
    });

    // Handler for actual GET API request
    svr.Get("/api/data", [](const httplib::Request& req, httplib::Response& res) {
        set_cors_headers(req, res);
        std::string json_response = "{\"message\": \"Hello from C++ CORS API!\"}";
        res.set_content(json_response, "application/json");
    });
    
    svr.set_error_handler([](const httplib::Request&, httplib::Response& res) {
        res.set_content("Not Found", "text/plain");
        res.status = 404;
    });

    std::cout << "C++ server started on port " << port << std::endl;
    std::cout << "Run the test cases below in a new terminal." << std::endl;
    std::cout << "\n--- Test Cases (run with curl) ---" << std::endl;
    std::cout << "1. Preflight OPTIONS from allowed origin (should succeed with CORS headers):\n"
              << "   curl -v -X OPTIONS http://localhost:8082/api/data -H \"Origin: https://safe.example.com\" -H \"Access-Control-Request-Method: GET\"" << std::endl;
    std::cout << "\n2. GET from allowed origin (should succeed with data and CORS header):\n"
              << "   curl -v http://localhost:8082/api/data -H \"Origin: https://safe.example.com\"" << std::endl;
    std::cout << "\n3. GET from disallowed origin (should get data but NO CORS header):\n"
              << "   curl -v http://localhost:8082/api/data -H \"Origin: http://evil.com\"" << std.endl;
    std::cout << "\n4. GET with no Origin header (should get data but NO CORS header):\n"
              << "   curl -v http://localhost:8082/api/data" << std::endl;
    std::cout << "\n5. GET request to a non-existent path (should get 404):\n"
              << "   curl -v http://localhost:8082/other/path -H \"Origin: https://safe.example.com\"" << std::endl;

    svr.listen("0.0.0.0", port);
    
    return 0;
}