#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>

static const size_t MAX_URL_LEN = 2048;

static std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
    return s;
}

std::string redirect_url(const std::string& url) {
    if (url.empty() || url.size() > MAX_URL_LEN) {
        return "404 Not Found";
    }

    // Trim leading/trailing spaces safely
    size_t start = 0;
    while (start < url.size() && std::isspace(static_cast<unsigned char>(url[start]))) start++;
    size_t end = url.size();
    while (end > start && std::isspace(static_cast<unsigned char>(url[end - 1]))) end--;
    if (start >= end) return "404 Not Found";
    std::string u = url.substr(start, end - start);

    // Parse scheme
    size_t scheme_end = u.find("://");
    if (scheme_end == std::string::npos) {
        return "404 Not Found";
    }
    std::string scheme = to_lower(u.substr(0, scheme_end));
    if (!(scheme == "http" || scheme == "https")) {
        return "404 Not Found";
    }

    size_t auth_start = scheme_end + 3;
    if (auth_start >= u.size()) return "404 Not Found";

    // Authority ends at first of / ? #
    size_t auth_end = u.find_first_of("/?#", auth_start);
    if (auth_end == std::string::npos) auth_end = u.size();
    if (auth_end <= auth_start) return "404 Not Found";

    std::string authority = u.substr(auth_start, auth_end - auth_start);

    // Strip userinfo
    size_t at = authority.rfind('@');
    std::string hostport = (at != std::string::npos) ? authority.substr(at + 1) : authority;

    std::string host;
    if (!hostport.empty() && hostport[0] == '[') {
        size_t closing = hostport.find(']');
        if (closing == std::string::npos || closing == 1) {
            return "404 Not Found";
        }
        host = hostport.substr(1, closing - 1);
    } else {
        size_t colon = hostport.find(':');
        host = (colon != std::string::npos) ? hostport.substr(0, colon) : hostport;
    }

    if (to_lower(host) == "example.com") {
        return "REDIRECT " + u;
    }
    return "404 Not Found";
}

int main() {
    std::string tests[5] = {
        "https://example.com/path",
        "http://example.com:8080?q=1",
        "http://sub.example.com/index.html",
        "ftp://example.com/file",
        "https://evil.com/"
    };
    for (const auto& t : tests) {
        std::cout << redirect_url(t) << "\n";
    }
    return 0;
}