/*
NOTE: This code requires two third-party libraries:
1. cpp-httplib: A header-only, cross-platform HTTP/HTTPS library.
   Download 'httplib.h' from https://github.com/yhirose/cpp-httplib and place it in your include path.
2. yaml-cpp: A YAML parser and emitter in C++.
   Install it via your system's package manager (e.g., `sudo apt-get install libyaml-cpp-dev` on Debian/Ubuntu)
   or build from source: https://github.com/jbeder/yaml-cpp

Compilation command:
g++ -o server Lse84.cpp -I/path/to/httplib/ -lyaml-cpp -pthread -std=c++17
*/

#define CPPHTTPLIB_OPENSSL_SUPPORT // Comment out if you don't need SSL support
#include <iostream>
#include "httplib.h"
#include "yaml-cpp/yaml.h"
#include <string>

void handle_request(const httplib::Request& req, httplib::Response& res) {
    if (!req.has_param("payload")) {
        res.status = 400;
        res.set_content("Error: 'payload' parameter is missing.", "text/plain");
        return;
    }

    std::string payload = req.get_param_value("payload");
    if (payload.empty()) {
        res.status = 400;
        res.set_content("Error: 'payload' parameter is empty.", "text/plain");
        return;
    }

    try {
        YAML::Node data = YAML::Load(payload);
        // yaml-cpp is generally safe as it parses into its own node types,
        // not arbitrary C++ objects, thus avoiding classic deserialization vulnerabilities.
        if (!data.IsMap()) {
             res.status = 400;
             res.set_content("Error: Payload must be a YAML mapping (key-value pairs).", "text/plain");
             return;
        }

        if (data["type"]) {
            std::string type = data["type"].as<std::string>();
            if (type == "Create") {
                res.status = 400;
                res.set_content("Error: 'Create' type is not allowed.", "text/plain");
            } else {
                res.status = 200;
                res.set_content("Payload processed successfully.", "text/plain");
            }
        } else {
             res.status = 400;
             res.set_content("Error: 'type' key not found in payload.", "text/plain");
        }

    } catch (const YAML::ParserException& e) {
        res.status = 400;
        std::string error_msg = "Error: Could not parse YAML payload. ";
        error_msg += e.what();
        res.set_content(error_msg, "text/plain");
    }
}

class Lse84 {
public:
    static void run_server(int port) {
        httplib::Server svr;

        svr.Get("/", handle_request);
        svr.Get("/test", handle_request);

        std::cout << "Server started on port " << port << std::endl;
        
        std::cout << "Running 5 test cases using curl:" << std::endl;
        std::cout << "1. No 'payload' parameter: curl -i \"http://localhost:5000/test\"" << std::endl;
        std::cout << "2. 'payload' with 'type: Create': curl -i \"http://localhost:5000/?payload=type%3A%20Create\"" << std::endl;
        std::cout << "3. Valid 'payload': curl -i \"http://localhost:5000/?payload=type%3A%20Update%0Aname%3A%20test\"" << std::endl;
        std::cout << "4. Malformed YAML in 'payload': curl -i \"http://localhost:5000/?payload=type%3A%20%5Binvalid\"" << std::endl;
        std::cout << "5. Empty 'payload': curl -i \"http://localhost:5000/?payload=\"" << std::endl;
        
        svr.listen("0.0.0.0", port);
    }
};

int main() {
    Lse84::run_server(5000);
    return 0;
}