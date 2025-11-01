#include <iostream>
#include <string>
#include <regex>

static std::string url_decode(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '%' && i + 2 < s.size()) {
            auto hex = s.substr(i + 1, 2);
            char ch = static_cast<char>(std::stoi(hex, nullptr, 16));
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

static std::string get_query_param(const std::string& query, const std::string& key) {
    size_t start = 0;
    while (start <= query.size()) {
        size_t end = query.find('&', start);
        if (end == std::string::npos) end = query.size();
        std::string part = query.substr(start, end - start);
        size_t eq = part.find('=');
        std::string k = (eq == std::string::npos) ? part : part.substr(0, eq);
        std::string v = (eq == std::string::npos) ? std::string() : part.substr(eq + 1);
        if (k == key) return v;
        start = end + 1;
    }
    return "";
}

std::string get_redirect(const std::string& query) {
    std::string raw = get_query_param(query, "target");
    if (raw.empty() && query.find("target=") == std::string::npos) return "/";
    std::string decoded = url_decode(raw);
    static const std::regex pattern("^[A-Za-z0-9-]+\\..*");
    if (std::regex_match(decoded, pattern)) {
        return decoded;
    }
    return std::string("/");
}

int main() {
    std::string tests[] = {
        "target=sub.example.com",
        "target=http%3A%2F%2Fevil.com",
        "target=foo.bar/baz",
        "page=home",
        "target=foo."
    };
    for (const auto& q : tests) {
        std::cout << get_redirect(q) << std::endl;
    }
    return 0;
}