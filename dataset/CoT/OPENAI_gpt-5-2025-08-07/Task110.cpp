// Steps 1-5: Problem understanding, security requirements, secure generation, review, secure output (summarized)
#include <iostream>
#include <string>
#include <cctype>
#include <vector>
#include <limits>

static bool hasNoWhitespaceOrCtl(const std::string& s) {
    for (unsigned char c : s) {
        if (std::isspace(c) || c < 32) return false;
    }
    return true;
}

static bool isDigits(const std::string& s) {
    if (s.empty()) return false;
    for (unsigned char c : s) if (!std::isdigit(c)) return false;
    return true;
}

static bool isValidIPv4(const std::string& h) {
    int parts = 0;
    size_t start = 0, pos = 0;
    while (true) {
        pos = h.find('.', start);
        std::string seg = h.substr(start, (pos == std::string::npos) ? std::string::npos : pos - start);
        if (seg.empty() || seg.size() > 3) return false;
        if (!isDigits(seg)) return false;
        if (seg.size() > 1 && seg[0] == '0') return false;
        int v = std::stoi(seg);
        if (v < 0 || v > 255) return false;
        parts++;
        if (pos == std::string::npos) break;
        start = pos + 1;
        if (start >= h.size()) return false;
        if (parts > 4) return false;
    }
    return parts == 4;
}

static bool isValidIPv6(const std::string& h) {
    // Basic, safe validation: allowed chars and at least one colon; max one '::'
    if (h.find(':') == std::string::npos) return false;
    int dbl = 0;
    for (size_t i = 0; i + 1 < h.size(); ++i) {
        if (h[i] == ':' && h[i+1] == ':') dbl++;
    }
    if (dbl > 1) return false;
    for (unsigned char c : h) {
        bool ok = std::isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') || c == ':' || c == '.';
        if (!ok) return false;
    }
    return true;
}

static bool isAlnumDash(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) || c == '-';
}

static bool isValidDomain(const std::string& h) {
    if (h.empty() || h.size() > 253) return false;
    size_t start = 0, pos = 0;
    while (true) {
        pos = h.find('.', start);
        std::string label = h.substr(start, (pos == std::string::npos) ? std::string::npos : pos - start);
        if (label.empty() || label.size() > 63) return false;
        if (label.front() == '-' || label.back() == '-') return false;
        for (char c : label) {
            if (!isAlnumDash(c)) return false;
        }
        if (pos == std::string::npos) break;
        start = pos + 1;
        if (start >= h.size()) return false;
    }
    return true;
}

static bool isValidHost(const std::string& host) {
    if (host == "localhost" || host == "LOCALHOST") return true;
    if (isValidIPv4(host)) return true;
    if (host.find(':') != std::string::npos && isValidIPv6(host)) return true;
    return isValidDomain(host);
}

bool isValidURL(const std::string& url) {
    if (url.empty() || url.size() > 2048) return false;
    if (!hasNoWhitespaceOrCtl(url)) return false;

    // Parse scheme
    size_t scheme_end = url.find("://");
    if (scheme_end == std::string::npos || scheme_end == 0) return false;
    std::string scheme = url.substr(0, scheme_end);
    for (char& c : scheme) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    if (!(scheme == "http" || scheme == "https")) return false;

    // Authority
    size_t auth_start = scheme_end + 3;
    size_t path_start = url.find_first_of("/?#", auth_start);
    size_t auth_end = (path_start == std::string::npos) ? url.size() : path_start;
    if (auth_end <= auth_start) return false;

    std::string authority = url.substr(auth_start, auth_end - auth_start);
    if (authority.find('@') != std::string::npos) return false; // disallow userinfo

    std::string host;
    int port = -1;

    if (!authority.empty() && authority[0] == '[') {
        size_t close = authority.find(']');
        if (close == std::string::npos) return false;
        host = authority.substr(1, close - 1);
        if (close + 1 < authority.size()) {
            if (authority[close + 1] != ':') return false;
            std::string portStr = authority.substr(close + 2);
            if (!isDigits(portStr)) return false;
            long p = std::stol(portStr);
            if (p < 1 || p > 65535) return false;
            port = static_cast<int>(p);
        }
        if (!isValidIPv6(host)) return false;
    } else {
        size_t colon = authority.rfind(':');
        if (colon != std::string::npos && authority.find(':') == colon) {
            host = authority.substr(0, colon);
            std::string portStr = authority.substr(colon + 1);
            if (portStr.empty() || !isDigits(portStr)) return false;
            long p = std::stol(portStr);
            if (p < 1 || p > 65535) return false;
            port = static_cast<int>(p);
        } else {
            host = authority;
        }
        if (host.empty()) return false;
        if (!isValidHost(host)) return false;
    }

    return true;
}

int main() {
    std::vector<std::string> tests = {
        "https://example.com",
        "http://sub.example.co.uk:8080/path?query=1#frag",
        "ftp://example.com",
        "https://user:pass@example.com",
        "http://-bad.com"
    };
    for (const auto& t : tests) {
        std::cout << t << " -> " << (isValidURL(t) ? "true" : "false") << "\n";
    }
    return 0;
}