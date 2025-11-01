// Chain-of-Through step 1: Problem understanding
// Implement a function to register a user by hashing the password with PBKDF2, returning salt and hash.
// Chain-of-Through step 2: Security requirements
// - Validate input; use OpenSSL PBKDF2 with SHA-256, strong iterations, random salt.
// Chain-of-Through step 3: Secure coding generation (below)
// Chain-of-Through step 4: Code review
// Chain-of-Through step 5: Secure code output

#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <sstream>
#include <iomanip>
#include <stdexcept>

#include <openssl/evp.h>
#include <openssl/rand.h>

struct RegistrationResult {
    std::string username;
    std::string algorithm;
    int iterations;
    std::string salt_hex;
    std::string hash_hex;
    std::string toString() const {
        std::ostringstream oss;
        oss << "{"
            << "\"username\":\"" << username << "\","
            << "\"algorithm\":\"" << algorithm << "\","
            << "\"iterations\":" << iterations << ","
            << "\"salt_hex\":\"" << salt_hex << "\","
            << "\"hash_hex\":\"" << hash_hex << "\""
            << "}";
        return oss.str();
    }
};

static bool validate_username(const std::string& u) {
    static const std::regex re("^[A-Za-z0-9_.-]{3,32}$");
    return std::regex_match(u, re);
}

static bool validate_password(const std::string& p) {
    if (p.size() < 12) return false;
    bool up=false, lo=false, di=false, sp=false;
    for (unsigned char c : p) {
        if (std::isupper(c)) up = true;
        else if (std::islower(c)) lo = true;
        else if (std::isdigit(c)) di = true;
        else sp = true;
    }
    return up && lo && di && sp;
}

static std::string to_hex(const unsigned char* data, size_t len) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (size_t i = 0; i < len; ++i) {
        oss << std::setw(2) << static_cast<unsigned int>(data[i]);
    }
    return oss.str();
}

RegistrationResult register_user(const std::string& username, const std::string& password) {
    if (!validate_username(username)) {
        throw std::invalid_argument("Invalid username. Use 3-32 chars: letters, digits, _ . -");
    }
    if (!validate_password(password)) {
        throw std::invalid_argument("Weak password. Min 12 chars with upper, lower, digit, special.");
    }

    const int salt_len = 16;
    const int iterations = 200000;
    const int dk_len = 32;

    unsigned char salt[salt_len];
    if (RAND_bytes(salt, salt_len) != 1) {
        throw std::runtime_error("Failed to generate salt");
    }

    std::vector<unsigned char> pwbuf(password.begin(), password.end());
    std::vector<unsigned char> dk(dk_len);

    if (PKCS5_PBKDF2_HMAC(reinterpret_cast<const char*>(pwbuf.data()),
                          static_cast<int>(pwbuf.size()),
                          salt, salt_len,
                          iterations,
                          EVP_sha256(),
                          dk_len, dk.data()) != 1) {
        std::fill(pwbuf.begin(), pwbuf.end(), 0);
        throw std::runtime_error("PBKDF2 failed");
    }

    std::string salt_hex = to_hex(salt, salt_len);
    std::string hash_hex = to_hex(dk.data(), dk.size());

    std::fill(pwbuf.begin(), pwbuf.end(), 0);
    std::fill(dk.begin(), dk.end(), 0);

    RegistrationResult res;
    res.username = username;
    res.algorithm = "PBKDF2-HMAC-SHA256";
    res.iterations = iterations;
    res.salt_hex = salt_hex;
    res.hash_hex = hash_hex;
    return res;
}

int main() {
    std::pair<std::string, std::string> tests[5] = {
        {"alice_1", "StrongPassw0rd!"},
        {"Bob-User", "Another$tr0ngPass!"},
        {"charlie.user", "Y3tAn0ther#Pass!"},
        {"dave_2025", "S0methingComp!ex"},
        {"eve.test", "Unbr3akable@Key!"}
    };
    for (auto& t : tests) {
        try {
            RegistrationResult r = register_user(t.first, t.second);
            std::cout << r.toString() << "\n";
        } catch (const std::exception& ex) {
            std::cout << "{\"error\":\"" << ex.what() << "\"}\n";
        }
    }
    return 0;
}