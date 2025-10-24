#include <iostream>
#include <string>
#include <vector>

static std::string escapeHtml(const std::string& s) {
    std::string out;
    out.reserve(s.size() * 2);
    for (char c : s) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&#39;"; break;
            default: out += c;
        }
    }
    return out;
}

std::string displayOnWeb(const std::string& input) {
    std::string escaped = escapeHtml(input);
    std::string prefix = "<!doctype html><html><head><meta charset=\"utf-8\"><title>Echo</title></head><body><h1>Echo</h1><pre>";
    std::string suffix = "</pre></body></html>";
    return prefix + escaped + suffix;
}

int main() {
    std::vector<std::string> tests = {
        "Hello, world!",
        "<script>alert('xss')</script>",
        "Tom & Jerry",
        "Quotes: \"double\" and 'single'",
        "Line1\nLine2\tTabbed"
    };
    for (const auto& t : tests) {
        std::cout << displayOnWeb(t) << std::endl;
    }
    return 0;
}