#include <iostream>
#include <regex>
#include <string>

static const std::regex ALLOWED(
    R"(^https://(www\.)?example\.com(/[A-Za-z0-9._~\-/%?#&=]*)?$)",
    std::regex::ECMAScript | std::regex::optimize
);
static const size_t MAX_LEN = 2048;
static const std::string SAFE_FALLBACK = "https://www.example.com/";

static bool hasCtl(const std::string& s) {
    for (unsigned char ch : s) {
        if (ch < 0x20 || ch == 0x7F) return true;
    }
    return false;
}

static std::string sanitizeDefault(const std::string& def) {
    if (def.empty()) return SAFE_FALLBACK;
    if (def.size() > MAX_LEN) return SAFE_FALLBACK;
    if (hasCtl(def)) return SAFE_FALLBACK;
    if (def.rfind("https://", 0) != 0) return SAFE_FALLBACK;
    return def;
}

std::string resolveRedirect(const std::string& targetUrl, const std::string& defaultUrl) {
    std::string safeDefault = sanitizeDefault(defaultUrl);
    if (targetUrl.empty()) return safeDefault;
    if (targetUrl.size() > MAX_LEN) return safeDefault;
    if (hasCtl(targetUrl)) return safeDefault;
    if (std::regex_match(targetUrl, ALLOWED)) {
        return targetUrl;
    }
    return safeDefault;
}

int main() {
    std::string defaultUrl = "https://www.example.com/safe";
    std::string tests[5] = {
        "https://www.example.com/path?x=1",   // allowed
        "https://evil.com/",                  // wrong domain
        "http://www.example.com/",            // wrong scheme
        "",                                   // empty (simulating missing)
        "https://www.example.com/\r\nattack"  // CRLF injection attempt
    };
    for (int i = 0; i < 5; ++i) {
        std::cout << "Test " << (i+1) << ": " << resolveRedirect(tests[i], defaultUrl) << "\n";
    }
    return 0;
}