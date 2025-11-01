#include <iostream>
#include <string>
#include <vector>

/**
 * Checks if the URL's domain is "example.com".
 *
 * @param url The URL to check.
 * @return A redirection message if the domain is valid, otherwise an error message.
 */
std::string redirectInvalidUrl(const std::string& url) {
    const std::string protocol_delimiter = "://";
    size_t protocol_pos = url.find(protocol_delimiter);

    if (protocol_pos == std::string::npos) {
        return "Error: Malformed URL provided (missing '://').";
    }

    size_t host_start = protocol_pos + protocol_delimiter.length();
    if (host_start >= url.length()) {
        return "Error: Malformed URL provided (empty host).";
    }

    size_t host_end = url.find('/', host_start);
    
    std::string host;
    if (host_end == std::string::npos) {
        // No path, host is the rest of the string
        host = url.substr(host_start);
    } else {
        host = url.substr(host_start, host_end - host_start);
    }

    if (host.empty()) {
        return "Error: Malformed URL provided (empty host).";
    }

    if (host == "example.com") {
        return "Redirecting to " + url;
    } else {
        return "Error: Invalid domain. Only example.com is allowed. Domain was: " + host;
    }
}

int main() {
    std::vector<std::string> testUrls = {
        "http://example.com/path/to/page",
        "https://www.google.com",
        "http://test.example.com",
        "ftp://example.com/resource",
        "htp:/invalid-url" // Malformed URL
    };

    std::cout << "--- Running C++ Test Cases ---" << std::endl;
    for (const auto& url : testUrls) {
        std::cout << "Input : " << url << std::endl;
        std::cout << "Output: " << redirectInvalidUrl(url) << std::endl;
        std::cout << "---------------------------------" << std::endl;
    }

    return 0;
}