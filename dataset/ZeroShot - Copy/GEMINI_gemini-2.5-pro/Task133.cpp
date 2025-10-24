#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <random>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <regex>

// --- Security Constants ---
const int SALT_LENGTH = 16;
const int TOKEN_LENGTH = 32;
const long TOKEN_VALIDITY_MINUTES = 15;
const int MIN_PASSWORD_LENGTH = 8;
// Password must contain at least one digit, one lowercase, one uppercase, and one special character.
const std::regex PASSWORD_POLICY("^(?=.*[0-9])(?=.*[a-z])(?=.*[A-Z])(?=.*[@#$%^&+=!])(?=\\S+$).{8,}$");


// --- Embedded SHA256 Implementation ---
// This is a minimal, public-domain SHA256 implementation for demonstration.
// **WARNING**: For production code, use a proper, well-vetted cryptography library
// like OpenSSL or libsodium to implement a key-stretching function like Argon2, scrypt, or PBKDF2.
// A simple salted SHA256 is NOT sufficient for storing passwords securely.
namespace SimpleSHA256 {
    const unsigned int SHA256_K[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };

    void transform(unsigned int state[8], const unsigned char block[64]) {
        unsigned int w[64];
        for (int i = 0; i < 16; ++i) {
            w[i] = (block[i*4] << 24) | (block[i*4+1] << 16) | (block[i*4+2] << 8) | block[i*4+3];
        }
        for (int i = 16; i < 64; ++i) {
            unsigned int s0 = (w[i-15] >> 7 | w[i-15] << 25) ^ (w[i-15] >> 18 | w[i-15] << 14) ^ (w[i-15] >> 3);
            unsigned int s1 = (w[i-2] >> 17 | w[i-2] << 15) ^ (w[i-2] >> 19 | w[i-2] << 13) ^ (w[i-2] >> 10);
            w[i] = w[i-16] + s0 + w[i-7] + s1;
        }

        unsigned int a = state[0], b = state[1], c = state[2], d = state[3], e = state[4], f = state[5], g = state[6], h = state[7];
        for (int i = 0; i < 64; ++i) {
            unsigned int s1 = (e >> 6 | e << 26) ^ (e >> 11 | e << 21) ^ (e >> 25 | e << 7);
            unsigned int ch = (e & f) ^ (~e & g);
            unsigned int temp1 = h + s1 + ch + SHA256_K[i] + w[i];
            unsigned int s0 = (a >> 2 | a << 30) ^ (a >> 13 | a << 19) ^ (a >> 22 | a << 10);
            unsigned int maj = (a & b) ^ (a & c) ^ (b & c);
            unsigned int temp2 = s0 + maj;
            h = g; g = f; f = e; e = d + temp1; d = c; c = b; b = a; a = temp1 + temp2;
        }
        state[0] += a; state[1] += b; state[2] += c; state[3] += d;
        state[4] += e; state[5] += f; state[6] += g; state[7] += h;
    }

    std::string hash(const std::string& input) {
        unsigned char buffer[64];
        unsigned int state[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
        
        uint64_t bitlen = input.length() * 8;
        std::vector<unsigned char> data(input.begin(), input.end());
        data.push_back(0x80);
        while ((data.size() + 8) % 64 != 0) {
            data.push_back(0x00);
        }
        for(int i=0; i<8; ++i) {
            data.push_back((bitlen >> (56 - i*8)) & 0xFF);
        }

        for (size_t i = 0; i < data.size(); i += 64) {
            transform(state, data.data() + i);
        }

        std::stringstream ss;
        for (int i = 0; i < 8; ++i) {
            ss << std::hex << std::setw(8) << std::setfill('0') << state[i];
        }
        return ss.str();
    }
} // namespace SimpleSHA256


struct User {
    std::string username;
    std::string hashedPassword;
    std::string salt;
    std::string resetToken;
    std::chrono::time_point<std::chrono::system_clock> tokenExpiry;
};

// --- In-memory user store (for simulation) ---
static std::unordered_map<std::string, User> userDatabase;


// --- Helper Functions ---
std::string bytes_to_hex(const std::vector<unsigned char>& bytes) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned char byte : bytes) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    return ss.str();
}

std::vector<unsigned char> generate_random_bytes(size_t len) {
    // NOTE: std::random_device may not be non-deterministic on all platforms.
    // For production, use OS-specific APIs like /dev/urandom or CryptGenRandom.
    std::random_device rd;
    std::uniform_int_distribution<int> dist(0, 255);
    std::vector<unsigned char> bytes(len);
    std::generate(bytes.begin(), bytes.end(), [&]() { return dist(rd); });
    return bytes;
}

std::string hash_password(const std::string& password, const std::string& salt) {
    // **SECURITY WARNING**: This is a simplified hash for demonstration.
    // DO NOT USE this in production. Use PBKDF2, scrypt, or Argon2.
    return SimpleSHA256::hash(password + salt);
}


// --- Core Logic ---
std::string requestPasswordReset(const std::string& username) {
    auto it = userDatabase.find(username);
    if (it == userDatabase.end()) {
        return ""; // Return empty string if user not found
    }
    
    std::string token = bytes_to_hex(generate_random_bytes(TOKEN_LENGTH));
    it->second.resetToken = token;
    it->second.tokenExpiry = std::chrono::system_clock::now() + std::chrono::minutes(TOKEN_VALIDITY_MINUTES);
    
    return token;
}

bool resetPassword(const std::string& username, const std::string& token, const std::string& newPassword) {
    auto it = userDatabase.find(username);
    if (it == userDatabase.end()) {
        return false;
    }
    User& user = it->second;

    // 1. Validate token
    if (user.resetToken.empty() || user.resetToken != token) {
        return false;
    }

    // 2. Check token expiration
    if (std::chrono::system_clock::now() > user.tokenExpiry) {
        user.resetToken.clear(); // Expired, invalidate it
        return false;
    }
    
    // 3. Invalidate token immediately
    user.resetToken.clear();
    
    // 4. Validate new password against policy
    if (newPassword.length() < MIN_PASSWORD_LENGTH || !std::regex_match(newPassword, PASSWORD_POLICY)) {
        return false;
    }

    // 5. Update password
    std::string newSalt = bytes_to_hex(generate_random_bytes(SALT_LENGTH));
    user.salt = newSalt;
    user.hashedPassword = hash_password(newPassword, newSalt);
    
    return true;
}

bool checkPassword(const std::string& username, const std::string& password) {
    auto it = userDatabase.find(username);
    if (it == userDatabase.end()) return false;
    return it->second.hashedPassword == hash_password(password, it->second.salt);
}

int main() {
    // --- Setup: Create some users ---
    std::string initialPassAlice = "AlicePass123!";
    std::string saltAlice = bytes_to_hex(generate_random_bytes(SALT_LENGTH));
    userDatabase["alice"] = {"alice", hash_password(initialPassAlice, saltAlice), saltAlice};

    std::string initialPassBob = "BobSecure@2023";
    std::string saltBob = bytes_to_hex(generate_random_bytes(SALT_LENGTH));
    userDatabase["bob"] = {"bob", hash_password(initialPassBob, saltBob), saltBob};
    
    std::cout << "--- Running Password Reset Test Cases ---" << std::endl;

    // --- Test Case 1: Successful Reset ---
    std::cout << "\n[Test Case 1: Successful Reset]" << std::endl;
    std::string token1 = requestPasswordReset("alice");
    std::cout << "Alice requested reset. Token received: " << !token1.empty() << std::endl;
    std::string newPassword1 = "NewSecurePass!456";
    bool success1 = resetPassword("alice", token1, newPassword1);
    std::cout << "Password reset attempt with valid token: " << (success1 ? "SUCCESS" : "FAIL") << std::endl;
    std::cout << "Alice can log in with new password: " << checkPassword("alice", newPassword1) << std::endl;
    std::cout << "Alice cannot log in with old password: " << !checkPassword("alice", initialPassAlice) << std::endl;

    // --- Test Case 2: Invalid Token ---
    std::cout << "\n[Test Case 2: Invalid Token]" << std::endl;
    std::string token2 = requestPasswordReset("bob");
    std::cout << "Bob requested reset. Token received: " << !token2.empty() << std::endl;
    bool success2 = resetPassword("bob", "invalid-token-string", "SomePassword1!");
    std::cout << "Password reset attempt with invalid token: " << (success2 ? "SUCCESS" : "FAIL") << std::endl;
    std::cout << "Bob can still log in with old password: " << checkPassword("bob", initialPassBob) << std::endl;

    // --- Test Case 3: Expired Token ---
    std::cout << "\n[Test Case 3: Expired Token]" << std::endl;
    std::string token3 = requestPasswordReset("bob");
    // Manually expire token
    userDatabase.at("bob").tokenExpiry = std::chrono::system_clock::now() - std::chrono::seconds(1);
    std::cout << "Bob requested reset. Token is now expired." << std::endl;
    bool success3 = resetPassword("bob", token3, "AnotherPass!789");
    std::cout << "Password reset attempt with expired token: " << (success3 ? "SUCCESS" : "FAIL") << std::endl;
    std::cout << "Bob can still log in with old password: " << checkPassword("bob", initialPassBob) << std::endl;

    // --- Test Case 4: Weak Password ---
    std::cout << "\n[Test Case 4: Weak Password]" << std::endl;
    std::string token4 = requestPasswordReset("alice");
    std::cout << "Alice requested reset again." << std::endl;
    bool success4 = resetPassword("alice", token4, "weak");
    std::cout << "Password reset attempt with weak password: " << (success4 ? "SUCCESS" : "FAIL") << std::endl;
    std::cout << "Alice can still log in with her last valid password: " << checkPassword("alice", newPassword1) << std::endl;

    // --- Test Case 5: Non-Existent User ---
    std::cout << "\n[Test Case 5: Non-Existent User]" << std::endl;
    std::string token5 = requestPasswordReset("dave");
    std::cout << "Request reset for non-existent user 'dave'. Token received: " << !token5.empty() << std::endl;
    bool success5 = resetPassword("dave", "any-token", "anyPassword1!");
    std::cout << "Password reset attempt for non-existent user: " << (success5 ? "SUCCESS" : "FAIL") << std::endl;
    
    return 0;
}