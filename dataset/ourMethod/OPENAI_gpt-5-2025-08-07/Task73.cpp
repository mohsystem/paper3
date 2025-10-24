#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <stdexcept>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

static bool meetsPasswordPolicy(const std::string &pwd) {
    const size_t minLen = 12;
    const size_t maxLen = 128;
    if (pwd.size() < minLen || pwd.size() > maxLen) return false;

    bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
    for (unsigned char uc : pwd) {
        if (uc < 0x20 || uc > 0x7E) return false; // disallow control/non-ASCII
        if (std::isspace(uc)) return false;       // no spaces
        if (std::isupper(uc)) hasUpper = true;
        else if (std::islower(uc)) hasLower = true;
        else if (std::isdigit(uc)) hasDigit = true;
        else hasSpecial = true; // ASCII punctuation
    }
    return hasUpper && hasLower && hasDigit && hasSpecial;
}

static bool constantTimeEquals(const std::vector<unsigned char> &a, const std::vector<unsigned char> &b) {
    size_t n = std::max(a.size(), b.size());
    unsigned char diff = 0;
    for (size_t i = 0; i < n; ++i) {
        unsigned char av = (i < a.size()) ? a[i] : 0;
        unsigned char bv = (i < b.size()) ? b[i] : 0;
        diff |= (unsigned char)(av ^ bv);
    }
    return diff == 0 && a.size() == b.size();
}

static bool hexToBytes(const std::string &hex, std::vector<unsigned char> &out) {
    if (hex.size() == 0 || hex.size() > 4096 || (hex.size() % 2) != 0) return false;
    out.clear();
    out.reserve(hex.size() / 2);
    auto hexVal = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
        if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
        return -1;
    };
    for (size_t i = 0; i < hex.size(); i += 2) {
        int hi = hexVal(hex[i]);
        int lo = hexVal(hex[i + 1]);
        if (hi < 0 || lo < 0) return false;
        out.push_back((unsigned char)((hi << 4) | lo));
    }
    return true;
}

static std::string bytesToHex(const std::vector<unsigned char> &in) {
    static const char *digits = "0123456789abcdef";
    std::string out;
    out.reserve(in.size() * 2);
    for (unsigned char b : in) {
        out.push_back(digits[b >> 4]);
        out.push_back(digits[b & 0x0F]);
    }
    return out;
}

static bool deriveKeyPBKDF2(const std::string &password,
                            const std::vector<unsigned char> &salt,
                            int iterations,
                            size_t dkLen,
                            std::vector<unsigned char> &out) {
    if (iterations < 10000 || iterations > 10000000) return false;
    if (salt.empty() || dkLen == 0 || dkLen > 1024) return false;
    out.assign(dkLen, 0);
    int rc = PKCS5_PBKDF2_HMAC(password.c_str(),
                               (int)password.size(),
                               salt.data(),
                               (int)salt.size(),
                               iterations,
                               EVP_sha256(),
                               (int)dkLen,
                               out.data());
    if (rc != 1) return false;
    return true;
}

static bool secureRandomBytes(std::vector<unsigned char> &buf, size_t n) {
    buf.assign(n, 0);
    if (n == 0) return true;
    int rc = RAND_bytes(buf.data(), (int)n);
    return rc == 1;
}

static void secureShuffle(std::string &s) {
    for (size_t i = s.size(); i > 1; --i) {
        unsigned int r = 0;
        RAND_bytes(reinterpret_cast<unsigned char *>(&r), sizeof(r));
        size_t j = (size_t)(r % i);
        std::swap(s[i - 1], s[j]);
    }
}

static std::string generatePolicyCompliantPassword(size_t length) {
    const std::string lowers = "abcdefghijklmnopqrstuvwxyz";
    const std::string uppers = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const std::string digits = "0123456789";
    const std::string specials = "!@#$%^&*()-_=+[]{};:,<.>/?\\|`~";
    const std::string all = lowers + uppers + digits + specials;

    if (length < 12) length = 12;
    std::string pwd;
    pwd.reserve(length);

    auto pickFrom = [](const std::string &set) -> char {
        unsigned int r = 0;
        RAND_bytes(reinterpret_cast<unsigned char *>(&r), sizeof(r));
        return set[r % set.size()];
    };

    // Ensure complexity
    pwd.push_back(pickFrom(lowers));
    pwd.push_back(pickFrom(uppers));
    pwd.push_back(pickFrom(digits));
    pwd.push_back(pickFrom(specials));

    // Fill remaining
    while (pwd.size() < length) {
        pwd.push_back(pickFrom(all));
    }
    secureShuffle(pwd);
    return pwd;
}

static bool setEnvVar(const std::string &name, const std::string &value) {
#if defined(_WIN32)
    return _putenv_s(name.c_str(), value.c_str()) == 0;
#else
    return setenv(name.c_str(), value.c_str(), 1) == 0;
#endif
}

static bool getEnvStr(const char *name, std::string &out) {
    const char *val = std::getenv(name);
    if (!val) return false;
    out.assign(val);
    return true;
}

static bool verifyAdminPassword(const std::string &password) {
    if (!meetsPasswordPolicy(password)) {
        return false;
    }

    std::string saltHex, hashHex, iterStr;
    if (!getEnvStr("ADMIN_PW_SALT", saltHex)) return false;
    if (!getEnvStr("ADMIN_PW_HASH", hashHex)) return false;
    int iterations = 120000;
    if (getEnvStr("ADMIN_PW_ITERS", iterStr)) {
        try {
            long long v = std::stoll(iterStr);
            if (v < 10000 || v > 10000000) return false;
            iterations = static_cast<int>(v);
        } catch (...) {
            return false;
        }
    }

    std::vector<unsigned char> salt;
    std::vector<unsigned char> storedHash;
    if (!hexToBytes(saltHex, salt)) return false;
    if (!hexToBytes(hashHex, storedHash)) return false;
    if (salt.size() < 16) return false;
    if (storedHash.size() < 16 || storedHash.size() > 64) return false;

    std::vector<unsigned char> derived;
    if (!deriveKeyPBKDF2(password, salt, iterations, storedHash.size(), derived)) return false;

    bool ok = constantTimeEquals(derived, storedHash);

    // Clean sensitive derived material
    if (!derived.empty()) OPENSSL_cleanse(derived.data(), derived.size());

    return ok;
}

int main() {
    // Setup: generate admin credentials securely for testing without printing secrets
    const size_t saltLen = 16;
    std::vector<unsigned char> salt;
    if (!secureRandomBytes(salt, saltLen)) {
        std::cerr << "Secure RNG failure\n";
        return 1;
    }
    std::string adminPwd = generatePolicyCompliantPassword(16);
    int iterations = 150000;

    std::vector<unsigned char> adminHash;
    if (!deriveKeyPBKDF2(adminPwd, salt, iterations, 32, adminHash)) {
        std::cerr << "Derive failed\n";
        return 1;
    }

    std::string saltHex = bytesToHex(salt);
    std::string hashHex = bytesToHex(adminHash);
    std::string iterStr = std::to_string(iterations);

    if (!setEnvVar("ADMIN_PW_SALT", saltHex) ||
        !setEnvVar("ADMIN_PW_HASH", hashHex) ||
        !setEnvVar("ADMIN_PW_ITERS", iterStr)) {
        std::cerr << "Failed to set environment\n";
        return 1;
    }

    // Prepare test passwords
    std::string correct = adminPwd;

    std::string wrong = correct;
    // Flip last character deterministically without revealing it
    if (!wrong.empty()) {
        char c = wrong.back();
        // simple transformation keeping printable range
        if (std::isdigit(static_cast<unsigned char>(c))) wrong.back() = (c == '9') ? '0' : (char)(c + 1);
        else if (std::isalpha(static_cast<unsigned char>(c))) wrong.back() = (char)((std::tolower(c) == 'z' || std::toupper(c) == 'Z') ? (std::islower(c) ? 'a' : 'A') : c + 1);
        else wrong.back() = (c == '!') ? '@' : '!';
    }

    std::string tooShort = "Aa1!Aa1!Aa"; // length 10 -> should fail policy
    std::string noUpper = "lowercase1!more"; // lacks uppercase
    // Tamper test: modify stored hash
    std::string tamperedHashHex = hashHex;
    if (!tamperedHashHex.empty()) {
        tamperedHashHex[0] = (tamperedHashHex[0] == 'f') ? '0' : 'f';
    }

    // Run 5 tests
    int passed = 0, total = 5;

    bool r1 = verifyAdminPassword(correct);
    std::cout << "Test 1 (correct password): " << (r1 ? "PASS" : "FAIL") << "\n";
    passed += r1 ? 1 : 0;

    bool r2 = verifyAdminPassword(wrong);
    std::cout << "Test 2 (wrong password): " << (!r2 ? "PASS" : "FAIL") << "\n";
    passed += (!r2) ? 1 : 0;

    bool r3 = verifyAdminPassword(tooShort);
    std::cout << "Test 3 (too short): " << (!r3 ? "PASS" : "FAIL") << "\n";
    passed += (!r3) ? 1 : 0;

    bool r4 = verifyAdminPassword(noUpper);
    std::cout << "Test 4 (missing uppercase): " << (!r4 ? "PASS" : "FAIL") << "\n";
    passed += (!r4) ? 1 : 0;

    // Tamper env hash
    setEnvVar("ADMIN_PW_HASH", tamperedHashHex);
    bool r5 = verifyAdminPassword(correct);
    std::cout << "Test 5 (tampered hash): " << (!r5 ? "PASS" : "FAIL") << "\n";
    passed += (!r5) ? 1 : 0;

    // Restore hash env (cleanup)
    setEnvVar("ADMIN_PW_HASH", hashHex);

    // Clean sensitive buffers
    if (!adminHash.empty()) OPENSSL_cleanse(adminHash.data(), adminHash.size());
    if (!salt.empty()) OPENSSL_cleanse(salt.data(), salt.size());
    // Attempt to cleanse password string
    for (char &ch : adminPwd) ch = '\0';

    std::cout << "Passed " << passed << " of " << total << " tests\n";
    return (passed == total) ? 0 : 2;
}