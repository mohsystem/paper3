/*
Chain-of-Through in-code notes:
1) Problem: Provide validation function redirect_invalid_url() and a simulated route handler.
2) Security: Validate scheme (http/https), exact host example.com, and disallow CRLF.
3) Secure coding: Manual safe parsing without external libs, careful bounds.
4) Review: Avoid reflecting untrusted input without validation.
5) Output: Provide tests invoking the function.
*/

#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>

static bool contains_crlf(const std::string& s) {
    return s.find('\r') != std::string::npos || s.find('\n') != std::string::npos;
}

static std::string tolower_str(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return (char)std::tolower(c); });
    return s;
}

// Extract scheme and host from a URL: returns true on success.
static bool extract_scheme_host(const std::string& url, std::string& scheme, std::string& host) {
    // Find scheme
    auto pos = url.find("://");
    if (pos == std::string::npos) return false;
    scheme = url.substr(0, pos);
    std::string rest = url.substr(pos + 3);
    if (rest.empty()) return false;
    // Host is up to '/', ':', '?', '#'
    size_t end = rest.find_first_of("/:#?");
    if (end == std::string::npos) {
        host = rest;
    } else {
        host = rest.substr(0, end);
    }
    if (host.empty()) return false;
    return true;
}

// Returns pair<isRedirect, messageOrUrl>
std::pair<bool, std::string> redirect_invalid_url(const std::string& inputUrl) {
    if (inputUrl.empty()) return {false, "Missing URL"};
    if (contains_crlf(inputUrl)) return {false, "Invalid characters in URL"};
    std::string scheme, host;
    if (!extract_scheme_host(inputUrl, scheme, host)) {
        return {false, "URL must include scheme and host"};
    }
    std::string lscheme = tolower_str(scheme);
    if (!(lscheme == "http" || lscheme == "https")) {
        return {false, "Only http and https schemes are allowed"};
    }
    std::string lhost = tolower_str(host);
    if (lhost != "example.com") {
        return {false, "URL must have domain example.com"};
    }
    // Basic sanity check again
    if (contains_crlf(inputUrl)) return {false, "Invalid URL"};
    return {true, inputUrl};
}

// Simulated route handler (not a real socket server)
static std::string handle_request(const std::string& urlParam) {
    auto d = redirect_invalid_url(urlParam);
    if (d.first) {
        return "HTTP/1.1 302 Found\r\nLocation: " + d.second + "\r\n\r\n";
    } else {
        return "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\n" + d.second;
    }
}

int main() {
    std::string tests[5] = {
        "https://example.com/path",
        "http://example.com",
        "https://sub.example.com/path",  // invalid: subdomain
        "ftp://example.com/resource",    // invalid: scheme
        "https://evil.com/path"          // invalid: host
    };
    std::cout << "C++ tests for redirect_invalid_url:\n";
    for (auto& t : tests) {
        auto d = redirect_invalid_url(t);
        if (d.first) {
            std::cout << "OK redirect -> " << d.second << "\n";
        } else {
            std::cout << "ERROR -> " << d.second << "\n";
        }
    }
    // Show simulated route output for a valid and invalid case
    std::cout << "\nSimulated route responses:\n";
    std::cout << handle_request("https://example.com/ok") << "\n";
    std::cout << handle_request("https://evil.com/bad") << "\n";
    return 0;
}