// Chain-of-Through Step 1: Problem understanding
// Provide 2FA with random OTP, expiry, attempt limits, and constant-time compares.

// Chain-of-Through Step 2: Security requirements
// - Use std::random_device for non-deterministic randomness per-digit
// - Implement constant-time compare
// - Enforce expiry and attempt limits
// - Avoid logging OTPs (printed only in tests)

#include <iostream>
#include <unordered_map>
#include <string>
#include <chrono>
#include <random>

class TwoFAService {
public:
    TwoFAService(int otpLength = 6, std::chrono::milliseconds ttl = std::chrono::milliseconds(120000), int maxAttempts = 3)
        : otpLength_(otpLength), ttl_(ttl), maxAttempts_(maxAttempts) {
        if (otpLength_ < 4 || otpLength_ > 12) throw std::invalid_argument("OTP length out of bounds");
        if (ttl_.count() < 1000) throw std::invalid_argument("TTL too short");
        if (maxAttempts_ < 1) throw std::invalid_argument("Attempts must be >=1");
    }

    std::string generateOtp(const std::string& userId) {
        if (userId.empty()) throw std::invalid_argument("userId required");
        std::string otp;
        otp.reserve(otpLength_);
        for (int i = 0; i < otpLength_; ++i) {
            int d = static_cast<int>(rd_() % 10);
            otp.push_back(static_cast<char>('0' + d));
        }
        auto now = std::chrono::steady_clock::now();
        OtpRecord rec{otp, now + ttl_, maxAttempts_, true};
        records_[userId] = rec;
        return otp; // In production, send via secure channel
    }

    bool verifyOtp(const std::string& userId, const std::string& otpInput) {
        auto it = records_.find(userId);
        if (it == records_.end()) return false;
        OtpRecord& rec = it->second;
        auto now = std::chrono::steady_clock::now();
        if (!rec.valid) return false;
        if (now > rec.expiresAt) {
            rec.valid = false;
            return false;
        }
        if (rec.attemptsLeft <= 0) {
            rec.valid = false;
            return false;
        }
        bool ok = ctEq(rec.otp, otpInput);
        if (ok) {
            rec.valid = false;
            return true;
        } else {
            rec.attemptsLeft--;
            if (rec.attemptsLeft <= 0) rec.valid = false;
            return false;
        }
    }

    void expireNowForTest(const std::string& userId) {
        auto it = records_.find(userId);
        if (it != records_.end()) {
            it->second.expiresAt = std::chrono::steady_clock::now() - std::chrono::milliseconds(1);
        }
    }

private:
    struct OtpRecord {
        std::string otp;
        std::chrono::steady_clock::time_point expiresAt;
        int attemptsLeft;
        bool valid;
    };

    static bool ctEq(const std::string& a, const std::string& b) {
        const size_t lenA = a.size();
        const size_t lenB = b.size();
        size_t len = (lenA < lenB) ? lenA : lenB;
        unsigned char acc = 0;
        for (size_t i = 0; i < len; ++i) {
            acc |= static_cast<unsigned char>(a[i]) ^ static_cast<unsigned char>(b[i]);
        }
        acc |= static_cast<unsigned char>(lenA) ^ static_cast<unsigned char>(lenB);
        return acc == 0;
    }

    int otpLength_;
    std::chrono::milliseconds ttl_;
    int maxAttempts_;
    std::unordered_map<std::string, OtpRecord> records_;
    std::random_device rd_;
};

class UserStore {
public:
    void addUser(const std::string& username, const std::string& password) {
        data_[username] = password;
    }
    bool verifyPassword(const std::string& username, const std::string& password) const {
        auto it = data_.find(username);
        if (it == data_.end()) return false;
        return ctEq(it->second, password);
    }
private:
    static bool ctEq(const std::string& a, const std::string& b) {
        size_t lenA = a.size(), lenB = b.size();
        size_t len = (lenA < lenB) ? lenA : lenB;
        unsigned char acc = 0;
        for (size_t i = 0; i < len; ++i) {
            acc |= static_cast<unsigned char>(a[i]) ^ static_cast<unsigned char>(b[i]);
        }
        acc |= static_cast<unsigned char>(lenA) ^ static_cast<unsigned char>(lenB);
        return acc == 0;
    }
    std::unordered_map<std::string, std::string> data_;
};

bool loginStep1(const UserStore& store, const std::string& username, const std::string& password) {
    return store.verifyPassword(username, password);
}

int main() {
    // Chain-of-Through Step 3: Secure coding generation
    UserStore store;
    store.addUser("alice", "pass123");
    store.addUser("bob", "secret!");
    store.addUser("carol", "p@ssw0rd");
    store.addUser("dave", "letmein");
    store.addUser("erin", "hunter2");

    TwoFAService twofa(6, std::chrono::milliseconds(2000), 3);

    // Test 1: Success
    {
        bool pw = loginStep1(store, "alice", "pass123");
        std::string otp = pw ? twofa.generateOtp("alice") : "";
        std::cout << "Test1 OTP (simulated send): " << otp << "\n";
        bool ok = pw && twofa.verifyOtp("alice", otp);
        std::cout << "Test1 success: " << (ok ? "true" : "false") << "\n";
    }

    // Test 2: Wrong password
    {
        bool pw = loginStep1(store, "bob", "wrong");
        std::cout << "Test2 success (should be false): " << (pw ? "true" : "false") << "\n";
    }

    // Test 3: Wrong OTP
    {
        bool pw = loginStep1(store, "carol", "p@ssw0rd");
        std::string otp = pw ? twofa.generateOtp("carol") : "";
        std::cout << "Test3 OTP (simulated send): " << otp << "\n";
        bool ok = pw && twofa.verifyOtp("carol", "000000");
        std::cout << "Test3 success (should be false): " << (ok ? "true" : "false") << "\n";
    }

    // Test 4: Expired
    {
        bool pw = loginStep1(store, "dave", "letmein");
        std::string otp = pw ? twofa.generateOtp("dave") : "";
        std::cout << "Test4 OTP (simulated send): " << otp << "\n";
        twofa.expireNowForTest("dave");
        bool ok = pw && twofa.verifyOtp("dave", otp);
        std::cout << "Test4 success (should be false): " << (ok ? "true" : "false") << "\n";
    }

    // Test 5: Attempt limit exceeded
    {
        bool pw = loginStep1(store, "erin", "hunter2");
        std::string otp = pw ? twofa.generateOtp("erin") : "";
        std::cout << "Test5 OTP (simulated send): " << otp << "\n";
        bool a = twofa.verifyOtp("erin", "111111");
        bool b = twofa.verifyOtp("erin", "222222");
        bool c = twofa.verifyOtp("erin", "333333");
        bool d = twofa.verifyOtp("erin", otp);
        std::cout << "Test5 sequence (should be false,false,false,false): "
                  << (a ? "true" : "false") << ","
                  << (b ? "true" : "false") << ","
                  << (c ? "true" : "false") << ","
                  << (d ? "true" : "false") << "\n";
    }

    // Chain-of-Through Step 4 & 5: Reviewed and finalized
    return 0;
}