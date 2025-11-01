// This code requires the following single-header libraries:
// 1. httplib.h: from https://github.com/yhirose/cpp-httplib
// 2. nlohmann/json.hpp: from https://github.com/nlohmann/json
// Please download them and place them in the same directory or in your include path.

#define CPPHTTPLIB_OPENSSL_SUPPORT // Uncomment for HTTPS support if needed
#include "httplib.h"
#include "json.hpp"

#include <iostream>
#include <string>
#include <optional>

// for convenience
using json = nlohmann::json;

/**
 * @brief Processes a JSON string payload to extract the 'name' field.
 *
 * @param payload The JSON string payload.
 * @return An optional containing the name if found, otherwise an empty optional.
 */
std::optional<std::string> process_payload(const std::string& payload) {
    if (payload.empty()) {
        return std::nullopt;
    }
    try {
        json j = json::parse(payload);
        if (j.is_object() && j.contains("name") && j["name"].is_string()) {
            return j["name"].get<std::string>();
        }
    } catch (json::parse_error& e) {
        // Log error if needed, but don't return details to the client
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return std::nullopt;
    }
    return std::nullopt;
}

void run_tests() {
    std::cout << "--- Running Test Cases ---" << std::endl;
    
    std::pair<std::string, std::optional<std::string>> test_cases[] = {
        {"{\"name\":\"Alice\"}", "Alice"},
        {"{\"age\":30, \"name\":\"Bob\"}", "Bob"},
        {"   { \"name\" : \"Charlie\" }   ", "Charlie"},
        {"{\"city\":\"New York\"}", std::nullopt},
        {"invalid json", std::nullopt},
        {"{\"name\": 123}", std::nullopt}
    };

    int i = 1;
    for (const auto& tc : test_cases) {
        auto result = process_payload(tc.first);
        bool correct = (result == tc.second);
        std::cout << "Test Case " << i++ << ": Payload='" << tc.first << "'" << std::endl;
        std::cout << "  -> Expected: " << (tc.second.has_value() ? *tc.second : "null")
                  << ", Got: " << (result.has_value() ? *result : "null")
                  << ". Correct: " << (correct ? "true" : "false") << std::endl;
    }
    std::cout << "--- Test Cases Finished ---" << std::endl << std::endl;
}

int main() {
    run_tests();

    // HTTP server setup
    httplib::Server svr;

    // Set a payload size limit to prevent DoS attacks
    svr.set_payload_max_length(1024);

    svr.Post("/api/name", [](const httplib::Request& req, httplib::Response& res) {
        auto name_opt = process_payload(req.body);
        if (name_opt) {
            res.set_content(*name_opt, "text/plain; charset=utf-8");
            res.status = 200;
        } else {
            res.set_content("Field 'name' not found or payload is malformed.", "text/plain; charset=utf-8");
            res.status = 400;
        }
    });
    
    // Custom error handler to provide generic messages
    svr.set_error_handler([](const httplib::Request&, httplib::Response& res) {
        const char *fmt = "<p>Error Status: <span style='color:red;'>%d</span></p>";
        char buf[BUFSIZ];
        snprintf(buf, sizeof(buf), fmt, res.status);
        res.set_content(buf, "text/html");
    });

    std::cout << "Server started on http://localhost:8080" << std::endl;
    std::cout << "Send a POST request to /api/name" << std::endl;
    std::cout << "Example: curl -X POST -H \"Content-Type: application/json\" -d '{\"name\":\"YourName\"}' http://localhost:8080/api/name" << std::endl;
    
    if (!svr.listen("localhost", 8080)) {
        std::cerr << "Failed to start server." << std::endl;
        return 1;
    }

    return 0;
}