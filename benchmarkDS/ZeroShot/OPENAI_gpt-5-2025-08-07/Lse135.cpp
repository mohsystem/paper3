#include <iostream>
#include <string>
#include <algorithm>

static bool constant_time_equals(const std::string& a, const std::string& b) {
    const size_t maxlen = std::max(a.size(), b.size());
    unsigned int diff = static_cast<unsigned int>(a.size() ^ b.size());
    for (size_t i = 0; i < maxlen; ++i) {
        unsigned char x = i < a.size() ? static_cast<unsigned char>(a[i]) : 0;
        unsigned char y = i < b.size() ? static_cast<unsigned char>(b[i]) : 0;
        diff |= static_cast<unsigned int>(x ^ y);
    }
    return diff == 0;
}

std::string check_mod(const std::string& username, const std::string& password) {
    static const std::string STORED_USER = "admin";
    static const std::string STORED_PASS = "S3cureP@ss!";
    bool userOk = constant_time_equals(username, STORED_USER);
    bool passOk = constant_time_equals(password, STORED_PASS);
    return (userOk && passOk) ? "true" : "false";
}

int main() {
    // 5 test cases
    std::cout << check_mod("admin", "S3cureP@ss!") << "\n";  // expected true
    std::cout << check_mod("admin", "wrong") << "\n";        // expected false
    std::cout << check_mod("user", "S3cureP@ss!") << "\n";   // expected false
    std::cout << check_mod("", "") << "\n";                  // expected false
    std::cout << check_mod("admin", "S3cureP@ss!!") << "\n"; // expected false
    return 0;
}