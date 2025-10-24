#include <iostream>
#include <string>
#include <cctype>
#include <vector>
#include <limits>
#include <cstring>
#if defined(__unix__) || defined(__APPLE__) || defined(__MACH__)
  #include <arpa/inet.h>
  #include <netinet/in.h>
  #define HAVE_INET_PTON 1
#else
  // inet_pton may not be available on non-POSIX; IPv6 validation will be limited.
  #define HAVE_INET_PTON 0
#endif

static bool hasControlOrSpace(const std::string& s) {
    for (unsigned char ch : s) {
        if (std::isspace(ch) || ch < 0x20 || ch == 0x7F) return true;
    }
    return false;
}

static bool isDigits(const std::string& s) {
    if (s.empty()) return false;
    for (unsigned char ch : s) if (!std::isdigit(ch)) return false;
    return true;
}

static bool isValidPort(const std::string& s) {
    if (!isDigits(s)) return false;
    if (s.size() > 1 && s[0] == '0') return false;
    try {
        unsigned long v = std::stoul(s);
        return v >= 1 && v <= 65535;
    } catch (...) {
        return false;
    }
}

static bool isValidIPv4(const std::string& host) {
    int dots = 0;
    for (char c : host) if (c == '.') dots++;
    if (dots != 3) return false;
    size_t start = 0, end = 0;
    int parts = 0;
    while (end <= host.size()) {
        if (end == host.size() || host[end] == '.') {
            std::string part = host.substr(start, end - start);
            if (part.empty() || part.size() > 3) return false;
            for (char c : part) if (!std::isdigit((unsigned char)c)) return false;
            if (part.size() > 1 && part[0] == '0') return false;
            int val = std::stoi(part);
            if (val < 0 || val > 255) return false;
            parts++;
            start = end + 1;
        }
        end++;
    }
    return parts == 4;
}

static bool isAlphaNumDash(char c) {
    return std::isalnum((unsigned char)c) || c == '-';
}

static bool isValidDomain(const std::string& host) {
    if (host.empty() || host.size() > 253) return false;
    std::string h = host;
    if (!h.empty() && h.back() == '.') h.pop_back();
    if (h.empty()) return false;
    size_t start = 0, end = 0;
    while (end <= h.size()) {
        if (end == h.size() || h[end] == '.') {
            std::string label = h.substr(start, end - start);
            if (label.size() < 1 || label.size() > 63) return false;
            for (char c : label) if (!isAlphaNumDash(c)) return false;
            if (label.front() == '-' || label.back() == '-') return false;
            start = end + 1;
        }
        end++;
    }
    return true;
}

static bool isValidIPv6Literal(const std::string& host) {
#if HAVE_INET_PTON
    unsigned char buf[16];
    return inet_pton(AF_INET6, host.c_str(), buf) == 1;
#else
    // Fallback: very basic check - must contain ':' and no spaces/brackets inside
    if (host.find(':') == std::string::npos) return false;
    for (char c : host) {
        if (!(std::isxdigit((unsigned char)c) || c == ':' || c == '.')) return false;
    }
    // This is a minimal sanity check; rigorous validation requires inet_pton.
    return true;
#endif
}

bool isValidURL(const std::string& url) {
    if (url.empty() || url.size() > 2048) return false;
    if (hasControlOrSpace(url)) return false;

    auto pos = url.find("://");
    if (pos == std::string::npos || pos == 0) return false;
    std::string scheme = url.substr(0, pos);
    for (char& c : scheme) c = (char)std::tolower((unsigned char)c);
    if (!(scheme == "http" || scheme == "https")) return false;

    size_t authStart = pos + 3;
    size_t authEnd = url.find_first_of("/?#", authStart);
    if (authEnd == std::string::npos) authEnd = url.size();
    if (authEnd <= authStart) return false;
    std::string netloc = url.substr(authStart, authEnd - authStart);
    if (netloc.find('@') != std::string::npos) return false; // disallow credentials

    std::string host;
    std::string portStr;
    if (!netloc.empty() && netloc.front() == '[') {
        // IPv6 bracketed
        size_t rb = netloc.find(']');
        if (rb == std::string::npos) return false;
        host = netloc.substr(1, rb - 1);
        if (rb + 1 < netloc.size()) {
            if (netloc[rb + 1] != ':') return false;
            portStr = netloc.substr(rb + 2);
            if (!isValidPort(portStr)) return false;
        }
        if (!isValidIPv6Literal(host)) return false;
    } else {
        // split host:port, but ensure only one colon (IPv6 not allowed here)
        size_t colon = netloc.rfind(':');
        if (colon != std::string::npos) {
            // ensure only one colon
            if (netloc.find(':') != colon) return false;
            host = netloc.substr(0, colon);
            portStr = netloc.substr(colon + 1);
            if (!isValidPort(portStr)) return false;
        } else {
            host = netloc;
        }
        if (host.empty()) return false;
        bool ipv4Candidate = true;
        for (char c : host) {
            if (!(std::isdigit((unsigned char)c) || c == '.')) { ipv4Candidate = false; break; }
        }
        if (ipv4Candidate) {
            if (!isValidIPv4(host)) return false;
        } else {
            if (!isValidDomain(host)) return false;
        }
    }
    return true;
}

int main() {
    std::vector<std::string> tests = {
        "https://example.com",
        "http://sub.example.co.uk/path?query=1#frag",
        "https://user:pass@example.com",
        "ftp://example.com",
        "http://256.256.256.256"
    };
    for (const auto& t : tests) {
        std::cout << t << " -> " << (isValidURL(t) ? "true" : "false") << "\n";
    }
    return 0;
}