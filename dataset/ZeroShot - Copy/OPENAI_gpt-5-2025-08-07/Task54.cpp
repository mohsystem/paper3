#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <cstring>
#include <chrono>
#include <openssl/evp.h>
#include <openssl/rand.h>

struct User {
    std::vector<unsigned char> salt;
    std::vector<unsigned char> hash;
    int iterations;
};

struct PendingOtp {
    std::string otp;
    std::chrono::steady_clock::time_point expiresAt;
    bool used;
};

class Task54 {
public:
    static constexpr int SALT_LEN = 16;
    static constexpr int HASH_LEN = 32;
    static constexpr int PBKDF2_ITER = 120000;
    static constexpr int OTP_TTL_SECONDS = 120;

    bool register_user(const std::string& username, const std::string& password) {
        if (username.empty() || username.size() > 64 || password.size() < 8) return false;
        if (users.find(username) != users.end()) return false;
        std::vector<unsigned char> salt(SALT_LEN);
        if (RAND_bytes(salt.data(), SALT_LEN) != 1) return false;
        std::vector<unsigned char> hash(HASH_LEN);
        if (!pbkdf2(password, salt, PBKDF2_ITER, HASH_LEN, hash)) return false;
        users.emplace(username, User{salt, hash, PBKDF2_ITER});
        return true;
    }

    std::string login_request(const std::string& username, const std::string& password) {
        auto it = users.find(username);
        if (it == users.end()) return std::string();
        const User& u = it->second;
        std::vector<unsigned char> cand(HASH_LEN);
        if (!pbkdf2(password, u.salt, u.iterations, HASH_LEN, cand)) return std::string();
        if (!ct_eq(u.hash, cand)) return std::string();
        std::string otp = generate_otp();
        pending[username] = PendingOtp{otp, std::chrono::steady_clock::now() + std::chrono::seconds(OTP_TTL_SECONDS), false};
        return otp;
    }

    bool verify_otp(const std::string& username, const std::string& otp) {
        auto it = pending.find(username);
        if (it == pending.end()) return false;
        auto now = std::chrono::steady_clock::now();
        if (it->second.used || now > it->second.expiresAt) {
            pending.erase(it);
            return false;
        }
        bool ok = ct_eq_str(it->second.otp, otp);
        if (ok) {
            it->second.used = true;
            pending.erase(it);
            return true;
        }
        return false;
    }

private:
    std::unordered_map<std::string, User> users;
    std::unordered_map<std::string, PendingOtp> pending;

    static bool pbkdf2(const std::string& password, const std::vector<unsigned char>& salt, int iterations, int dkLen, std::vector<unsigned char>& out) {
        out.resize(dkLen);
        if (PKCS5_PBKDF2_HMAC(password.c_str(), (int)password.size(),
                              salt.data(), (int)salt.size(),
                              iterations, EVP_sha256(), dkLen, out.data()) != 1) {
            return false;
        }
        return true;
    }

    static bool ct_eq(const std::vector<unsigned char>& a, const std::vector<unsigned char>& b) {
        if (a.size() != b.size()) return false;
        unsigned char r = 0;
        for (size_t i = 0; i < a.size(); ++i) r |= (a[i] ^ b[i]);
        return r == 0;
    }

    static bool ct_eq_str(const std::string& a, const std::string& b) {
        if (a.size() != b.size()) return false;
        unsigned char r = 0;
        for (size_t i = 0; i < a.size(); ++i) r |= (static_cast<unsigned char>(a[i]) ^ static_cast<unsigned char>(b[i]));
        return r == 0;
    }

    static std::string generate_otp() {
        // Rejection sampling to avoid modulo bias
        uint32_t val;
        const uint32_t max = 4294000000u; // largest multiple of 1,000,000 below 2^32
        while (true) {
            if (RAND_bytes(reinterpret_cast<unsigned char*>(&val), sizeof(val)) != 1) {
                continue;
            }
            if (val < max) {
                uint32_t n = val % 1000000u;
                char buf[7];
                std::snprintf(buf, sizeof(buf), "%06u", n);
                return std::string(buf);
            }
        }
    }
};

int main() {
    Task54 t;

    // Test 1: Successful login with correct password and OTP
    std::cout << "Register alice: " << (t.register_user("alice", "password123") ? "true" : "false") << "\n";
    std::string otp1 = t.login_request("alice", "password123");
    std::cout << "OTP issued (alice): " << (otp1.empty() ? "None" : otp1) << "\n";
    std::cout << "Verify correct OTP (alice): " << (t.verify_otp("alice", otp1) ? "true" : "false") << "\n";

    // Test 2: Wrong password
    std::string otpWrong = t.login_request("alice", "wrongpass!");
    std::cout << "Login with wrong password returns OTP? " << (!otpWrong.empty() ? "true" : "false") << "\n";

    // Test 3: Wrong OTP
    std::string otp2 = t.login_request("alice", "password123");
    std::cout << "OTP issued (alice) second time: " << (otp2.empty() ? "None" : otp2) << "\n";
    std::cout << "Verify wrong OTP (alice): " << (t.verify_otp("alice", "000000") ? "true" : "false") << "\n";

    // Test 4: Nonexistent user
    std::string otpNoUser = t.login_request("bob", "anything123");
    std::cout << "OTP issued (nonexistent user bob): " << (otpNoUser.empty() ? "None" : otpNoUser) << "\n";

    // Test 5: OTP reuse should fail
    std::string otp3 = t.login_request("alice", "password123");
    std::cout << "OTP issued (alice third time): " << (otp3.empty() ? "None" : otp3) << "\n";
    bool firstUse = t.verify_otp("alice", otp3);
    bool secondUse = t.verify_otp("alice", otp3);
    std::cout << "First use success? " << (firstUse ? "true" : "false") << ", second reuse should fail: " << (secondUse ? "true" : "false") << "\n";

    return 0;
}