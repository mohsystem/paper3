/*
 * Dependencies:
 * 1. cpp-httplib: A header-only cross-platform HTTP/HTTPS library.
 *    Download httplib.h from https://github.com/yhirose/cpp-httplib
 * 2. yaml-cpp: A YAML parser and emitter in C++.
 *    Install via a package manager (e.g., `sudo apt-get install libyaml-cpp-dev` on Debian/Ubuntu,
 *    `brew install yaml-cpp` on macOS).
 *
 * Compilation:
 * g++ -std=c++17 -o server Lse84.cpp -I/path/to/httplib/include -lyaml-cpp -lpthread
 *
 * Example (assuming httplib.h is in the current directory):
 * g++ -std=c++17 -o server Lse84.cpp -lyaml-cpp -lpthread
 */

#include <iostream>
#include <string>
#include <utility>

// Download this header from the official repository and place it in your project.
#include "httplib.h"
#include "yaml-cpp/yaml.h"

const int MAX_PAYLOAD_SIZE = 4096;

std::pair<int, std::string> process_payload(const std::string& payload) {
    if (payload.empty()) {
        return {400, "Error: 'payload' parameter is missing or empty."};
    }

    if (payload.length() > MAX_PAYLOAD_SIZE) {
        return {400, "Error: Payload exceeds maximum size limit."};
    }

    try {
        YAML::Node root = YAML::Load(payload);

        if (root.IsMap() && root["type"]) {
            std::string type = root["type"].as<std::string>();
            if (type == "Create") {
                return {400, "Error: 'Create' type is not allowed."};
            }
        }
        return {200, "Success: Payload processed."};
    } catch (const YAML::Exception&) {
        // Avoid echoing exception details
        return {400, "Error: Invalid YAML format in payload."};
    } catch (const std::exception&) {
        // Generic error for other potential issues
        return {500, "Error: Could not process payload."};
    }
}

void setup_and_run_server(httplib::Server& svr, int port) {
    svr.Get("/", [](const httplib::Request& req, httplib::Response& res) {
        std::string payload_str;
        if (req.has_param("payload")) {
            payload_str = req.get_param_value("payload");
        }
        
        auto [status, body] = process_payload(payload_str);
        
        res.status = status;
        res.set_content(body, "text/plain; charset=utf-8");
    });
    
    std::cout << "Server starting on http://localhost:" << port << std::endl;
    if (!svr.listen("0.0.0.0", port)) {
         std::cerr << "Error: Could not start server on port " << port << std::endl;
         exit(1);
    }
}

int main() {
    const int port = 5000;

    std::cout << "--- Test Cases (run in another terminal) ---" << std::endl;
    std::cout << "1. No payload parameter:" << std::endl;
    std::cout << "   curl -v \"http://localhost:5000/\"" << std::endl;
    std::cout << "2. Payload with type 'Create' (should be rejected):" << std::endl;
    std::cout << "   curl -v \"http://localhost:5000/?payload=type%3A%20Create%0Aname%3A%20test\"" << std::endl;
    std::cout << "3. Payload with a different type (should be accepted):" << std::endl;
    std::cout << "   curl -v \"http://localhost:5000/?payload=type%3A%20Update%0Aname%3A%20test\"" << std::endl;
    std::cout << "4. Malformed YAML payload (should be rejected):" << std::endl;
    std::cout << "   curl -v \"http://localhost:5000/?payload=type%3A%20Create%3A%20Fail\"" << std::endl;
    std::cout << "5. Empty payload (should be rejected):" << std::endl;
    std::cout << "   curl -v \"http://localhost:5000/?payload=\"" << std::endl;
    std::cout << "\nStarting server... Press Ctrl+C to stop." << std::endl;

    httplib::Server svr;
    try {
       setup_and_run_server(svr, port);
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}