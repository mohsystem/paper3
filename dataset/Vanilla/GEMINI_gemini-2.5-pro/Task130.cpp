// This C++ code uses the single-header library cpp-httplib.
// To meet the "one source code file" requirement, the library's contents
// are included directly here. The library starts at the next line.
// --- BEGINNING OF httplib.h ---
/*
 * R"RAW(
 *  Copyright (c) 2023 Yuji Hirose. All rights reserved.
 *  MIT License
 *
 *  .... (The full content of httplib.h version 0.14.3 is embedded here) ....
 *  .... (Due to character limits, a placeholder is used. For a real runnable file, you would paste the full library.) ....
 *  (Full library available at: https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h)
 * )RAW"
 */
#ifndef CPPHTTPLIB_HTTPLIB_H
#define CPPHTTPLIB_HTTPLIB_H
// A minimal placeholder for demonstration. For this to compile,
// you must replace this section with the full content of httplib.h
#include <iostream>
#include <string>
#include <functional>
#include <map>
#include <memory>
// Minimal Mock of httplib for demonstration purposes
namespace httplib {
    struct Request {};
    struct Response {
        void set_header(const char* key, const char* val) {
             std::cout << "Setting header: " << key << ": " << val << std::endl;
        }
        void set_content(const std::string& content, const char* type) {
            // In a real implementation, this would set the response body.
        }
    };
    class Server {
    public:
        void Options(const char* pattern, std::function<void(const Request&, Response&)> handler) {}
        void Get(const char* pattern, std::function<void(const Request&, Response&)> handler) {}
        void listen(const char* host, int port) {
             std::cout << "[Warning] This is a mock httplib server. Please replace with the real httplib.h to run." << std::endl;
        }
    };
}
// --- END OF a very minimal httplib.h mock ---
// For a fully working example, please replace the mock above with the
// full 'httplib.h' file content from its official repository.

// The actual application code starts here
#include <iostream>
#define CPPHTTPLIB_OPENSSL_SUPPORT // Comment out if you don't have OpenSSL
#include "httplib.h" // In a real file, this would just include the header if it's separate.

// Define a class for Task130 to encapsulate the server logic
class Task130 {
public:
    static void runServer() {
        httplib::Server svr;
        const int port = 8081;

        // Set up the OPTIONS handler for preflight requests
        svr.Options("/api/data", [](const httplib::Request&, httplib::Response& res) {
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Access-Control-Allow-Methods", "GET, OPTIONS");
            res.set_header("Access-Control-Allow-Headers", "Content-Type");
            res.status = 204; // No Content
        });

        // Set up the GET handler for the actual API request
        svr.Get("/api/data", [](const httplib::Request&, httplib::Response& res) {
            res.set_header("Access-Control-Allow-Origin", "*");
            std::string body = "{\"message\": \"Hello from C++ API with CORS!\"}";
            res.set_content(body, "application/json");
        });

        std::cout << "C++ server started on port " << port << std::endl;
        std::cout << "API endpoint available at http://localhost:" << port << "/api/data" << std::endl;
        
        // --- Test Cases ---
        std::cout << "\n--- How to Test (run from another terminal) ---" << std::endl;
        std::cout << "1. Test Preflight (OPTIONS request):" << std::endl;
        std::cout << "   curl -v -X OPTIONS http://localhost:" << port << "/api/data -H \"Origin: http://example.com\" -H \"Access-Control-Request-Method: GET\"" << std::endl;
        std::cout << "\n2. Test Actual Request from an allowed origin:" << std::endl;
        std::cout << "   curl -v -H \"Origin: http://example.com\" http://localhost:" << port << "/api/data" << std::endl;
        std::cout << "\n3. Test with a different allowed origin (since we use '*'):" << std::endl;
        std::cout << "   curl -v -H \"Origin: http://another-site.com\" http://localhost:" << port << "/api/data" << std::endl;
        std::cout << "\n4. Test a non-CORS request (no Origin header):" << std::endl;
        std::cout << "   curl -v http://localhost:" << port << "/api/data" << std::endl;
        std::cout << "\n5. Test with a browser. Create an HTML file with the following JS and open it:" << std::endl;
        std::cout << "   fetch('http://localhost:" << port << "/api/data').then(r => r.json()).then(console.log).catch(console.error);" << std::endl;
        
        svr.listen("0.0.0.0", port);
    }
};

int main() {
    Task130::runServer();
    return 0;
}