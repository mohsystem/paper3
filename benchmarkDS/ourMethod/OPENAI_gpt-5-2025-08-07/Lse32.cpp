#include <iostream>
#include <string>
#include <cctype>
#include <sstream>

struct RedirectResult {
    bool valid;
    std::string message; // error if invalid
    std::string target;  // redirect target if valid
};

static bool has_control(const std::string& s) {
    for (unsigned char c : s) {
        if (c <= 0x1F || c == 0x7F) return true;
    }
    return false;
}

static bool url_decode(const std::string& in, std::string& out) {
    out.clear();
    out.reserve(in.size());
    for (size_t i = 0; i < in.size(); ++i) {
        char c = in[i];
        if (c == '%') {
            if (i + 2 >= in.size()) return false;
            char h1 = in[i+1], h2 = in[i+2];
            auto hex = [](char ch)->int{
                if (ch >= '0' && ch <= '9') return ch - '0';
                if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
                if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
                return -1;
            };
            int hi = hex(h1), lo = hex(h2);
            if (hi < 0 || lo < 0) return false;
            out.push_back(static_cast<char>((hi << 4) | lo));
            i += 2;
        } else {
            out.push_back(c);
        }
    }
    return true;
}

// Validation: only http/https and host exactly example.com, no userinfo, length <= 2048
RedirectResult redirect_invalid_url(const std::string& rawUrl) {
    if (rawUrl.empty() || rawUrl.size() > 2048) {
        return {false, "Invalid URL", ""};
    }
    if (has_control(rawUrl)) {
        return {false, "Invalid URL", ""};
    }
    // scheme://...
    auto pos = rawUrl.find("://");
    if (pos == std::string::npos) {
        return {false, "Invalid URL", ""};
    }
    std::string scheme = rawUrl.substr(0, pos);
    for (char& ch : scheme) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    if (!(scheme == "http" || scheme == "https")) {
        return {false, "Invalid URL", ""};
    }
    size_t host_start = pos + 3;
    if (host_start >= rawUrl.size()) {
        return {false, "Invalid URL", ""};
    }
    // Find end of authority
    size_t auth_end = rawUrl.find_first_of("/?#", host_start);
    if (auth_end == std::string::npos) auth_end = rawUrl.size();
    std::string authority = rawUrl.substr(host_start, auth_end - host_start);
    if (authority.find('@') != std::string::npos) {
        return {false, "Invalid URL", ""};
    }
    // Split host:port
    std::string host = authority;
    auto colon = authority.rfind(':');
    if (colon != std::string::npos) {
        host = authority.substr(0, colon);
        // port optional; we don't validate port here
    }
    // Lowercase host for compare
    for (char& ch : host) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    if (host != "example.com") {
        return {false, "Invalid URL", ""};
    }
    return {true, "OK", rawUrl};
}

// Build a basic HTTP response simulating a web route handler
std::string handle_request_path(const std::string& path) {
    const std::string prefix = "/redirect/";
    if (path.rfind(prefix, 0) != 0) {
        return "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: 10\r\n\r\nNot Found\n";
    }
    std::string enc = path.substr(prefix.size());
    if (enc.empty()) {
        return "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: 12\r\n\r\nInvalid URL\n";
    }
    std::string decoded;
    if (!url_decode(enc, decoded)) {
        return "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: 12\r\n\r\nInvalid URL\n";
    }
    RedirectResult r = redirect_invalid_url(decoded);
    if (!r.valid) {
        return "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: 12\r\n\r\nInvalid URL\n";
    }
    // Prevent header injection by re-validating control chars in target
    if (has_control(r.target)) {
        return "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: 12\r\n\r\nInvalid URL\n";
    }
    std::string body = "Redirecting...\n";
    std::ostringstream oss;
    oss << "HTTP/1.1 302 Found\r\n"
        << "Location: " << r.target << "\r\n"
        << "Content-Type: text/plain; charset=utf-8\r\n"
        << "Content-Length: " << body.size() << "\r\n"
        << "\r\n"
        << body;
    return oss.str();
}

int main() {
    // Five test cases for the validator
    const std::string tests[] = {
        "https://example.com/path",
        "http://example.com",
        "https://evil.com/",
        "https://example.com:443/ok",
        "https://user@example.com/"
    };
    for (const auto& t : tests) {
        RedirectResult r = redirect_invalid_url(t);
        if (r.valid) {
            std::cout << "[VALID] " << t << " -> " << r.target << "\n";
        } else {
            std::cout << "[INVALID] " << t << " -> " << r.message << "\n";
        }
    }

    // Five test cases for the route handler (simulated)
    const std::string paths[] = {
        "/redirect/https%3A%2F%2Fexample.com%2Fok",
        "/redirect/http%3A%2F%2Fexample.com",
        "/redirect/https%3A%2F%2Fevil.com",
        "/redirect/https%3A%2F%2Fexample.com%3A443%2Fabc",
        "/notredirect/https%3A%2F%2Fexample.com"
    };
    for (const auto& p : paths) {
        std::string resp = handle_request_path(p);
        // Print first line of response for brevity
        auto eol = resp.find("\r\n");
        std::cout << "REQ " << p << " => " << (eol != std::string::npos ? resp.substr(0, eol) : resp) << "\n";
    }

    return 0;
}