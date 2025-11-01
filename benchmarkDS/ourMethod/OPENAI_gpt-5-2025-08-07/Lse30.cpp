#include <iostream>
#include <string>
#include <regex>
#include <cctype>
#include <limits>

static const size_t MAX_QUERY_LEN = 4096;
static const size_t MAX_VALUE_LEN = 2048;

static bool is_hex(char c) {
    return std::isdigit(static_cast<unsigned char>(c)) ||
           (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F');
}

static int hex_val(char c) {
    if (std::isdigit(static_cast<unsigned char>(c))) return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    return 10 + (c - 'A');
}

static bool url_decode_limited(const std::string &in, std::string &out, size_t max_out) {
    out.clear();
    out.reserve(in.size());
    for (size_t i = 0; i < in.size(); ++i) {
        if (out.size() >= max_out) return false;
        char c = in[i];
        if (c == '%') {
            if (i + 2 >= in.size() || !is_hex(in[i + 1]) || !is_hex(in[i + 2])) {
                return false;
            }
            char d = static_cast<char>((hex_val(in[i + 1]) << 4) | hex_val(in[i + 2]));
            out.push_back(d);
            i += 2;
        } else if (c == '+') {
            out.push_back(' ');
        } else {
            out.push_back(c);
        }
    }
    return out.size() <= max_out;
}

static bool is_safe_ascii(const std::string &s) {
    for (unsigned char c : s) {
        if (c < 0x20 || c == 0x7F || c > 0x7E) return false;
    }
    return true;
}

static bool extract_param_first(const std::string &query, const std::string &name, std::string &value) {
    // Split on '&' up to a reasonable limit
    size_t start = 0;
    int parts = 0;
    while (start <= query.size() && parts < 200) {
        size_t end = query.find('&', start);
        std::string pair = (end == std::string::npos) ? query.substr(start) : query.substr(start, end - start);
        if (!pair.empty() && pair[0] == '?') pair.erase(0, 1);
        size_t eq = pair.find('=');
        if (eq != std::string::npos) {
            std::string k = pair.substr(0, eq);
            std::string v = pair.substr(eq + 1);
            if (k == name) {
                return url_decode_limited(v, value, MAX_VALUE_LEN);
            }
        }
        if (end == std::string::npos) break;
        start = end + 1;
        parts++;
    }
    return false;
}

std::string determine_redirect(const std::string &query_string) {
    if (query_string.size() > MAX_QUERY_LEN) return "/";
    std::string target;
    if (!extract_param_first(query_string, "target", target)) return "/";
    if (target.empty() || target.size() > MAX_VALUE_LEN) return "/";
    if (!is_safe_ascii(target)) return "/";
    try {
        static const std::regex safe_prefix("^[A-Za-z0-9-]+\\..*", std::regex::ECMAScript);
        if (std::regex_match(target, safe_prefix)) {
            return target;
        }
    } catch (...) {
        return "/";
    }
    return "/";
}

int main() {
    const std::string tests[5] = {
        "target=abc.example.com",           // valid
        "target=abc-",                      // invalid
        "other=1&target=-sub.example.org",  // valid
        "x=1&target=foo.&y=2",              // valid
        "target=%0d%0ahttps://evil.com"     // invalid (control chars)
    };
    for (const auto &q : tests) {
        std::cout << "Query: " << q << " -> Redirect: " << determine_redirect(q) << "\n";
    }
    return 0;
}