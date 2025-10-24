#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

#include <array>
#include <cctype>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace secure2fa {

static constexpr size_t SALT_LEN = 16;
static constexpr size_t HASH_LEN = 32;
static constexpr size_t OTP_LEN = 6;
static constexpr int PBKDF2_ITERS = 210000;
static constexpr int OTP_TTL_SEC = 120;

struct User {
    std::string username;
    std::string contact;
    std::array<unsigned char, SALT_LEN> salt{};
    std::array<unsigned char, HASH_LEN> hash{};
    int iterations{PBKDF2_ITERS};
};

struct Challenge {
    std::string id;
    std::string username;
    std::array<char, OTP_LEN + 1> otp{}; // null-terminated
    std::time_t expires_at{};
    bool used{false};
};

struct AuthState {
    std::vector<User> users;
    std::vector<Challenge> challenges;
};

static bool secureRandom(unsigned char* buf, size_t len) {
    if (!buf || len == 0) return false;
    return RAND_bytes(buf, static_cast<int>(len)) == 1;
}

static bool deriveKeyPBKDF2(const std::string& password,
                            const unsigned char* salt,
                            size_t salt_len,
                            int iterations,
                            unsigned char* out,
                            size_t out_len) {
    if (!salt || !out || out_len == 0 || salt_len == 0 || iterations < 100000) return false;
    return PKCS5_PBKDF2_HMAC(password.data(),
                             static_cast<int>(password.size()),
                             salt,
                             static_cast<int>(salt_len),
                             iterations,
                             EVP_sha256(),
                             static_cast<int>(out_len),
                             out) == 1;
}

static bool constantTimeEq(const unsigned char* a, const unsigned char* b, size_t len) {
    if (!a || !b) return false;
    return CRYPTO_memcmp(a, b, len) == 0;
}

static bool constantTimeEqChars(const char* a, const char* b, size_t len) {
    if (!a || !b) return false;
    return CRYPTO_memcmp(a, b, len) == 0;
}

static bool toHex(const unsigned char* in, size_t len, std::string& out_hex) {
    if (!in || len == 0) return false;
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (size_t i = 0; i < len; ++i) {
        oss << std::setw(2) << static_cast<unsigned int>(in[i]);
    }
    out_hex = oss.str();
    return true;
}

static bool checkPasswordPolicy(const std::string& pwd) {
    if (pwd.size() < 12) return false;
    bool hasLower = false, hasUpper = false, hasDigit = false, hasSpecial = false;
    for (unsigned char c : pwd) {
        if (std::islower(c)) hasLower = true;
        else if (std::isupper(c)) hasUpper = true;
        else if (std::isdigit(c)) hasDigit = true;
        else if (std::ispunct(c) || std::isspace(c)) hasSpecial = true;
    }
    return hasLower && hasUpper && hasDigit && hasSpecial;
}

static bool generateOtp(std::array<char, OTP_LEN + 1>& out) {
    out.fill('\0');
    size_t produced = 0;
    while (produced < OTP_LEN) {
        unsigned char byte = 0;
        if (!secureRandom(&byte, 1)) return false;
        if (byte >= 250) continue; // rejection sampling to avoid modulo bias
        char digit = static_cast<char>('0' + (byte % 10));
        out[produced++] = digit;
    }
    out[OTP_LEN] = '\0';
    return true;
}

static bool createUser(AuthState& state, const std::string& username,
                       const std::string& password, const std::string& contact) {
    if (username.empty() || contact.empty()) return false;
    for (const auto& u : state.users) {
        if (u.username == username) return false; // already exists
    }
    if (!checkPasswordPolicy(password)) return false;

    User u;
    u.username = username;
    u.contact = contact;

    if (!secureRandom(u.salt.data(), u.salt.size())) return false;
    if (!deriveKeyPBKDF2(password, u.salt.data(), u.salt.size(), u.iterations, u.hash.data(), u.hash.size()))
        return false;

    state.users.push_back(u);
    return true;
}

static const User* findUser(const AuthState& state, const std::string& username) {
    for (const auto& u : state.users) {
        if (u.username == username) return &u;
    }
    return nullptr;
}

static bool startLogin(AuthState& state, const std::string& username, const std::string& password,
                       std::time_t now, std::string& out_challenge_id) {
    out_challenge_id.clear();
    const User* u = findUser(state, username);
    if (!u) return false;

    std::array<unsigned char, HASH_LEN> candidate{};
    if (!deriveKeyPBKDF2(password, u->salt.data(), u->salt.size(), u->iterations, candidate.data(), candidate.size()))
        return false;

    bool ok = constantTimeEq(candidate.data(), u->hash.data(), u->hash.size());
    OPENSSL_cleanse(candidate.data(), candidate.size());
    if (!ok) return false;

    Challenge c;
    // Generate challenge ID (16 bytes -> 32 hex chars)
    std::array<unsigned char, 16> rnd{};
    if (!secureRandom(rnd.data(), rnd.size())) return false;
    if (!toHex(rnd.data(), rnd.size(), c.id)) return false;

    c.username = username;
    if (!generateOtp(c.otp)) return false;
    c.expires_at = now + OTP_TTL_SEC;
    c.used = false;

    state.challenges.push_back(c);
    out_challenge_id = c.id;
    // In real systems, send OTP via secure channel (SMS/Email/Authenticator). Do not log or print it.
    return true;
}

static Challenge* findChallenge(AuthState& state, const std::string& challenge_id) {
    for (auto& c : state.challenges) {
        if (c.id == challenge_id) return &c;
    }
    return nullptr;
}

static bool verifyOtp(AuthState& state, const std::string& challenge_id,
                      const std::string& otp_input, std::time_t now) {
    if (otp_input.size() != OTP_LEN) return false;
    Challenge* c = findChallenge(state, challenge_id);
    if (!c) return false;
    if (c->used) return false;
    if (now > c->expires_at) {
        // wipe OTP on expiration
        OPENSSL_cleanse(c->otp.data(), c->otp.size());
        c->used = true; // invalidate
        return false;
    }
    bool match = constantTimeEqChars(c->otp.data(), otp_input.data(), OTP_LEN);
    // Wipe OTP and mark used regardless of success to prevent reuse
    OPENSSL_cleanse(c->otp.data(), c->otp.size());
    c->used = true;
    return match;
}

// Test helper: retrieve OTP without printing (for automated tests only)
static bool getOtpForTesting(AuthState& state, const std::string& challenge_id, std::string& out_otp) {
    Challenge* c = findChallenge(state, challenge_id);
    if (!c || c->used) return false;
    out_otp.assign(c->otp.data(), OTP_LEN);
    return true;
}

} // namespace secure2fa

int main() {
    using namespace secure2fa;
    AuthState state;

    // Setup test user
    const std::string username = "alice";
    const std::string contact = "alice@example.com";
    const std::string strongPwd = "Str0ngP@ssw0rd!";

    if (!createUser(state, username, strongPwd, contact)) {
        std::cout << "Setup failed\n";
        return 1;
    }

    std::time_t now = std::time(nullptr);

    int passed = 0, total = 0;

    // Test 1: Wrong password should fail
    {
        total++;
        std::string cid;
        bool ok = startLogin(state, username, "WrongP@ssw0rd!", now, cid);
        if (!ok && cid.empty()) {
            std::cout << "Test1 PASS\n";
            passed++;
        } else {
            std::cout << "Test1 FAIL\n";
        }
    }

    // Test 2: Correct password but wrong OTP should fail
    {
        total++;
        std::string cid;
        bool ok = startLogin(state, username, strongPwd, now, cid);
        bool test_ok = false;
        if (ok && !cid.empty()) {
            std::string realOtp;
            if (getOtpForTesting(state, cid, realOtp)) {
                // craft a wrong OTP deterministically
                std::string wrong = realOtp;
                wrong[0] = (wrong[0] == '0') ? '1' : '0';
                if (!verifyOtp(state, cid, wrong, now)) {
                    test_ok = true;
                }
            }
        }
        std::cout << (test_ok ? "Test2 PASS\n" : "Test2 FAIL\n");
        if (test_ok) passed++;
    }

    // Test 3: Correct password and OTP but expired should fail
    {
        total++;
        std::string cid;
        bool ok = startLogin(state, username, strongPwd, now, cid);
        bool test_ok = false;
        if (ok && !cid.empty()) {
            std::string realOtp;
            if (getOtpForTesting(state, cid, realOtp)) {
                std::time_t later = now + 1000; // beyond TTL
                if (!verifyOtp(state, cid, realOtp, later)) {
                    test_ok = true;
                }
            }
        }
        std::cout << (test_ok ? "Test3 PASS\n" : "Test3 FAIL\n");
        if (test_ok) passed++;
    }

    // Test 4: Correct password and OTP should succeed
    {
        total++;
        std::string cid;
        bool ok = startLogin(state, username, strongPwd, now, cid);
        bool test_ok = false;
        if (ok && !cid.empty()) {
            std::string realOtp;
            if (getOtpForTesting(state, cid, realOtp)) {
                if (verifyOtp(state, cid, realOtp, now)) {
                    test_ok = true;
                }
            }
        }
        std::cout << (test_ok ? "Test4 PASS\n" : "Test4 FAIL\n");
        if (test_ok) passed++;
    }

    // Test 5: OTP reuse should fail
    {
        total++;
        std::string cid;
        bool ok = startLogin(state, username, strongPwd, now, cid);
        bool test_ok = false;
        if (ok && !cid.empty()) {
            std::string realOtp;
            if (getOtpForTesting(state, cid, realOtp)) {
                bool first = verifyOtp(state, cid, realOtp, now);
                bool second = verifyOtp(state, cid, realOtp, now);
                if (first && !second) test_ok = true;
            }
        }
        std::cout << (test_ok ? "Test5 PASS\n" : "Test5 FAIL\n");
        if (test_ok) passed++;
    }

    std::cout << "Passed " << passed << " of " << total << " tests\n";
    return 0;
}