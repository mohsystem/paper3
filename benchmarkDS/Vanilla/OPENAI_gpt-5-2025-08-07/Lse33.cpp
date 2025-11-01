#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <iostream>
#include <cctype>

static std::string to_lower(const std::string &s) {
    std::string r = s;
    for (char &c : r) c = (char)std::tolower((unsigned char)c);
    return r;
}

static std::string url_decode(const std::string &s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '%' && i + 2 < s.size() && std::isxdigit((unsigned char)s[i+1]) && std::isxdigit((unsigned char)s[i+2])) {
            char hex[3] = { s[i+1], s[i+2], 0 };
            char ch = (char) strtol(hex, nullptr, 16);
            out.push_back(ch);
            i += 2;
        } else if (s[i] == '+') {
            out.push_back(' ');
        } else {
            out.push_back(s[i]);
        }
    }
    return out;
}

static std::string get_query_param(const std::string &query, const std::string &key) {
    size_t start = 0;
    while (start <= query.size()) {
        size_t end = query.find('&', start);
        if (end == std::string::npos) end = query.size();
        size_t eq = query.find('=', start);
        if (eq != std::string::npos && eq < end) {
            std::string k = url_decode(query.substr(start, eq - start));
            if (k == key) {
                std::string v = url_decode(query.substr(eq + 1, end - (eq + 1)));
                return v;
            }
        } else {
            std::string k = url_decode(query.substr(start, end - start));
            if (k == key) return "";
        }
        start = end + 1;
    }
    return "";
}

static bool extract_host(const std::string &url, std::string &host_out) {
    // Expect scheme://host[:port]/...
    size_t pos = url.find("://");
    if (pos == std::string::npos) return false;
    std::string scheme = to_lower(url.substr(0, pos));
    if (scheme != "http" && scheme != "https") return false;
    size_t host_start = pos + 3;
    if (host_start >= url.size()) return false;
    size_t host_end = url.find_first_of(":/?#", host_start);
    if (host_end == std::string::npos) host_end = url.size();
    if (host_end <= host_start) return false;
    host_out = url.substr(host_start, host_end - host_start);
    return true;
}

static bool redirect_invalid_url(const std::string &url, std::string &error_or_location, int &status_out) {
    if (url.empty()) {
        error_or_location = "Missing url parameter";
        status_out = 400;
        return false;
    }
    std::string host;
    if (!extract_host(url, host)) {
        error_or_location = "Invalid URL";
        status_out = 400;
        return false;
    }
    std::string host_l = to_lower(host);
    if (host_l == "example.com") {
        error_or_location = url;
        status_out = 302;
        return true;
    } else {
        error_or_location = "Invalid domain";
        status_out = 400;
        return false;
    }
}

int main() {
    const char* req_method = std::getenv("REQUEST_METHOD");
    if (req_method) {
        // CGI mode
        const char* qs_c = std::getenv("QUERY_STRING");
        std::string qs = qs_c ? qs_c : "";
        std::string url = get_query_param(qs, "url");
        std::string result;
        int status = 200;
        bool ok = redirect_invalid_url(url, result, status);
        if (ok) {
            std::printf("Status: 302 Found\r\n");
            std::printf("Location: %s\r\n", result.c_str());
            std::printf("Content-Type: text/plain\r\n\r\n");
            std::printf("Redirecting to %s\n", result.c_str());
        } else {
            std::printf("Status: 400 Bad Request\r\n");
            std::printf("Content-Type: text/plain\r\n\r\n");
            std::printf("%s\n", result.c_str());
        }
        return 0;
    }

    // 5 test cases (non-CGI mode)
    const char* tests[] = {
        "https://example.com/path",
        "http://example.com",
        "https://sub.example.com",
        "https://example.org",
        "not a url"
    };
    for (const char* t : tests) {
        std::string res;
        int status = 0;
        bool ok = redirect_invalid_url(t, res, status);
        std::cout << "Test: " << t << " -> ok=" << (ok ? "true" : "false") << " status=" << status << " result=" << res << "\n";
    }
    return 0;
}