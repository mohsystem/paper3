#include <iostream>
#include <string>
#include <vector>

/**
 * A simple URL parser to extract the hostname.
 * This is a simplified version for demonstration.
 * In a real-world application, a robust library should be used.
 */
std::string get_hostname(const std::string& url) {
    // Find the protocol separator "://"
    size_t protocol_pos = url.find("://");
    if (protocol_pos == std::string::npos) {
        return ""; // No protocol found
    }

    // The hostname starts after the protocol separator
    size_t host_start_pos = protocol_pos + 3;

    // Find the end of the hostname, which is marked by '/', ':', '?', or '#'
    size_t host_end_pos = url.find_first_of("/:?#", host_start_pos);

    if (host_end_pos == std::string::npos) {
        // If no delimiter is found, the rest of the string is the host
        return url.substr(host_start_pos);
    } else {
        return url.substr(host_start_pos, host_end_pos - host_start_pos);
    }
}

/**
 * Checks if the URL has the domain "example.com".
 * This function simulates the logic of a web server route.
 * 
 * @param url The URL to check.
 * @return A string indicating the result (redirect or error).
 */
std::string redirect_invalid_url(const std::string& url) {
    if (url.empty()) {
        return "Error: URL is empty.";
    }
    
    std::string hostname = get_hostname(url);
    
    if (hostname.empty()) {
        return "Error: Could not parse hostname from URL.";
    }

    // Secure check: only allow redirection to the exact "example.com" domain.
    if (hostname == "example.com") {
        return "Redirecting to: " + url;
    } else {
        return "Error: Invalid domain. Provided: '" + hostname + "'. Redirection is only allowed to example.com.";
    }
}

int main() {
    // 5 Test Cases
    std::vector<std::string> test_cases = {
        // 1. Valid URL
        "https://example.com/path/to/page",
        // 2. Invalid domain
        "http://www.google.com",
        // 3. Subdomain exploit attempt
        "https://sub.example.com.malicious.net/phishing",
        // 4. Valid domain with port
        "http://example.com:8080/secure",
        // 5. URL with no path
        "http://another-site.org"
    };

    std::cout << "--- C++ Test Cases ---" << std::endl;
    for (size_t i = 0; i < test_cases.size(); ++i) {
        std::cout << "Test " << i + 1 << ": Checking URL '" << test_cases[i] << "'" << std::endl;
        std::string result = redirect_invalid_url(test_cases[i]);
        std::cout << "  Result: " << result << std::endl << std::endl;
    }

    return 0;
}