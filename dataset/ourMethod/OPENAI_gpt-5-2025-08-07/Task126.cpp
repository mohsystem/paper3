#include <iostream>
#include <string>
#include <unordered_map>
#include <cctype>

static bool isSafeUsername(const std::string &u) {
    if (u.empty() || u.size() > 30) return false;
    for (char c : u) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_')) {
            return false;
        }
    }
    return true;
}

static std::string toLowerAscii(const std::string &s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    return out;
}

std::string resolve_redirect(const std::string &input) {
    static const std::unordered_map<std::string, std::string> routes = {
        {"home", "/home"},
        {"about", "/about"},
        {"contact", "/contact"},
        {"help", "/help"},
        {"dashboard", "/user/dashboard"}
    };

    // Trim
    size_t start = 0;
    size_t end = input.size();
    while (start < end && std::isspace(static_cast<unsigned char>(input[start]))) start++;
    while (end > start && std::isspace(static_cast<unsigned char>(input[end - 1]))) end--;
    if (start == end) return "/error?code=bad-request";
    std::string s = input.substr(start, end - start);
    if (s.size() > 100) return "/error?code=too-long";

    auto pos = s.find(':');
    if (pos != std::string::npos) {
        std::string head = s.substr(0, pos);
        std::string tail = s.substr(pos + 1);
        // Trim tail spaces
        size_t tstart = 0, tend = tail.size();
        while (tstart < tend && std::isspace(static_cast<unsigned char>(tail[tstart]))) tstart++;
        while (tend > tstart && std::isspace(static_cast<unsigned char>(tail[tend - 1]))) tend--;
        tail = tail.substr(tstart, tend - tstart);

        std::string headLower = toLowerAscii(head);
        if (headLower == "profile") {
            if (!isSafeUsername(tail)) {
                return "/error?code=invalid-username";
            }
            return std::string("/user/profile/") + tail;
        }
        return "/error?code=unknown-page";
    } else {
        std::string key = toLowerAscii(s);
        auto it = routes.find(key);
        if (it != routes.end()) return it->second;
        return "/error?code=unknown-page";
    }
}

int main() {
    const std::string tests[5] = {
        "home",
        "Profile:Alice_123",
        "profile:bad/../hacker",
        "unknown",
        "dashboard"
    };
    for (const auto &t : tests) {
        std::cout << "Input: " << t << " -> Redirect: " << resolve_redirect(t) << "\n";
    }
    return 0;
}