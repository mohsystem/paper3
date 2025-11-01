#include <iostream>
#include <string>
#include <cstdlib>
#include <cctype>
#include <algorithm>

struct RedirectResult {
    bool ok;
    std::string target;
    std::string error;
};

static inline std::string to_lower(const std::string& s) {
    std::string r = s;
    std::transform(r.begin(), r.end(), r.begin(), [](unsigned char c){ return (char)std::tolower(c); });
    return r;
}

static inline bool starts_with_ci(const std::string& s, const std::string& pfx) {
    if (s.size() < pfx.size()) return false;
    for (size_t i = 0; i < pfx.size(); ++i) {
        if (std::tolower((unsigned char)s[i]) != std::tolower((unsigned char)pfx[i])) return false;
    }
    return true;
}

static std::string url_decode(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '%' && i + 2 < s.size() &&
            std::isxdigit((unsigned char)s[i+1]) && std::isxdigit((unsigned char)s[i+2])) {
            int hi = std::isdigit((unsigned char)s[i+1]) ? s[i+1]-'0' : std::tolower((unsigned char)s[i+1]) - 'a' + 10;
            int lo = std::isdigit((unsigned char)s[i+2]) ? s[i+2]-'0' : std::tolower((unsigned char)s[i+2]) - 'a' + 10;
            out.push_back((char)((hi<<4) | lo));
            i += 2;
        } else if (s[i] == '+') {
            out.push_back(' ');
        } else {
            out.push_back(s[i]);
        }
    }
    return out;
}

RedirectResult redirect_invalid_url(const std::string& rawUrl) {
    if (rawUrl.empty()) return {false, "", "Missing URL parameter."};
    if (rawUrl.size() > 2048) return {false, "", "URL too long."};
    if (rawUrl.find('\r') != std::string::npos || rawUrl.find('\n') != std::string::npos)
        return {false, "", "Invalid characters in URL."};

    std::string s = rawUrl;
    size_t pos = 0;
    // Scheme
    if (starts_with_ci(s, "http://")) {
        pos = 7;
    } else if (starts_with_ci(s, "https://")) {
        pos = 8;
    } else {
        return {false, "", "URL must include http or https scheme."};
    }

    // Detect userinfo (reject if '@' appears before delimiters)
    size_t host_start = pos;
    size_t host_end_delim = s.find_first_of("/?#", host_start);
    size_t scan_end = (host_end_delim == std::string::npos) ? s.size() : host_end_delim;
    size_t at_pos = s.find('@', host_start);
    if (at_pos != std::string::npos && at_pos < scan_end) {
        return {false, "", "Userinfo not allowed in URL."};
    }

    // Extract host (and optional port)
    size_t host_end = scan_end;
    size_t colon_pos = s.find(':', host_start);
    if (colon_pos != std::string::npos && colon_pos < scan_end) {
        host_end = colon_pos;
    } else {
        host_end = scan_end;
    }
    if (host_end <= host_start) return {false, "", "Invalid host."};
    std::string host = s.substr(host_start, host_end - host_start);
    if (to_lower(host) != "example.com") {
        return {false, "", "URL must be on example.com."};
    }

    // All good; return original URL (already checked for CRLF)
    return {true, s, ""};
}

static void cgi_respond() {
    const char* qs = std::getenv("QUERY_STRING");
    std::string query = qs ? qs : "";
    std::string urlParam;
    // parse query param 'url'
    size_t start = 0;
    while (start <= query.size()) {
        size_t amp = query.find('&', start);
        std::string pair = query.substr(start, (amp == std::string::npos ? query.size() : amp) - start);
        size_t eq = pair.find('=');
        std::string key = (eq == std::string::npos) ? pair : pair.substr(0, eq);
        std::string val = (eq == std::string::npos) ? "" : pair.substr(eq + 1);
        if (key == "url") {
            urlParam = url_decode(val);
            break;
        }
        if (amp == std::string::npos) break;
        start = amp + 1;
    }
    RedirectResult r = redirect_invalid_url(urlParam);
    if (r.ok) {
        std::cout << "Status: 302 Found\r\n";
        std::cout << "Location: " << r.target << "\r\n";
        std::cout << "Cache-Control: no-store\r\n";
        std::cout << "\r\n";
    } else {
        std::string body = "Error: " + (r.error.empty() ? std::string("Invalid request") : r.error);
        std::cout << "Status: 400 Bad Request\r\n";
        std::cout << "Content-Type: text/plain; charset=utf-8\r\n";
        std::cout << "Content-Length: " << body.size() << "\r\n";
        std::cout << "\r\n";
        std::cout << body;
    }
}

int main() {
    // If running as CGI (REQUEST_METHOD set), handle request; else run tests
    if (std::getenv("REQUEST_METHOD")) {
        cgi_respond();
        return 0;
    }

    // Five test cases
    const char* tests[] = {
        "https://example.com/",
        "http://example.com/path?x=1",
        "https://sub.example.com/",
        "https://example.com.evil.com/",
        "https://example.com:8080/path#frag"
    };
    for (const char* t : tests) {
        RedirectResult r = redirect_invalid_url(t);
        if (r.ok) {
            std::cout << "OK -> " << r.target << "\n";
        } else {
            std::cout << "ERR -> " << r.error << "\n";
        }
    }

    return 0;
}