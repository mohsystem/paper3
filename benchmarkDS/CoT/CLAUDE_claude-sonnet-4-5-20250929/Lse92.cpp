
#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <regex>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <iomanip>
#include <sstream>

class Lse92 {
private:
    static const int SALT_LENGTH = 16;
    static const int HASH_ITERATIONS = 100000;
    static const int MIN_PASSWORD_LENGTH = 8;
    static const int MAX_PASSWORD_LENGTH = 128;
    static const int HASH_LENGTH = 32;

    static std::string base64_encode(const unsigned char* data, size_t len) {
        static const char base64_chars[] = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        
        std::string result;
        int i = 0;
        unsigned char char_array_3[3];
        unsigned char char_array_4[4];

        while (len--) {
            char_array_3[i++] = *(data++);
            if (i == 3) {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                for(i = 0; i < 4; i++)
                    result += base64_chars[char_array_4[i]];
                i = 0;
            }
        }

        if (i) {
            for(int j = i; j < 3; j++)
                char_array_3[j] = '\\0';

            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

            for (int j = 0; j < i + 1; j++)
                result += base64_chars[char_array_4[j]];

            while(i++ < 3)
                result += '=';
        }

        return result;
    }

    static bool hash_password(const std::string& password, const unsigned char* salt, 
                             unsigned char* output) {
        return PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),
                                 salt, SALT_LENGTH,
                                 HASH_ITERATIONS,
                                 EVP_sha256(),
                                 HASH_LENGTH,
                                 output) == 1;
    }

public:
    struct RegistrationResult {
        bool success;
        std::string message;
        std::string hashed_password;
        std::string salt;

        RegistrationResult(bool s, const std::string& m, const std::string& h, const std::string& sa)
            : success(s), message(m), hashed_password(h), salt(sa) {}
    };

    static RegistrationResult register_user(const std::string& username, const std::string& password) {
        // Input validation
        if (username.empty() || username.find_first_not_of(' ') == std::string::npos) {
            return RegistrationResult(false, "Username cannot be empty", "", "");
        }

        std::regex username_pattern("^[a-zA-Z0-9_]{3,20}$");
        if (!std::regex_match(username, username_pattern)) {
            return RegistrationResult(false, "Invalid username format", "", "");
        }

        if (password.length() < MIN_PASSWORD_LENGTH) {
            return RegistrationResult(false, "Password too short", "", "");
        }

        if (password.length() > MAX_PASSWORD_LENGTH) {
            return RegistrationResult(false, "Password too long", "", "");
        }

        try {
            // Generate secure random salt
            unsigned char salt[SALT_LENGTH];
            if (RAND_bytes(salt, SALT_LENGTH) != 1) {
                return RegistrationResult(false, "Registration failed", "", "");
            }

            // Hash password
            unsigned char hash[HASH_LENGTH];
            if (!hash_password(password, salt, hash)) {
                return RegistrationResult(false, "Registration failed", "", "");
            }

            std::string hashed_base64 = base64_encode(hash, HASH_LENGTH);
            std::string salt_base64 = base64_encode(salt, SALT_LENGTH);

            return RegistrationResult(true, "Registration successful", hashed_base64, salt_base64);
        } catch (...) {
            return RegistrationResult(false, "Registration failed", "", "");
        }
    }
};

int main() {
    std::cout << "Registration Form - Test Cases\\n\\n";

    // Test case 1: Valid registration
    auto result1 = Lse92::register_user("john_doe", "SecurePass123!");
    std::cout << "Test 1 - Valid user:\\n";
    std::cout << "Success: " << (result1.success ? "true" : "false") 
              << ", Message: " << result1.message << "\\n";
    if (result1.success) {
        std::cout << "Hash: " << result1.hashed_password.substr(0, 20) << "...\\n";
    }

    // Test case 2: Short password
    auto result2 = Lse92::register_user("alice", "pass");
    std::cout << "\\nTest 2 - Short password:\\n";
    std::cout << "Success: " << (result2.success ? "true" : "false") 
              << ", Message: " << result2.message << "\\n";

    // Test case 3: Invalid username
    auto result3 = Lse92::register_user("user@name!", "ValidPass123");
    std::cout << "\\nTest 3 - Invalid username:\\n";
    std::cout << "Success: " << (result3.success ? "true" : "false") 
              << ", Message: " << result3.message << "\\n";

    // Test case 4: Empty username
    auto result4 = Lse92::register_user("", "ValidPass123");
    std::cout << "\\nTest 4 - Empty username:\\n";
    std::cout << "Success: " << (result4.success ? "true" : "false") 
              << ", Message: " << result4.message << "\\n";

    // Test case 5: Valid registration
    auto result5 = Lse92::register_user("user_123", "MyP@ssw0rd!2024");
    std::cout << "\\nTest 5 - Valid user with special chars:\\n";
    std::cout << "Success: " << (result5.success ? "true" : "false") 
              << ", Message: " << result5.message << "\\n";
    if (result5.success) {
        std::cout << "Hash: " << result5.hashed_password.substr(0, 20) << "...\\n";
    }

    return 0;
}
