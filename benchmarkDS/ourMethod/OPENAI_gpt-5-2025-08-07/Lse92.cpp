#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

static const int ITERATIONS = 210000;
static const size_t SALT_LEN = 16;
static const size_t DK_LEN = 32;
static const size_t MIN_USERNAME_LEN = 3;
static const size_t MAX_USERNAME_LEN = 32;
static const size_t MIN_PASSWORD_LEN = 12;
static const size_t MAX_PASSWORD_LEN = 128;

static bool isValidUsername(const std::string& u) {
    if (u.size() < MIN_USERNAME_LEN || u.size() > MAX_USERNAME_LEN) return false;
    for (unsigned char c : u) {
        bool ok = (c >= 'A' && c <= 'Z') ||
                  (c >= 'a' && c <= 'z') ||
                  (c >= '0' && c <= '9') ||
                  c == '_' || c == '.' || c == '-';
        if (!ok) return false;
    }
    return true;
}

static bool isValidPassword(const std::string& p) {
    if (p.size() < MIN_PASSWORD_LEN || p.size() > MAX_PASSWORD_LEN) return false;
    bool hasU = false, hasL = false, hasD = false, hasS = false;
    const std::string specials = "!@#$%^&*()_+-=[]{};':\"\\|,.<>/?";
    for (unsigned char c : p) {
        if (c >= 'A' && c <= 'Z') hasU = true;
        else if (c >= 'a' && c <= 'z') hasL = true;
        else if (c >= '0' && c <= '9') hasD = true;
        else if (specials.find(static_cast<char>(c)) != std::string::npos) hasS = true;
    }
    return hasU && hasL && hasD && hasS;
}

static std::string b64encode(const std::vector<unsigned char>& data) {
    if (data.empty()) return std::string();
    size_t outLen = 4 * ((data.size() + 2) / 3);
    std::string out(outLen, '\0');
    int written = EVP_EncodeBlock(reinterpret_cast<unsigned char*>(&out[0]),
                                  data.data(),
                                  static_cast<int>(data.size()));
    if (written < 0) throw std::runtime_error("Base64 encode failed");
    out.resize(static_cast<size_t>(written));
    return out;
}

std::string register_user(const std::string& username, const std::string& password) {
    if (!isValidUsername(username)) throw std::invalid_argument("Invalid input.");
    if (!isValidPassword(password)) throw std::invalid_argument("Invalid input.");

    std::vector<unsigned char> salt(SALT_LEN);
    if (RAND_bytes(salt.data(), static_cast<int>(salt.size())) != 1) {
        throw std::runtime_error("Random generation failed.");
    }

    std::vector<unsigned char> dk(DK_LEN);
    if (PKCS5_PBKDF2_HMAC(password.data(),
                          static_cast<int>(password.size()),
                          salt.data(),
                          static_cast<int>(salt.size()),
                          ITERATIONS,
                          EVP_sha256(),
                          static_cast<int>(dk.size()),
                          dk.data()) != 1) {
        throw std::runtime_error("PBKDF2 failed.");
    }

    std::string saltB64 = b64encode(salt);
    std::string hashB64 = b64encode(dk);

    OPENSSL_cleanse(dk.data(), dk.size());

    std::string record = "pbkdf2_sha256$" + std::to_string(ITERATIONS) + "$" + saltB64 + "$" + hashB64;
    return record;
}

int main() {
    std::pair<std::string, std::string> tests[5] = {
        {"alice_01", "S3cure!Passw0rd"},
        {"bob-user", "An0ther$Strong1"},
        {"charlie.test", "Ultr@S3curePass!"},
        {"dora-2025", "G00d#Password_++"},
        {"eve.user", "Y3t@N0ther_Strong"},
    };

    for (const auto& t : tests) {
        try {
            std::string rec = register_user(t.first, t.second);
            std::cout << "Registered: " << t.first << " -> " << rec << "\n";
        } catch (const std::exception&) {
            std::cout << "Registration failed for " << t.first << "\n";
        }
    }
    return 0;
}