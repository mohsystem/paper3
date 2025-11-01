#include <iostream>
#include <regex>
#include <string>

static const std::regex SAFE_TARGET_PATTERN(R"(^[A-Za-z0-9-]+\.)");
static const size_t MAX_TARGET_LENGTH = 2048;

std::string get_redirect_url(const std::string& target_param) {
    std::string target = target_param;
    // Trim whitespace
    auto l = target.find_first_not_of(" \t\r\n");
    auto r = target.find_last_not_of(" \t\r\n");
    if (l == std::string::npos) {
        target.clear();
    } else {
        target = target.substr(l, r - l + 1);
    }

    if (target.empty() || target.size() > MAX_TARGET_LENGTH) {
        return "/";
    }
    if (target.find('\r') != std::string::npos || target.find('\n') != std::string::npos) {
        return "/";
    }
    if (std::regex_search(target, SAFE_TARGET_PATTERN)) {
        return target;
    }
    return "/";
}

int main() {
    std::string tests[5] = {
        "example.com/home",
        "sub-domain.example",
        "/internal/path",
        "http://evil.com",
        "bad..com"
    };
    for (const auto& t : tests) {
        std::cout << "Input: " << t << " => Redirect: " << get_redirect_url(t) << std::endl;
    }
    return 0;
}