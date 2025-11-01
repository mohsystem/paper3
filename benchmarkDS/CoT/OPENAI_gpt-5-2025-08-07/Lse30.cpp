#include <iostream>
#include <string>
#include <regex>
#include <cctype>

static const std::regex SAFE_PREFIX("^[A-Za-z0-9-]+\\.");
static const size_t MAX_LEN = 2048;

static inline int hexVal(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    return -1;
}

std::string urlDecode(const std::string &s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (c == '%' && i + 2 < s.size()) {
            int h1 = hexVal(s[i + 1]);
            int h2 = hexVal(s[i + 2]);
            if (h1 >= 0 && h2 >= 0) {
                out.push_back(static_cast<char>((h1 << 4) | h2));
                i += 2;
            } else {
                out.push_back(c);
            }
        } else if (c == '+') {
            out.push_back(' ');
        } else {
            out.push_back(c);
        }
    }
    return out;
}

std::string getParam(const std::string &query, const std::string &key) {
    size_t start = 0;
    while (start <= query.size()) {
        size_t amp = query.find('&', start);
        size_t end = (amp == std::string::npos) ? query.size() : amp;
        std::string pair = query.substr(start, end - start);
        size_t eq = pair.find('=');
        std::string k = (eq == std::string::npos) ? pair : pair.substr(0, eq);
        std::string v = (eq == std::string::npos) ? "" : pair.substr(eq + 1);
        k = urlDecode(k);
        if (k == key) {
            v = urlDecode(v);
            return v;
        }
        if (amp == std::string::npos) break;
        start = amp + 1;
    }
    return std::string();
}

std::string decideRedirectFromQuery(const std::string &query) {
    std::string q = query;
    if (!q.empty() && q[0] == '?') q.erase(0, 1);
    std::string target = getParam(q, "target");
    if (target.empty()) return "/";

    // Trim
    while (!target.empty() && std::isspace(static_cast<unsigned char>(target.front()))) target.erase(target.begin());
    while (!target.empty() && std::isspace(static_cast<unsigned char>(target.back()))) target.pop_back();

    if (target.empty() || target.size() > MAX_LEN) return "/";
    if (target.find('\r') != std::string::npos || target.find('\n') != std::string::npos) return "/";

    if (std::regex_search(target, SAFE_PREFIX)) {
        return target;
    }
    return "/";
}

int main() {
    std::string tests[] = {
        "target=abc.",
        "target=abc.def",
        "foo=1&target=-hyphen.",
        "target=..evil",
        "foo=bar"
    };
    for (const auto &t : tests) {
        std::cout << decideRedirectFromQuery(t) << std::endl;
    }
    return 0;
}