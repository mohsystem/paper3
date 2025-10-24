// Chain-of-Through process in code generation:
// 1) Problem understanding: validate inputs using regex (email, phone, username, password, URL).
// 2) Security requirements: anchor patterns, bound lengths, avoid complex lookarounds.
// 3) Secure coding generation: multiple simple regex checks + length limits.
// 4) Code review: anchored, limited, safe I/O.
// 5) Secure code output: final code with 5 test cases.

#include <regex>
#include <string>
#include <iostream>
#include <vector>

namespace Task109Cpp {

static const size_t MAX_EMAIL_LEN = 320;
static const size_t MAX_PHONE_LEN = 16;
static const size_t MIN_USERNAME_LEN = 2;
static const size_t MAX_USERNAME_LEN = 30;
static const size_t MIN_PASSWORD_LEN = 8;
static const size_t MAX_PASSWORD_LEN = 64;
static const size_t MAX_URL_LEN = 2083;

static const std::regex EMAIL_RX(
    R"(^[A-Za-z0-9](?:[A-Za-z0-9._%+-]{0,62}[A-Za-z0-9])?@(?:[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?\.)+[A-Za-z]{2,63}$)",
    std::regex::ECMAScript
);
static const std::regex PHONE_RX(
    R"(^\+?[1-9]\d{1,14}$)",
    std::regex::ECMAScript
);
static const std::regex USERNAME_RX(
    R"(^[A-Za-z0-9](?:[A-Za-z0-9._-]{0,28}[A-Za-z0-9])?$)",
    std::regex::ECMAScript
);
static const std::regex PWD_HAS_LOWER(R"([a-z])", std::regex::ECMAScript);
static const std::regex PWD_HAS_UPPER(R"([A-Z])", std::regex::ECMAScript);
static const std::regex PWD_HAS_DIGIT(R"(\d)", std::regex::ECMAScript);
static const std::regex PWD_HAS_SPECIAL(R"([^A-Za-z0-9\s])", std::regex::ECMAScript);
static const std::regex PWD_HAS_SPACE(R"(\s)", std::regex::ECMAScript);
static const std::regex URL_RX(
    R"(^https?://(?:[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?\.)+[A-Za-z]{2,63}(?::\d{2,5})?(?:/[A-Za-z0-9._~!$&'()*+,;=:@%/\-?]*)?$)",
    std::regex::ECMAScript
);

static bool within(const std::string& s, size_t minLen, size_t maxLen) {
    return s.size() >= minLen && s.size() <= maxLen;
}

bool validateEmail(const std::string& input) {
    if (!within(input, 3, MAX_EMAIL_LEN)) return false;
    if (!std::regex_match(input, EMAIL_RX)) return false;
    auto atPos = input.find('@');
    if (atPos == std::string::npos || atPos == 0 || atPos == input.size() - 1) return false;
    std::string local = input.substr(0, atPos);
    std::string domain = input.substr(atPos + 1);
    if (local.find("..") != std::string::npos) return false;
    if (domain.find("..") != std::string::npos) return false;
    return true;
}

bool validatePhoneE164(const std::string& input) {
    if (!within(input, 2, MAX_PHONE_LEN)) return false;
    return std::regex_match(input, PHONE_RX);
}

bool validateUsername(const std::string& input) {
    if (!within(input, MIN_USERNAME_LEN, MAX_USERNAME_LEN)) return false;
    if (!std::regex_match(input, USERNAME_RX)) return false;
    for (size_t i = 1; i < input.size(); ++i) {
        char a = input[i - 1], b = input[i];
        auto isSpec = [](char c){ return c == '.' || c == '_' || c == '-'; };
        if (isSpec(a) && isSpec(b)) return false;
    }
    return true;
}

bool validateStrongPassword(const std::string& input) {
    if (!within(input, MIN_PASSWORD_LEN, MAX_PASSWORD_LEN)) return false;
    if (std::regex_search(input, PWD_HAS_SPACE)) return false;
    if (!std::regex_search(input, PWD_HAS_LOWER)) return false;
    if (!std::regex_search(input, PWD_HAS_UPPER)) return false;
    if (!std::regex_search(input, PWD_HAS_DIGIT)) return false;
    if (!std::regex_search(input, PWD_HAS_SPECIAL)) return false;
    return true;
}

bool validateURL(const std::string& input) {
    if (!within(input, 8, MAX_URL_LEN)) return false;
    return std::regex_match(input, URL_RX);
}

} // namespace Task109Cpp

int main() {
    using namespace Task109Cpp;
    std::vector<std::string> emails = {
        "user@example.com",
        "first.last+alias@sub.domain.co",
        ".user@domain.com",
        "user..name@domain.com",
        "user@-domain.com"
    };
    std::vector<std::string> phones = {
        "+14155552671",
        "4155552671",
        "+001234",
        "++123",
        "+1234567890123456"
    };
    std::vector<std::string> usernames = {
        "alice_01",
        "a",
        "user--name",
        "_user",
        "valid.user-name"
    };
    std::vector<std::string> passwords = {
        "Abcdef1!",
        "short1!",
        "NoDigits!",
        "noupper1!",
        "With Space1!"
    };
    std::vector<std::string> urls = {
        "https://example.com",
        "http://sub.domain.co:8080/path/to/resource?x=1",
        "ftp://example.com",
        "http://-bad.com",
        "http://example"
    };

    for (auto& e : emails) std::cout << "Email: " << e << " -> " << validateEmail(e) << "\n";
    for (auto& p : phones) std::cout << "Phone: " << p << " -> " << validatePhoneE164(p) << "\n";
    for (auto& u : usernames) std::cout << "Username: " << u << " -> " << validateUsername(u) << "\n";
    for (auto& pw : passwords) std::cout << "Password: " << pw << " -> " << validateStrongPassword(pw) << "\n";
    for (auto& url : urls) std::cout << "URL: " << url << " -> " << validateURL(url) << "\n";

    return 0;
}