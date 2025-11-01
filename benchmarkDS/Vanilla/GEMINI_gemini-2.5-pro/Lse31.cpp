#include <iostream>
#include <string>
#include <vector>

/**
 * Parses a URL to check if its domain is "example.com".
 *
 * @param url The URL to check.
 * @return "redirected" if the domain is "example.com", otherwise "404 error".
 */
std::string redirect_url(const std::string& url) {
    // Find the protocol separator "://"
    std::string::size_type protocol_end_pos = url.find("://");
    if (protocol_end_pos == std::string::npos) {
        return "404 error"; // No protocol, treat as invalid URL
    }

    // The domain starts right after the protocol separator
    std::string::size_type domain_start_pos = protocol_end_pos + 3;

    // The domain ends at the next '/', ':', or the end of the string
    std::string::size_type domain_end_pos = url.find_first_of("/:", domain_start_pos);

    std::string domain;
    if (domain_end_pos == std::string::npos) {
        // The domain is the rest of the string
        domain = url.substr(domain_start_pos);
    } else {
        // Extract the domain substring
        domain = url.substr(domain_start_pos, domain_end_pos - domain_start_pos);
    }

    if (domain == "example.com") {
        return "redirected";
    } else {
        return "404 error";
    }
}

int main() {
    std::vector<std::string> testUrls = {
        "https://example.com/home",
        "http://www.example.com/test",
        "ftp://google.com/files",
        "https://example.com:8080/path?a=1",
        "not_a_valid_url"
    };

    std::cout << "CPP Test Cases:" << std::endl;
    for (const auto& url : testUrls) {
        std::cout << "URL: " << url << " -> " << redirect_url(url) << std::endl;
    }

    return 0;
}