#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

/**
 * Checks if a URL's domain is 'example.com' and returns a corresponding status message.
 * @param url The URL to check.
 * @return A string indicating redirection or a 404 error.
 */
std::string redirect_url(const std::string& url) {
    std::string protocol_delimiter = "://";
    size_t start_pos = url.find(protocol_delimiter);

    if (start_pos == std::string::npos) {
        return "404 Not Found";
    }

    // Move pointer past the delimiter
    start_pos += protocol_delimiter.length();

    // Find the end of the domain (either a '/' or the end of the string)
    size_t end_pos = url.find('/', start_pos);
    
    std::string domain;
    if (end_pos == std::string::npos) {
        domain = url.substr(start_pos);
    } else {
        domain = url.substr(start_pos, end_pos - start_pos);
    }

    // Case-insensitive comparison
    std::transform(domain.begin(), domain.end(), domain.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    if (domain == "example.com") {
        return "Redirecting to " + url;
    } else {
        return "404 Not Found";
    }
}

int main() {
    std::vector<std::string> test_urls = {
        "http://example.com/home",
        "https://EXAMPLE.com/login?user=1",
        "http://www.google.com",
        "https://sub.example.com",
        "not-a-valid-url"
    };

    std::cout << "--- Running 5 Test Cases ---" << std::endl;
    for (const auto& url : test_urls) {
        std::cout << "Input: " << url << std::endl;
        std::cout << "Output: " << redirect_url(url) << std::endl;
        std::cout << "--------------------" << std::endl;
    }

    return 0;
}