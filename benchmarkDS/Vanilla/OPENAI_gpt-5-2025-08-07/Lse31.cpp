#include <iostream>
#include <string>
#include <algorithm>

std::string redirect_url(const std::string& url) {
    // Find start after scheme
    size_t start = url.find("://");
    start = (start == std::string::npos) ? 0 : start + 3;

    // Adjust for credentials if present (user:pass@)
    size_t first_delim = url.find_first_of("/?#", start);
    size_t at_pos = url.find('@', start);
    if (at_pos != std::string::npos && (first_delim == std::string::npos || at_pos < first_delim)) {
        start = at_pos + 1;
    }

    // Find end of host (before :, /, ?, or #)
    size_t end = url.find_first_of(":/?#", start);
    std::string host = (end == std::string::npos) ? url.substr(start) : url.substr(start, end - start);

    // Strip port if present
    size_t colon = host.find(':');
    if (colon != std::string::npos) {
        host = host.substr(0, colon);
    }

    // Lowercase host
    std::string host_lower = host;
    std::transform(host_lower.begin(), host_lower.end(), host_lower.begin(), [](unsigned char c){ return std::tolower(c); });

    if (host_lower == "example.com") {
        return std::string("Redirecting to ") + url;
    }
    return "404 Not Found";
}

int main() {
    std::string tests[5] = {
        "http://example.com",
        "https://example.com/path",
        "http://example.com:8080/abc",
        "https://sub.example.com/home",
        "https://notexample.com"
    };
    for (const auto& t : tests) {
        std::cout << redirect_url(t) << std::endl;
    }
    return 0;
}