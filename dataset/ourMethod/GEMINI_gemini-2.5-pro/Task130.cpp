#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <map>

// Represents a simplified HTTP Response
struct Response {
    int status_code;
    std::map<std::string, std::string> headers;
    std::string body;
};

// Global constants for CORS configuration
// Whitelist of allowed origins.
const std::unordered_set<std::string> ALLOWED_ORIGINS = {
    "https://example.com",
    "https://trusted.site.org"
};
const std::string ALLOWED_METHODS = "GET, POST, OPTIONS";
const std::string ALLOWED_HEADERS = "Content-Type, Authorization";
const std::string MAX_AGE = "86400"; // 24 hours

/**
 * Handles a request by applying Cross-Origin Resource Sharing (CORS) rules.
 * This function simulates a server-side API endpoint's CORS logic.
 * 
 * @param method The HTTP method (e.g., "GET", "OPTIONS").
 * @param origin_header The value of the "Origin" header from the request.
 * @return A Response struct containing status, headers, and body.
 */
Response handle_cors_request(const std::string& method, const std::string& origin_header) {
    Response res;
    
    // Rule #3: Validate input. The origin must be from the predefined whitelist.
    bool is_origin_allowed = !origin_header.empty() && (ALLOWED_ORIGINS.count(origin_header) > 0);

    if (!is_origin_allowed) {
        // If the origin is not allowed, or no origin header was provided,
        // do not add any CORS headers and return an error.
        res.status_code = 403;
        res.body = "{\"error\": \"CORS policy does not allow this origin.\"}";
        return res;
    }
    
    // The origin is allowed, so add the primary CORS response header.
    res.headers["Access-Control-Allow-Origin"] = origin_header;

    // Handle preflight (OPTIONS) request
    if (method == "OPTIONS") {
        res.status_code = 204; // No Content
        res.headers["Access-Control-Allow-Methods"] = ALLOWED_METHODS;
        res.headers["Access-Control-Allow-Headers"] = ALLOWED_HEADERS;
        res.headers["Access-Control-Max-Age"] = MAX_AGE;
        return res;
    }

    // Handle actual API requests (e.g., GET, POST)
    if (method == "GET" || method == "POST") {
        res.status_code = 200;
        res.headers["Content-Type"] = "application/json";
        res.body = "{\"data\": \"some protected data\"}";
        return res;
    }

    // If the method is not OPTIONS, GET, or POST, it's not allowed.
    // The Access-Control-Allow-Origin header is still sent because the origin was valid.
    res.status_code = 405;
    res.body = "{\"error\": \"Method not allowed.\"}";
    return res;
}

void print_response(const Response& res) {
    std::cout << "Status: " << res.status_code << std::endl;
    std::cout << "Headers:" << std::endl;
    if (res.headers.empty()) {
        std::cout << "  (none)" << std::endl;
    } else {
        for (const auto& pair : res.headers) {
            std::cout << "  " << pair.first << ": " << pair.second << std::endl;
        }
    }
    std::cout << "Body:" << std::endl;
    std::cout << res.body << std::endl;
}

int main() {
    std::cout << "--- Test Case 1: Valid GET request from an allowed origin ---" << std::endl;
    Response res1 = handle_cors_request("GET", "https://example.com");
    print_response(res1);
    std::cout << "\n----------------------------------------------------------\n" << std::endl;

    std::cout << "--- Test Case 2: Valid Preflight (OPTIONS) request from an allowed origin ---" << std::endl;
    Response res2 = handle_cors_request("OPTIONS", "https://trusted.site.org");
    print_response(res2);
    std::cout << "\n----------------------------------------------------------\n" << std::endl;
    
    std::cout << "--- Test Case 3: Request from a disallowed origin ---" << std::endl;
    Response res3 = handle_cors_request("GET", "https://malicious.com");
    print_response(res3);
    std::cout << "\n----------------------------------------------------------\n" << std::endl;

    std::cout << "--- Test Case 4: Request with a missing Origin header ---" << std::endl;
    Response res4 = handle_cors_request("GET", ""); // Simulating missing header with an empty string
    print_response(res4);
    std::cout << "\n----------------------------------------------------------\n" << std::endl;
    
    std::cout << "--- Test Case 5: Request with a disallowed method (PUT) from an allowed origin ---" << std::endl;
    Response res5 = handle_cors_request("PUT", "https://example.com");
    print_response(res5);
    std::cout << "\n----------------------------------------------------------\n" << std::endl;

    return 0;
}