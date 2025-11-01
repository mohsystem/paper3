/*
 * IMPORTANT: This code requires the 'cpp-httplib' library.
 * It is a header-only library. You can get it from: https://github.com/yhirose/cpp-httplib
 * Just download the 'httplib.h' file and place it in the same directory or in your include path.
 *
 * Compilation command:
 * g++ -o server Lse32.cpp -I. -pthread -std=c++11
 *
 * NOTE: Manually parsing URLs is complex and error-prone. For a production system,
 * using a dedicated and well-tested URL parsing library is strongly recommended
 * to prevent security vulnerabilities.
 */
#include <iostream>
#include <string>
#include "httplib.h"

// A helper function to extract the hostname from a URL.
// This is a simplified parser and may not cover all edge cases.
std::string get_hostname(const std::string& url) {
    size_t scheme_end = url.find("://");
    if (scheme_end == std::string::npos) {
        return ""; // Not a full URL with a scheme
    }

    size_t host_start = scheme_end + 3;
    size_t path_start = url.find('/', host_start);
    
    std::string host_port_part;
    if (path_start == std::string::npos) {
        host_port_part = url.substr(host_start);
    } else {
        host_port_part = url.substr(host_start, path_start - host_start);
    }
    
    // Handle user:pass@host
    size_t at_pos = host_port_part.find('@');
    if (at_pos != std::string::npos) {
        host_port_part = host_port_part.substr(at_pos + 1);
    }

    // Handle host:port
    size_t port_pos = host_port_part.find(':');
    if (port_pos != std::string::npos) {
        return host_port_part.substr(0, port_pos);
    }

    return host_port_part;
}

void redirect_invalid_url(const httplib::Request& req, httplib::Response& res) {
    // The part of the URL after /redirect/ is captured
    std::string url_to_redirect = req.matches[1];
    std::string hostname = get_hostname(url_to_redirect);

    // Security Check: Only allow redirects to "example.com"
    if (hostname == "example.com") {
        res.set_redirect(url_to_redirect.c_str(), 302);
    } else {
        res.status = 400;
        res.set_content("Invalid URL. Only redirects to example.com are allowed.", "text/plain");
    }
}

int main(void) {
    httplib::Server svr;

    // Matches /redirect/ followed by any characters
    svr.Get(R"(/redirect/(.*))", redirect_invalid_url);

    int port = 8081;
    std::cout << "C++ server started on port " << port << std::endl;
    std::cout << "Test with the following URLs:" << std::endl;
    std::cout << "1. Valid: http://localhost:" << port << "/redirect/http://example.com" << std::endl;
    std::cout << "2. Valid with path: http://localhost:" << port << "/redirect/https://example.com/some/path" << std::endl;
    std::cout << "3. Invalid Host: http://localhost:" << port << "/redirect/http://google.com" << std::endl;
    std::cout << "4. Tricky URL (invalid): http://localhost:" << port << "/redirect/http://example.com@evil.com" << std::endl;
    std::cout << "5. Malformed URL: http://localhost:" << port << "/redirect/not-a-valid-url" << std::endl;
    
    svr.listen("0.0.0.0", port);
    
    return 0;
}