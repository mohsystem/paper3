#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <climits>

std::string hashPasswordWithSalt(const std::string& password, const std::string& salt) {
    if (salt.size() > static_cast<size_t>(INT_MAX)) {
        throw std::invalid_argument("salt too long");
    }
    unsigned char md[EVP_MAX_MD_SIZE];
    unsigned int md_len = 0;
    unsigned char* res = HMAC(EVP_sha256(),
                              salt.data(),
                              static_cast<int>(salt.size()),
                              reinterpret_cast<const unsigned char*>(password.data()),
                              password.size(),
                              md,
                              &md_len);
    if (res == nullptr || md_len == 0) {
        throw std::runtime_error("HMAC computation failed");
    }
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (unsigned int i = 0; i < md_len; ++i) {
        oss << std::setw(2) << static_cast<unsigned int>(md[i]);
    }
    return oss.str();
}

int main() {
    try {
        std::pair<std::string, std::string> tests[5] = {
            {"password123", "NaCl"},
            {"correcthorsebatterystaple", "pepper"},
            {"", "salt"},
            {"p@ssw0rd!#%", "S@1t"},
            {"unicode-パスワード", "ソルト"}
        };
        for (const auto& t : tests) {
            std::cout << hashPasswordWithSalt(t.first, t.second) << std::endl;
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}