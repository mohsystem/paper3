#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>

static std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) start++;
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) end--;
    return s.substr(start, end - start);
}

static std::string extract_host(const std::string& url) {
    std::string u = trim(url);
    if (u.empty()) return "";
    size_t start = std::string::npos;
    size_t schemePos = u.find("://");
    if (schemePos != std::string::npos) {
        start = schemePos + 3;
    } else if (u.rfind("//", 0) == 0) {
        start = 2;
    } else {
        return "";
    }
    size_t end = u.find_first_of("/?#", start);
    std::string authority = (end == std::string::npos) ? u.substr(start) : u.substr(start, end - start);
    if (authority.empty()) return "";
    size_t atPos = authority.rfind('@');
    std::string hostport = (atPos == std::string::npos) ? authority : authority.substr(atPos + 1);
    if (hostport.empty()) return "";
    // Handle IPv6 literals [::1]
    std::string host;
    if (!hostport.empty() && hostport[0] == '[') {
        size_t rb = hostport.find(']');
        if (rb == std::string::npos) return "";
        host = hostport.substr(1, rb - 1);
    } else {
        size_t colon = hostport.find(':');
        host = (colon == std::string::npos) ? hostport : hostport.substr(0, colon);
    }
    // lowercase
    std::string lowerHost = host;
    std::transform(lowerHost.begin(), lowerHost.end(), lowerHost.begin(),
                   [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
    return lowerHost;
}

int redirect_url(const std::string& url) {
    std::string host = extract_host(url);
    if (host == "example.com") {
        return 302;
    }
    return 404;
}

int main() {
    std::string tests[5] = {
        "https://example.com/page",
        "http://Example.com",
        "https://sub.example.com/home",
        "//example.com/resource",
        "mailto:user@example.com"
    };
    for (const auto& t : tests) {
        int code = redirect_url(t);
        std::cout << t << " -> " << code << "\n";
    }
    return 0;
}