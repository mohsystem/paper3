#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <regex>
#include <cstring>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

static const size_t SALT_LEN = 16; // 128-bit
static const size_t DERIVED_KEY_LEN = 32; // 256-bit
static const int PBKDF2_ITERATIONS = 210000;

static std::unordered_map<std::string, std::string> DATABASE; // username -> base64(salt||hash)

bool is_valid_username(const std::string &u) {
    static const std::regex re("^[A-Za-z0-9_.-]{3,32}$");
    return std::regex_match(u, re);
}

bool is_strong_password(const std::string &p) {
    if (p.size() < 12) return false;
    for (char c : p) { if (std::isspace(static_cast<unsigned char>(c))) return false; }
    bool has_upper=false, has_lower=false, has_digit=false, has_special=false;
    for (char c : p) {
        if (std::isupper(static_cast<unsigned char>(c))) has_upper = true;
        else if (std::islower(static_cast<unsigned char>(c))) has_lower = true;
        else if (std::isdigit(static_cast<unsigned char>(c))) has_digit = true;
        else has_special = true;
    }
    return has_upper && has_lower && has_digit && has_special;
}

std::vector<unsigned char> generate_salt(size_t len) {
    if (len < 16 || len > 64) throw std::runtime_error("Invalid salt length");
    std::vector<unsigned char> salt(len);
    if (RAND_bytes(salt.data(), static_cast<int>(salt.size())) != 1) {
        throw std::runtime_error("RAND_bytes failed");
    }
    return salt;
}

bool hash_password(const std::string &password,
                   const std::vector<unsigned char> &salt,
                   std::vector<unsigned char> &out_dk) {
    out_dk.assign(DERIVED_KEY_LEN, 0);
    const EVP_MD *md = EVP_sha256();
    if (PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.size()),
                          salt.data(), static_cast<int>(salt.size()),
                          PBKDF2_ITERATIONS, md,
                          static_cast<int>(out_dk.size()), out_dk.data()) != 1) {
        return false;
    }
    return true;
}

std::string base64_encode(const std::vector<unsigned char> &data) {
    int out_len = 4 * ((static_cast<int>(data.size()) + 2) / 3);
    std::string out(static_cast<size_t>(out_len), '\0');
    int enc_len = EVP_EncodeBlock(reinterpret_cast<unsigned char*>(&out[0]),
                                  data.data(), static_cast<int>(data.size()));
    if (enc_len < 0) throw std::runtime_error("EVP_EncodeBlock failed");
    out.resize(static_cast<size_t>(enc_len));
    return out;
}

std::vector<unsigned char> base64_decode(const std::string &b64) {
    int in_len = static_cast<int>(b64.size());
    if (in_len % 4 != 0) throw std::runtime_error("Invalid base64 length");
    int out_max = 3 * (in_len / 4);
    std::vector<unsigned char> out(static_cast<size_t>(out_max));
    int dec_len = EVP_DecodeBlock(out.data(),
                                  reinterpret_cast<const unsigned char*>(b64.data()),
                                  in_len);
    if (dec_len < 0) throw std::runtime_error("EVP_DecodeBlock failed");
    // Adjust for padding
    int pad = 0;
    if (in_len >= 2) {
        if (b64[in_len - 1] == '=') pad++;
        if (b64[in_len - 2] == '=') pad++;
    }
    dec_len -= pad;
    if (dec_len < 0) dec_len = 0;
    out.resize(static_cast<size_t>(dec_len));
    return out;
}

std::string concat_salt_hash_b64(const std::vector<unsigned char> &salt,
                                 const std::vector<unsigned char> &hash) {
    std::vector<unsigned char> combined;
    combined.reserve(salt.size() + hash.size());
    combined.insert(combined.end(), salt.begin(), salt.end());
    combined.insert(combined.end(), hash.begin(), hash.end());
    std::string enc = base64_encode(combined);
    // zeroize combined
    std::fill(combined.begin(), combined.end(), 0);
    return enc;
}

bool register_user(const std::string &username, const std::string &password) {
    if (!is_valid_username(username) || !is_strong_password(password)) return false;
    if (DATABASE.find(username) != DATABASE.end()) return false;
    try {
        auto salt = generate_salt(SALT_LEN);
        std::vector<unsigned char> dk;
        if (!hash_password(password, salt, dk)) return false;
        std::string record = concat_salt_hash_b64(salt, dk);
        DATABASE.emplace(username, record);
        // zeroize
        std::fill(dk.begin(), dk.end(), 0);
        std::fill(salt.begin(), salt.end(), 0);
        return true;
    } catch (...) {
        return false;
    }
}

bool verify_user(const std::string &username, const std::string &password) {
    auto it = DATABASE.find(username);
    if (it == DATABASE.end()) return false;
    std::vector<unsigned char> combined;
    try {
        combined = base64_decode(it->second);
    } catch (...) {
        return false;
    }
    if (combined.size() != SALT_LEN + DERIVED_KEY_LEN) return false;
    std::vector<unsigned char> salt(combined.begin(), combined.begin() + SALT_LEN);
    std::vector<unsigned char> stored_hash(combined.begin() + SALT_LEN, combined.end());
    std::vector<unsigned char> candidate;
    bool ok = hash_password(password, salt, candidate);
    if (!ok) return false;
    bool equal = (CRYPTO_memcmp(stored_hash.data(), candidate.data(), stored_hash.size()) == 0);
    // zeroize
    std::fill(salt.begin(), salt.end(), 0);
    std::fill(stored_hash.begin(), stored_hash.end(), 0);
    std::fill(candidate.begin(), candidate.end(), 0);
    std::fill(combined.begin(), combined.end(), 0);
    return equal;
}

int main() {
    // Test 1: valid registration
    bool t1 = register_user("alice", "Str0ng!Passw0rd");
    std::cout << "Test1 register valid: " << (t1 ? "true" : "false") << "\n";

    // Test 2: invalid username
    bool t2 = register_user("a", "Another$trong123");
    std::cout << "Test2 invalid username rejected: " << ((!t2) ? "true" : "false") << "\n";

    // Test 3: weak password
    bool t3 = register_user("bob", "weakpass");
    std::cout << "Test3 weak password rejected: " << ((!t3) ? "true" : "false") << "\n";

    // Test 4: duplicate username
    bool t4a = register_user("carol", "V3ry$trongPass!");
    bool t4b = register_user("carol", "Diff3rent$trong!");
    std::cout << "Test4 duplicate username rejected: " << ((t4a && !t4b) ? "true" : "false") << "\n";

    // Test 5: same password yields different stored records due to unique salts
    bool r1 = register_user("dave", "SamePassw0rd!!");
    bool r2 = register_user("erin", "SamePassw0rd!!");
    bool same_stored = false;
    if (r1 && r2) {
        same_stored = (DATABASE["dave"] == DATABASE["erin"]);
    }
    bool v1 = verify_user("dave", "SamePassw0rd!!");
    bool v2 = verify_user("erin", "SamePassw0rd!!");
    std::cout << "Test5 unique salts and verify: " << ((r1 && r2 && !same_stored && v1 && v2) ? "true" : "false") << "\n";

    return 0;
}