
#include <iostream>
#include <string>
#include <map>
#include <random>
#include <chrono>
#include <regex>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

class Task133 {
private:
    static const int TOKEN_EXPIRY_MINUTES = 15;
    static const int MIN_PASSWORD_LENGTH = 8;
    
    struct UserAccount {
        std::string email;
        std::string passwordHash;
        std::string salt;
        
        UserAccount() {}
        UserAccount(const std::string& e, const std::string& ph, const std::string& s)
            : email(e), passwordHash(ph), salt(s) {}
    };
    
    struct ResetToken {
        std::string tokenHash;
        std::string email;
        std::chrono::system_clock::time_point expiryTime;
        bool used;
        
        ResetToken() : used(false) {}
        ResetToken(const std::string& th, const std::string& e, 
                   const std::chrono::system_clock::time_point& et)
            : tokenHash(th), email(e), expiryTime(et), used(false) {}
    };
    
    std::map<std::string, UserAccount> userDatabase;
    std::map<std::string, ResetToken> tokenDatabase;
    
    std::string base64Encode(const unsigned char* buffer, size_t length) {
        static const char base64_chars[] = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string result;
        int i = 0, j = 0;
        unsigned char array_3[3], array_4[4];
        
        while (length--) {
            array_3[i++] = *(buffer++);
            if (i == 3) {
                array_4[0] = (array_3[0] & 0xfc) >> 2;
                array_4[1] = ((array_3[0] & 0x03) << 4) + ((array_3[1] & 0xf0) >> 4);
                array_4[2] = ((array_3[1] & 0x0f) << 2) + ((array_3[2] & 0xc0) >> 6);
                array_4[3] = array_3[2] & 0x3f;
                
                for(i = 0; i < 4; i++)
                    result += base64_chars[array_4[i]];
                i = 0;
            }
        }
        
        if (i) {
            for(j = i; j < 3; j++)
                array_3[j] = '\\0';
            
            array_4[0] = (array_3[0] & 0xfc) >> 2;
            array_4[1] = ((array_3[0] & 0x03) << 4) + ((array_3[1] & 0xf0) >> 4);
            array_4[2] = ((array_3[1] & 0x0f) << 2) + ((array_3[2] & 0xc0) >> 6);
            
            for (j = 0; j < i + 1; j++)
                result += base64_chars[array_4[j]];
            
            while(i++ < 3)
                result += '=';
        }
        
        return result;
    }
    
    std::string generateSalt() {
        unsigned char salt[16];
        RAND_bytes(salt, 16);
        return base64Encode(salt, 16);
    }
    
    std::string hashPassword(const std::string& password, const std::string& salt) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        std::string combined = salt + password;
        
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, combined.c_str(), combined.length());
        SHA256_Final(hash, &sha256);
        
        return base64Encode(hash, SHA256_DIGEST_LENGTH);
    }
    
    std::string generateSecureToken() {
        unsigned char token[32];
        RAND_bytes(token, 32);
        std::string encoded = base64Encode(token, 32);
        encoded.erase(std::remove(encoded.begin(), encoded.end(), '='), encoded.end());
        return encoded;
    }
    
    std::string toLowerCase(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }
    
public:
    Task133() {}
    
    bool registerUser(const std::string& email, const std::string& password) {
        if (email.empty() || email.find_first_not_of(' ') == std::string::npos) {
            return false;
        }
        
        if (!isPasswordValid(password)) {
            return false;
        }
        
        std::string normalizedEmail = toLowerCase(email);
        if (userDatabase.find(normalizedEmail) != userDatabase.end()) {
            return false;
        }
        
        std::string salt = generateSalt();
        std::string passwordHash = hashPassword(password, salt);
        userDatabase[normalizedEmail] = UserAccount(normalizedEmail, passwordHash, salt);
        return true;
    }
    
    std::string requestPasswordReset(const std::string& email) {
        if (email.empty() || email.find_first_not_of(' ') == std::string::npos) {
            return "";
        }
        
        std::string normalizedEmail = toLowerCase(email);
        
        if (userDatabase.find(normalizedEmail) == userDatabase.end()) {
            return "";
        }
        
        std::string token = generateSecureToken();
        std::string tokenHash = hashPassword(token, "");
        auto expiryTime = std::chrono::system_clock::now() + 
                         std::chrono::minutes(TOKEN_EXPIRY_MINUTES);
        
        tokenDatabase[token] = ResetToken(tokenHash, normalizedEmail, expiryTime);
        
        return token;
    }
    
    bool resetPassword(const std::string& token, const std::string& newPassword) {
        if (token.empty() || token.find_first_not_of(' ') == std::string::npos) {
            return false;
        }
        
        if (!isPasswordValid(newPassword)) {
            return false;
        }
        
        auto tokenIt = tokenDatabase.find(token);
        if (tokenIt == tokenDatabase.end()) {
            return false;
        }
        
        ResetToken& resetToken = tokenIt->second;
        
        if (resetToken.used) {
            return false;
        }
        
        if (std::chrono::system_clock::now() > resetToken.expiryTime) {
            tokenDatabase.erase(tokenIt);
            return false;
        }
        
        auto userIt = userDatabase.find(resetToken.email);
        if (userIt == userDatabase.end()) {
            return false;
        }
        
        std::string newSalt = generateSalt();
        std::string newPasswordHash = hashPassword(newPassword, newSalt);
        userIt->second.passwordHash = newPasswordHash;
        userIt->second.salt = newSalt;
        
        resetToken.used = true;
        tokenDatabase.erase(tokenIt);
        
        return true;
    }
    
    bool isPasswordValid(const std::string& password) {
        if (password.empty() || password.length() < MIN_PASSWORD_LENGTH) {
            return false;
        }
        
        std::regex pattern("^(?=.*[a-z])(?=.*[A-Z])(?=.*\\\\d)(?=.*[@$!%*?&])[A-Za-z\\\\d@$!%*?&]{8,}$");
        return std::regex_match(password, pattern);
    }
    
    bool verifyPassword(const std::string& email, const std::string& password) {
        if (email.empty() || password.empty()) {
            return false;
        }
        
        auto userIt = userDatabase.find(toLowerCase(email));
        if (userIt == userDatabase.end()) {
            return false;
        }
        
        std::string hashedPassword = hashPassword(password, userIt->second.salt);
        return hashedPassword == userIt->second.passwordHash;
    }
};

int main() {
    Task133 passwordReset;
    
    std::cout << "=== Test Case 1: Register users ===" << std::endl;
    bool result1 = passwordReset.registerUser("user1@example.com", "SecurePass123!");
    std::cout << "Register user1: " << (result1 ? "true" : "false") << std::endl;
    bool result2 = passwordReset.registerUser("user2@example.com", "AnotherPass456@");
    std::cout << "Register user2: " << (result2 ? "true" : "false") << std::endl;
    
    std::cout << "\\n=== Test Case 2: Request password reset ===" << std::endl;
    std::string token1 = passwordReset.requestPasswordReset("user1@example.com");
    std::cout << "Reset token generated: " << (!token1.empty() ? "true" : "false") << std::endl;
    
    std::cout << "\\n=== Test Case 3: Reset password with valid token ===" << std::endl;
    bool reset1 = passwordReset.resetPassword(token1, "NewSecurePass789!");
    std::cout << "Password reset successful: " << (reset1 ? "true" : "false") << std::endl;
    bool verify1 = passwordReset.verifyPassword("user1@example.com", "NewSecurePass789!");
    std::cout << "New password verified: " << (verify1 ? "true" : "false") << std::endl;
    
    std::cout << "\\n=== Test Case 4: Try to reuse token ===" << std::endl;
    bool reset2 = passwordReset.resetPassword(token1, "AnotherNewPass000!");
    std::cout << "Reuse token (should fail): " << (reset2 ? "true" : "false") << std::endl;
    
    std::cout << "\\n=== Test Case 5: Invalid password reset ===" << std::endl;
    std::string token2 = passwordReset.requestPasswordReset("user2@example.com");
    bool reset3 = passwordReset.resetPassword(token2, "weak");
    std::cout << "Reset with weak password (should fail): " << (reset3 ? "true" : "false") << std::endl;
    bool reset4 = passwordReset.resetPassword("invalid_token", "ValidPass123!");
    std::cout << "Reset with invalid token (should fail): " << (reset4 ? "true" : "false") << std::endl;
    
    return 0;
}
