#include <iostream>
#include <regex>
#include <string>

// 1) Problem understanding:
// - Precompile regex at module load.
// - Use regex to validate target URL.
// - If matches, return target; else return default.
// - Regex is not used to construct the redirect URL.

// 2) Security requirements:
// - Fixed regex (no user input).
// - Length cap to prevent excessive processing.
// - Safe default fallback.

// 3) Secure coding generation:
static const std::size_t MAX_URL_LENGTH = 2048;
static const std::regex ALLOWED_URL_PATTERN(
    "^https://(www\\.)?example\\.com(/[A-Za-z0-9._~!$&'()*+,;=:@%/-]*)?(\\?[A-Za-z0-9._~!$&'()*+,;=:@%/?-]*)?(#[A-Za-z0-9._~!$&'()*+,;=:@%/?-]*)?$",
    std::regex::icase
);

std::string determine_redirect(const std::string& target_url, const std::string& default_url) {
    const std::string safe_default = (!default_url.empty()) ? default_url : "https://example.com/";
    if (target_url.empty()) {
        return safe_default;
    }
    if (target_url.size() > MAX_URL_LENGTH) {
        return safe_default;
    }
    if (std::regex_match(target_url, ALLOWED_URL_PATTERN)) {
        return target_url;
    }
    return safe_default;
}

// 4) Code review and 5) Secure code output: tests
int main() {
    const std::string default_url = "https://safe.example.org/home";
    const std::string tests[5] = {
        "https://example.com/welcome",
        "https://www.example.com/path?item=42#section",
        "https://evil.com/phish",
        "http://example.com/insecure",
        ""
    };

    for (int i = 0; i < 5; ++i) {
        std::cout << "Test " << (i + 1) << ": " << determine_redirect(tests[i], default_url) << std::endl;
    }
    return 0;
}