
#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

class Task82 {
public:
    /**
     * Generates a cryptographically secure random salt
     * @param length The length of the salt in bytes
     * @return Base64 encoded salt string
     */
    static std::string generateSalt(int length) {
        std::vector<unsigned char> salt(length);
        if (RAND_bytes(salt.data(), length) != 1) {
            throw std::runtime_error("Failed to generate random salt");
        }
        return base64Encode(salt);
    }
    
    /**
     * Computes SHA-256 hash of password combined with salt
     * @param password The password to hash
     * @param salt The salt to combine with password
     * @return Hexadecimal encoded hash string
     */
    static std::string hashPasswordWithSalt(const std::string& password, const std::string& salt) {
        if (password.empty() || salt.empty()) {
            throw std::invalid_argument("Password and salt cannot be empty");
        }
        
        // Combine password and salt
        std::string combined = password + salt;
        
        // Compute SHA-256 hash
        unsigned char hash[SHA256_DIGEST_LENGTH];
        EVP_MD_CTX* context = EVP_MD_CTX_new();
        if (context == nullptr) {
            throw std::runtime_error("Failed to create EVP context");
        }
        
        if (EVP_DigestInit_ex(context, EVP_sha256(), nullptr) != 1 ||
            EVP_DigestUpdate(context, combined.c_str(), combined.length()) != 1 ||
            EVP_DigestFinal_ex(context, hash, nullptr) != 1) {
            EVP_MD_CTX_free(context);
            throw std::runtime_error("Failed to compute hash");
        }
        
        EVP_MD_CTX_free(context);
        
        // Convert to hexadecimal string
        return bytesToHex(hash, SHA256_DIGEST_LENGTH);
    }
    
    /**
     * Verifies a password against a stored hash using constant-time comparison
     * @param password The password to verify
     * @param salt The salt used in original hash
     * @param storedHash The stored hash to compare against
     * @return true if password matches, false otherwise
     */
    static bool verifyPassword(const std::string& password, const std::string& salt, 
                               const std::string& storedHash) {
        std::string computedHash = hashPasswordWithSalt(password, salt);
        return constantTimeCompare(computedHash, storedHash);
    }
    
private:
    /**
     * Encodes bytes to Base64
     */
    static std::string base64Encode(const std::vector<unsigned char>& data) {
        static const char base64_chars[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";
        
        std::string result;
        int i = 0;
        int j = 0;
        unsigned char char_array_3[3];
        unsigned char char_array_4[4];
        size_t in_len = data.size();
        
        for (size_t idx = 0; idx < in_len; idx++) {
            char_array_3[i++] = data[idx];
            if (i == 3) {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;
                
                for (i = 0; i < 4; i++)
                    result += base64_chars[char_array_4[i]];
                i = 0;
            }
        }
        
        if (i) {
            for (j = i; j < 3; j++)
                char_array_3[j] = '\\0';
            
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            
            for (j = 0; j < i + 1; j++)
                result += base64_chars[char_array_4[j]];
            
            while (i++ < 3)
                result += '=';
        }
        
        return result;
    }
    
    /**
     * Converts bytes to hexadecimal string
     */
    static std::string bytesToHex(const unsigned char* data, size_t length) {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (size_t i = 0; i < length; i++) {
            ss << std::setw(2) << static_cast<int>(data[i]);
        }
        return ss.str();
    }
    
    /**
     * Constant-time string comparison to prevent timing attacks
     */
    static bool constantTimeCompare(const std::string& a, const std::string& b) {
        if (a.length() != b.length()) {
            return false;
        }
        
        volatile unsigned char result = 0;
        for (size_t i = 0; i < a.length(); i++) {
            result |= a[i] ^ b[i];
        }
        
        return result == 0;
    }
};

int main() {
    std::cout << "=== Password Hashing with Salt - Test Cases ===\\n\\n";
    
    // Test Case 1: Basic password hashing
    std::cout << "Test Case 1: Basic password hashing\\n";
    std::string password1 = "MySecurePassword123!";
    std::string salt1 = Task82::generateSalt(16);
    std::string hash1 = Task82::hashPasswordWithSalt(password1, salt1);
    std::cout << "Password: " << password1 << "\\n";
    std::cout << "Salt: " << salt1 << "\\n";
    std::cout << "Hash: " << hash1 << "\\n";
    std::cout << "Verification: " << (Task82::verifyPassword(password1, salt1, hash1) ? "true" : "false") << "\\n\\n";
    
    // Test Case 2: Different password, same salt
    std::cout << "Test Case 2: Different password, same salt\\n";
    std::string password2 = "DifferentPassword456#";
    std::string hash2 = Task82::hashPasswordWithSalt(password2, salt1);
    std::cout << "Password: " << password2 << "\\n";
    std::cout << "Salt: " << salt1 << "\\n";
    std::cout << "Hash: " << hash2 << "\\n";
    std::cout << "Hashes are different: " << ((hash1 != hash2) ? "true" : "false") << "\\n\\n";
    
    // Test Case 3: Same password, different salt
    std::cout << "Test Case 3: Same password, different salt\\n";
    std::string salt3 = Task82::generateSalt(16);
    std::string hash3 = Task82::hashPasswordWithSalt(password1, salt3);
    std::cout << "Password: " << password1 << "\\n";
    std::cout << "Salt: " << salt3 << "\\n";
    std::cout << "Hash: " << hash3 << "\\n";
    std::cout << "Hashes are different: " << ((hash1 != hash3) ? "true" : "false") << "\\n\\n";
    
    // Test Case 4: Complex password with special characters
    std::cout << "Test Case 4: Complex password with special characters\\n";
    std::string password4 = "P@ssw0rd!#$%^&*()_+-=[]{}|;:',.<>?/~`";
    std::string salt4 = Task82::generateSalt(32);
    std::string hash4 = Task82::hashPasswordWithSalt(password4, salt4);
    std::cout << "Password: " << password4 << "\\n";
    std::cout << "Salt: " << salt4 << "\\n";
    std::cout << "Hash: " << hash4 << "\\n";
    std::cout << "Verification: " << (Task82::verifyPassword(password4, salt4, hash4) ? "true" : "false") << "\\n\\n";
    
    // Test Case 5: Wrong password verification
    std::cout << "Test Case 5: Wrong password verification\\n";
    std::string wrongPassword = "WrongPassword";
    bool verificationResult = Task82::verifyPassword(wrongPassword, salt1, hash1);
    std::cout << "Correct Password: " << password1 << "\\n";
    std::cout << "Wrong Password: " << wrongPassword << "\\n";
    std::cout << "Verification with wrong password: " << (verificationResult ? "true" : "false") << "\\n\\n";
    
    return 0;
}
