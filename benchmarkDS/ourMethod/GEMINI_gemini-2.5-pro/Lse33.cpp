// To compile: g++ -std=c++17 -o redirect_server redirect_server.cpp -lpthread
// You need to have the httplib.h header file in the same directory or in your include path.
// Get it from: https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h

#include <iostream>
#include <string>
#include <algorithm>
#include "httplib.h"

const std::string ALLOWED_DOMAIN = "example.com";

// A simple, non-comprehensive URL parser to extract the hostname.
// It is not fully RFC 3986 compliant and has limitations (e.g., with IPv6 addresses).
// For production, a robust URL parsing library is recommended.
std::string get_hostname(const std::string& url) {
    // Scheme check
    if (url.rfind("http://", 0) != 0 && url.rfind("https://", 0) != 0) {
        return "";
    }

    size_t authority_start = url.find("://");
    if (authority_start == std::string::npos) {
        return ""; // Should not happen due to check above but good for safety
    }
    authority_start += 3;

    size_t path_start = url.find_first_of("/?", authority_start);
    
    std::string authority;
    if (path_start == std::string::npos) {
        authority = url.substr(authority_start);
    } else {
        authority = url.substr(authority_start, path_start - authority_start);
    }

    size_t userinfo_end = authority.find('@');
    if (userinfo_end != std::string::npos) {
        authority = authority.substr(userinfo_end + 1);
    }
    
    size_t port_start = authority.find(':');
    if (port_start != std::string::npos) {
        // This logic is too simple for IPv6 addresses like [::1]:8080
        // We ignore that complexity here for the sake of a simple example.
        authority = authority.substr(0, port_start);
    }

    // Convert to lowercase for case-insensitive comparison
    std::transform(authority.begin(), authority.end(), authority.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    return authority;
}

void redirect_invalid_url(const httplib::Request& req, httplib::Response& res) {
    if (!req.has_param("url")) {
        res.status = 400;
        res.set_content("URL parameter is missing.", "text/plain");
        return;
    }

    std::string url_to_check = req.get_param_value("url");
    std::string hostname = get_hostname(url_to_check);

    // Rule #1: Whitelist validation to prevent Open Redirect (CWE-601)
    if (!hostname.empty() && hostname == ALLOWED_DOMAIN) {
        res.set_redirect(url_to_check, 302);
    } else {
        res.status = 400;
        res.set_content("Invalid URL or domain not allowed.", "text/plain");
    }
}

int main() {
    httplib::Server svr;

    svr.Get("/redirect", redirect_invalid_url);

    int port = 8080;
    std::cout << "Server started on port " << port << std::endl;
    std::cout << "--- Test Cases ---" << std::endl;
    std::cout << "1. Valid Redirect: curl -v \"http://localhost:8080/redirect?url=http%3A%2F%2Fexample.com%2Fsome%2Fpath\"" << std::endl;
    std::cout << "2. Valid Redirect (HTTPS): curl -v \"http://localhost:8080/redirect?url=https%3A%2F%2Fexample.com\"" << std::endl;
    std::cout << "3. Invalid Domain: curl -v \"http://localhost:8080/redirect?url=http%3A%2F%2Fgoogle.com\"" << std::endl;
    std::cout << "4. Invalid Subdomain Attack: curl -v \"http://localhost:8080/redirect?url=http%3A%2F%2Fexample.com.evil.com\"" << std::endl;
    std::cout << "5. Malformed URL: curl -v \"http://localhost:8080/redirect?url=not_a_valid_url\"" << std::endl;

    if (!svr.listen("0.0.0.0", port)) {
        std::cerr << "Failed to start server" << std::endl;
        return 1;
    }

    return 0;
}