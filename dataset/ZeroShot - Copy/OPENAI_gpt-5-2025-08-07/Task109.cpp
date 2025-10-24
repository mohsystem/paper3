#include <iostream>
#include <regex>
#include <string>

static const size_t MAX_USERNAME_LEN = 32;
static const size_t MIN_USERNAME_LEN = 3;
static const size_t MAX_EMAIL_LEN = 320;
static const size_t MAX_PHONE_LEN = 16;
static const size_t MIN_PASSWORD_LEN = 8;
static const size_t MAX_PASSWORD_LEN = 128;
static const size_t MAX_ZIP_LEN = 10;

static const std::regex RE_USERNAME(R"(^[A-Za-z0-9_]{3,16}$)", std::regex::ECMAScript);
static const std::regex RE_EMAIL(R"(^[A-Za-z0-9_%+.-]+(\.[A-Za-z0-9_%+.-]+)*@[A-Za-z0-9-]+(\.[A-Za-z0-9-]+)*\.[A-Za-z]{2,24}$)", std::regex::ECMAScript);
static const std::regex RE_PHONE(R"(^\+[1-9][0-9]{1,14}$)", std::regex::ECMAScript);
static const std::regex RE_ZIP(R"(^[0-9]{5}(-[0-9]{4})?$)", std::regex::ECMAScript);

static const std::regex RE_UPPER(R"([A-Z])", std::regex::ECMAScript);
static const std::regex RE_LOWER(R"([a-z])", std::regex::ECMAScript);
static const std::regex RE_DIGIT(R"([0-9])", std::regex::ECMAScript);
static const std::regex RE_SPECIAL(R"([^A-Za-z0-9])", std::regex::ECMAScript);

bool validate_username(const std::string& s) {
    if (s.size() < MIN_USERNAME_LEN || s.size() > MAX_USERNAME_LEN) return false;
    return std::regex_match(s, RE_USERNAME);
}

bool validate_email(const std::string& s) {
    if (s.size() > MAX_EMAIL_LEN) return false;
    return std::regex_match(s, RE_EMAIL);
}

bool validate_phone_e164(const std::string& s) {
    if (s.size() > MAX_PHONE_LEN) return false;
    return std::regex_match(s, RE_PHONE);
}

bool validate_password(const std::string& s) {
    if (s.size() < MIN_PASSWORD_LEN || s.size() > MAX_PASSWORD_LEN) return false;
    if (!std::regex_search(s, RE_UPPER)) return false;
    if (!std::regex_search(s, RE_LOWER)) return false;
    if (!std::regex_search(s, RE_DIGIT)) return false;
    if (!std::regex_search(s, RE_SPECIAL)) return false;
    return true;
}

bool validate_us_zip(const std::string& s) {
    if (s.size() > MAX_ZIP_LEN) return false;
    return std::regex_match(s, RE_ZIP);
}

int main() {
    // 5 test cases
    std::cout << std::boolalpha;
    std::cout << validate_username(std::string("user_name12")) << "\n";
    std::cout << validate_email(std::string("john.doe@example.co.uk")) << "\n";
    std::cout << validate_phone_e164(std::string("+14155552671")) << "\n";
    std::cout << validate_password(std::string("Str0ng!Pass")) << "\n";
    std::cout << validate_us_zip(std::string("12345-6789")) << "\n";
    return 0;
}