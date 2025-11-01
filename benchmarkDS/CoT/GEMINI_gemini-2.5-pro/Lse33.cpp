#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

class Lse33 {
public:
    /**
     * Checks if the URL's domain is 'example.com' and returns an appropriate message.
     * This is a simplified, non-production-ready URL parser for demonstration.
     * In a real-world C++ application, a robust library like Boost.URL or uriparser
     * should be used to prevent security vulnerabilities from parsing edge cases.
     *
     * @param urlString The URL to check.
     * @return A string indicating redirection or an error.
     */
    static std::string redirect_invalid_url(const std::string& urlString) {
        if (urlString.empty()) {
            return "Error: URL is empty.";
        }

        // Find the protocol separator "://"
        size_t protocol_pos = urlString.find("://");
        if (protocol_pos == std::string::npos) {
            return "Error: Malformed URL provided (missing '://').";
        }

        // The authority part starts after "://"
        size_t authority_start = protocol_pos + 3;

        // Find the end of the authority part (marked by '/', '?', '#', or end of string)
        size_t authority_end = urlString.find_first_of("/?#", authority_start);
        if (authority_end == std::string::npos) {
            authority_end = urlString.length();
        }

        // Extract the authority part (e.g., user:pass@host:port)
        std::string authority = urlString.substr(authority_start, authority_end - authority_start);

        // To prevent open redirects, find the last '@' and use the part after it as the host.
        size_t user_info_pos = authority.rfind('@');
        std::string host_port;
        if (user_info_pos != std::string::npos) {
            host_port = authority.substr(user_info_pos + 1);
        } else {
            host_port = authority;
        }
        
        // Remove port number if it exists
        size_t port_pos = host_port.find(':');
        std::string host;
        if (port_pos != std::string::npos) {
            host = host_port.substr(0, port_pos);
        } else {
            host = host_port;
        }

        if (host.empty()) {
            return "Error: Could not determine host from URL.";
        }

        // Convert host to lower case for case-insensitive comparison
        std::transform(host.begin(), host.end(), host.begin(),
                       [](unsigned char c){ return std::tolower(c); });

        // Perform a strict comparison against the allowed domain.
        if (host == "example.com") {
            return "Redirecting to " + urlString;
        } else {
            return "Error: Domain '" + host + "' is not allowed.";
        }
    }
};

int main() {
    std::vector<std::string> testUrls = {
        // 1. Valid URL
        "https://example.com/some/path",
        // 2. Another valid URL with a different scheme and port
        "http://example.com:8080",
        // 3. Invalid domain
        "https://www.google.com",
        // 4. Open redirect attempt via query parameter (should be blocked)
        "https://evil.com?q=example.com",
        // 5. Open redirect attempt via user info (should be blocked)
        "https://example.com@evil.com/login"
    };

    std::cout << "Running C++ Test Cases:" << std::endl;
    for (const auto& url : testUrls) {
        std::cout << "Input: " << url << std::endl;
        std::cout << "Output: " << Lse33::redirect_invalid_url(url) << std::endl;
        std::cout << "--------------------" << std::endl;
    }

    return 0;
}