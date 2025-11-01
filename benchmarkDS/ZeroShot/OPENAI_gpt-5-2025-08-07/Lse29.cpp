#include <iostream>
#include <string>
#include <regex>
#include <algorithm>

static const size_t MAX_URL_LENGTH = 2048;

static std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && isspace(static_cast<unsigned char>(s[start]))) start++;
    size_t end = s.size();
    while (end > start && isspace(static_cast<unsigned char>(s[end - 1]))) end--;
    return s.substr(start, end - start);
}

static bool istarts_with(const std::string& s, const std::string& pfx) {
    if (s.size() < pfx.size()) return false;
    for (size_t i = 0; i < pfx.size(); ++i) {
        char a = s[i], b = pfx[i];
        if (a >= 'A' && a <= 'Z') a = static_cast<char>(a - 'A' + 'a');
        if (b >= 'A' && b <= 'Z') b = static_cast<char>(b - 'A' + 'a');
        if (a != b) return false;
    }
    return true;
}

static std::string tolower_str(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
    return s;
}

std::string safe_redirect(const std::string& target) {
    if (target.empty()) return "/";

    std::string t = trim(target);
    if (t.empty() || t.size() > MAX_URL_LENGTH) return "/";

    if (t.find('\r') != std::string::npos || t.find('\n') != std::string::npos) return "/";

    if (!(istarts_with(t, "http://") || istarts_with(t, "https://"))) return "/";

    size_t scheme_end = t.find("://");
    if (scheme_end == std::string::npos) return "/";

    size_t host_start = scheme_end + 3;
    if (host_start >= t.size()) return "/";

    size_t host_end = t.find_first_of("/?#:", host_start);
    if (host_end == std::string::npos) host_end = t.size();

    if (host_end <= host_start) return "/";

    std::string host = t.substr(host_start, host_end - host_start);
    if (host.empty()) return "/";

    // Basic ASCII host validation
    if (host.size() > 253) return "/";
    for (char c : host) {
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '-' || c == '.')) {
            return "/";
        }
    }

    std::string host_lc = tolower_str(host);
    static const std::regex re(R"(^([^.]+\.)*example\.com$)", std::regex::icase);
    if (!std::regex_match(host_lc, re)) return "/";

    return t;
}

int main() {
    const std::string tests[5] = {
        "https://example.com/abc",
        "http://sub.example.com:8080/path",
        "https://evil.com",
        "/relative/path",
        "https://notexample.com"
    };
    for (const auto& test : tests) {
        std::cout << "Input: " << test << " -> Redirect: " << safe_redirect(test) << "\n";
    }
    return 0;
}