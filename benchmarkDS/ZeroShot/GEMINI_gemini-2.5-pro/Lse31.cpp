#include <iostream>
#include <string>
#include <vector>
#include <cctype>

/**
 * Checks if a URL's host is 'example.com' and returns a redirect message.
 * Otherwise, returns a 404 error message. This implementation is secure
 * against open redirect vulnerabilities by strictly checking the hostname.
 *
 * Note: C++ standard library does not have a built-in URL parser. This is a
 * simplified manual implementation. For production code, a robust third-party
 * library is highly recommended.
 *
 * @param url The URL string to validate.
 * @return A string indicating redirection or a 404 error.
 */
std::string redirect_url(const std::string& url) {
    std::string protocol_separator = "://";
    size_t host_start_pos = url.find(protocol_separator);

    if (host_start_pos == std::string::npos) {
        // No protocol found, could be a relative URL or malformed.
        return "404 Not Found";
    }

    host_start_pos += protocol_separator.length();

    // Find the end of the host part (marked by ':', '/', '?', or end of string)
    size_t host_end_pos = url.find_first_of(":/?", host_start_pos);
    
    std::string hostname;
    if (host_end_pos == std::string::npos) {
        hostname = url.substr(host_start_pos);
    } else {
        hostname = url.substr(host_start_pos, host_end_pos - host_start_pos);
    }
    
    if (hostname.empty()) {
        return "404 Not Found";
    }

    // Perform a case-insensitive comparison
    std::string target_host = "example.com";
    if (hostname.length() != target_host.length()) {
        return "404 Not Found";
    }

    for (size_t i = 0; i < hostname.length(); ++i) {
        if (std::tolower(static_cast<unsigned char>(hostname[i])) != target_host[i]) {
            return "404 Not Found";
        }
    }

    return "Redirecting to: " + url;
}

int main() {
    std::vector<std::string> testUrls = {
        "https://example.com/path/to/resource",
        "http://www.google.com",
        "https://evil-example.com",
        "https://example.com.hacker.com/login",
        "ftp://example.com:21/files"
    };

    std::cout << "Running C++ Test Cases:" << std::endl;
    for (const auto& url : testUrls) {
        std::cout << "Input: " << url << std::endl;
        std::cout << "Output: " << redirect_url(url) << std::endl;
        std::cout << "--------------------" << std::endl;
    }

    return 0;
}