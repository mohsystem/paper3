#include <iostream>
#include <string>
#include <regex>
#include <algorithm>
#include <cctype>

// Core logic function: accepts a URL and returns {allowed, message_or_target}
// If allowed == true, message_or_target is the redirect target; otherwise it's the error message.
std::pair<bool, std::string> redirect_invalid_url(const std::string& url) {
    if (url.empty()) {
        return {false, "Missing URL"};
    }
    try {
        std::regex re(R"(^([A-Za-z][A-Za-z0-9+\-.]*):\/\/([^\/:?#]+))");
        std::smatch m;
        if (!std::regex_search(url, m, re)) {
            return {false, "Invalid URL"};
        }
        std::string host = m[2].str();
        std::string lower_host = host;
        std::transform(lower_host.begin(), lower_host.end(), lower_host.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        if (lower_host == "example.com") {
            return {true, url};
        } else {
            return {false, "URL must be from example.com"};
        }
    } catch (...) {
        return {false, "Invalid URL"};
    }
}

int main() {
    // This C++ program demonstrates the core logic with 5 test cases.
    // It is not a full web server; integrate redirect_invalid_url into your server/router as needed.
    const std::string tests[5] = {
        "https://example.com/",
        "http://example.com/blog",
        "https://evil.com/hack",
        "notaurl",
        "https://sub.example.com/"
    };
    for (int i = 0; i < 5; ++i) {
        auto res = redirect_invalid_url(tests[i]);
        if (res.first) {
            std::cout << "Test " << (i+1) << ": 302 redirect to " << res.second << "\n";
        } else {
            std::cout << "Test " << (i+1) << ": 400 " << res.second << "\n";
        }
    }
    return 0;
}