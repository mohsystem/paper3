#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

static std::vector<unsigned char> generate_salt(size_t length) {
    if (length == 0 || length > 1024) {
        throw std::invalid_argument("Invalid salt length.");
    }
    std::vector<unsigned char> salt(length);
    if (RAND_bytes(salt.data(), static_cast<int>(salt.size())) != 1) {
        throw std::runtime_error("CSPRNG failure.");
    }
    return salt;
}

static bool is_password_strong(const std::string& pwd) {
    if (pwd.size() < 12) return false;
    bool hasUpper=false, hasLower=false, hasDigit=false, hasSpecial=false;
    for (unsigned char c : pwd) {
        if (std::isupper(c)) hasUpper = true;
        else if (std::islower(c)) hasLower = true;
        else if (std::isdigit(c)) hasDigit = true;
        else hasSpecial = true;
    }
    return hasUpper && hasLower && hasDigit && hasSpecial;
}

static std::string to_hex(const std::vector<unsigned char>& data) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (unsigned char b : data) {
        oss << std::setw(2) << static_cast<unsigned int>(b);
    }
    return oss.str();
}

static std::string hash_password(const std::string& password,
                                 const std::vector<unsigned char>& salt,
                                 int iterations = 210000,
                                 size_t dkLen = 32) {
    if (password.empty()) {
        throw std::invalid_argument("Password must not be empty.");
    }
    if (!is_password_strong(password)) {
        throw std::invalid_argument("Password does not meet strength policy.");
    }
    if (salt.size() < 8 || salt.size() > 1024) {
        throw std::invalid_argument("Invalid salt.");
    }
    if (iterations < 100000 || iterations > 5000000) {
        throw std::invalid_argument("Invalid iterations.");
    }
    if (dkLen < 16 || dkLen > 64) {
        throw std::invalid_argument("Invalid derived key length.");
    }

    std::vector<unsigned char> dk(dkLen, 0);
    int rc = PKCS5_PBKDF2_HMAC(password.c_str(),
                               static_cast<int>(password.size()),
                               salt.data(),
                               static_cast<int>(salt.size()),
                               iterations,
                               EVP_sha256(),
                               static_cast<int>(dkLen),
                               dk.data());
    if (rc != 1) {
        OPENSSL_cleanse(dk.data(), dk.size());
        throw std::runtime_error("Hashing failed.");
    }
    std::string hex = to_hex(dk);
    OPENSSL_cleanse(dk.data(), dk.size());
    return hex;
}

int main() {
    std::vector<std::string> passwords{
        "Str0ngPassw0rd!",
        "An0ther$ecurePwd",
        "C0mpl3x#Pass123",
        "S@feAndS0und2025",
        "R0bust_P@ssw0rd!!"
    };
    try {
        for (size_t i = 0; i < passwords.size(); ++i) {
            std::vector<unsigned char> salt = generate_salt(16);
            std::string h = hash_password(passwords[i], salt, 210000, 32);
            std::cout << "Test " << (i + 1) << " hash: " << h << "\n";
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error.\n";
        return 1;
    }
    return 0;
}