/*
 * Note: This code requires the 'cpp-httplib' and 'yaml-cpp' libraries.
 * 'cpp-httplib' is a header-only library. You can get it from: https://github.com/yhirose/cpp-httplib
 * 'yaml-cpp' needs to be built and linked. You can get it from: https://github.com/jbeder/yaml-cpp
 *
 * Compilation command (assuming libraries are in include/lib paths):
 * g++ -std=c++17 -o server Lse84.cpp -I/path/to/httplib -I/path/to/yaml-cpp/include -L/path/to/yaml-cpp/build -lyaml-cpp -lpthread
 */
#include <iostream>
#include "httplib.h"
#include "yaml-cpp/yaml.h"

class Lse84 {
public:
    static std::string processPayload(const std::string& payload) {
        try {
            YAML::Node data = YAML::Load(payload);
            if (data["type"] && data["type"].IsScalar() && data["type"].as<std::string>() == "Create") {
                return "Error: Payload with type 'Create' is not allowed.";
            } else {
                return "Payload processed successfully.";
            }
        } catch (const YAML::ParserException& e) {
            return "Error: Invalid YAML format in payload.";
        }
    }

    static void handleRequest(const httplib::Request& req, httplib::Response& res) {
        if (req.has_param("payload")) {
            auto payload = req.get_param_value("payload");
            std::string result = processPayload(payload);
            if (result.rfind("Error:", 0) == 0) { // Check if string starts with "Error:"
                res.status = 400;
            } else {
                res.status = 200;
            }
            res.set_content(result, "text/plain");
        } else {
            res.status = 400;
            res.set_content("Error: 'payload' parameter is missing.", "text/plain");
        }
    }

    static void runServer() {
        httplib::Server svr;
        svr.Get("/", handleRequest);
        
        int port = 5000;
        std::cout << "Server started on port " << port << std::endl;
        svr.listen("0.0.0.0", port);
    }
};

int main() {
    /*
     * Main function starts the server. To test, run the server and use a tool like curl:
     * 
     * Test Case 1: No payload parameter
     * curl "http://localhost:5000/"
     * Expected Output: Error: 'payload' parameter is missing.
     * 
     * Test Case 2: Payload with type: Create
     * curl "http://localhost:5000/?payload=type%3A%20Create%0Aname%3A%20my-resource"
     * Expected Output: Error: Payload with type 'Create' is not allowed.
     *
     * Test Case 3: Payload with a different type
     * curl "http://localhost:5000/?payload=type%3A%20Update%0Aname%3A%20my-resource"
     * Expected Output: Payload processed successfully.
     * 
     * Test Case 4: Malformed YAML payload
     * curl "http://localhost:5000/?payload=type%3A%20[invalid"
     * Expected Output: Error: Invalid YAML format in payload.
     *
     * Test Case 5: Valid YAML but no 'type' key
     * curl "http://localhost:5000/?payload=name%3A%20my-resource"
     * Expected Output: Payload processed successfully.
     */
    Lse84::runServer();
    return 0;
}