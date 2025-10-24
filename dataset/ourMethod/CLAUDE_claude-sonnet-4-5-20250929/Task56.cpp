
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

// Secure token generator using cryptographically secure random bytes
// Prevents CWE-338: Use of Cryptographically Weak PRNG
class TokenGenerator {
private:
    static const size_t TOKEN_BYTES = 32; // 256 bits of entropy
    
    // Convert binary data to hex string for safe transport/storage
    static std::string bytesToHex(const std::vector<unsigned char>& bytes) {
        std::ostringstream oss;
        for (unsigned char byte : bytes) {
            oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
        }
        return oss.str();
    }
    
public:
    // Generate a cryptographically secure authentication token
    // Uses OpenSSL's RAND_bytes which uses a CSPRNG (prevents CWE-338)\n    static std::string generateToken() {\n        std::vector<unsigned char> tokenBytes(TOKEN_BYTES);\n        \n        // Use OpenSSL's cryptographically secure RNG
        // RAND_bytes returns 1 on success, 0 or -1 on failure
        if (RAND_bytes(tokenBytes.data(), TOKEN_BYTES) != 1) {
            throw std::runtime_error("Failed to generate secure random token");
        }
        
        // Convert to hex string for safe storage/transmission
        return bytesToHex(tokenBytes);
    }
    
    // Generate token with user context (binds token to user)
    // Prevents token reuse across different users
    static std::string generateUserToken(const std::string& userId) {
        // Validate input to prevent injection attacks
        if (userId.empty() || userId.length() > 256) {
            throw std::invalid_argument("Invalid userId: must be 1-256 characters");
        }
        
        // Generate secure random bytes
        std::vector<unsigned char> randomBytes(TOKEN_BYTES);
        if (RAND_bytes(randomBytes.data(), TOKEN_BYTES) != 1) {
            throw std::runtime_error("Failed to generate secure random bytes");
        }
        
        // Combine userId with random bytes using SHA-256
        // This binds the token to the specific user
        EVP_MD_CTX* ctx = EVP_MD_CTX_new();
        if (!ctx) {
            throw std::runtime_error("Failed to create hash context");
        }
        
        std::vector<unsigned char> hash(EVP_MAX_MD_SIZE);
        unsigned int hashLen = 0;
        
        try {
            if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1) {
                throw std::runtime_error("Failed to initialize hash");
            }
            
            // Hash user ID
            if (EVP_DigestUpdate(ctx, userId.c_str(), userId.length()) != 1) {
                throw std::runtime_error("Failed to update hash with userId");
            }
            
            // Hash random bytes
            if (EVP_DigestUpdate(ctx, randomBytes.data(), randomBytes.size()) != 1) {
                throw std::runtime_error("Failed to update hash with random data");
            }
            
            if (EVP_DigestFinal_ex(ctx, hash.data(), &hashLen) != 1) {
                throw std::runtime_error("Failed to finalize hash");
            }
            
            EVP_MD_CTX_free(ctx);
        } catch (...) {
            EVP_MD_CTX_free(ctx);
            throw;
        }
        
        hash.resize(hashLen);
        return bytesToHex(hash);
    }
};

int main() {
    try {
        std::cout << "Authentication Token Generator\\n";
        std::cout << "================================\\n\\n";
        
        // Test case 1: Generate basic authentication token
        std::cout << "Test 1 - Basic token generation:\\n";
        std::string token1 = TokenGenerator::generateToken();
        std::cout << "Token: " << token1 << "\\n";
        std::cout << "Length: " << token1.length() << " characters\\n\\n";
        
        // Test case 2: Generate another token (should be different)
        std::cout << "Test 2 - Second token (should be unique):\\n";
        std::string token2 = TokenGenerator::generateToken();
        std::cout << "Token: " << token2 << "\\n";
        std::cout << "Unique: " << (token1 != token2 ? "Yes" : "No") << "\\n\\n";
        
        // Test case 3: Generate user-bound token
        std::cout << "Test 3 - User-bound token for 'user123':\\n";
        std::string userToken1 = TokenGenerator::generateUserToken("user123");
        std::cout << "Token: " << userToken1 << "\\n\\n";
        
        // Test case 4: Generate another token for same user (should be different)
        std::cout << "Test 4 - Another token for 'user123' (should differ):\\n";
        std::string userToken2 = TokenGenerator::generateUserToken("user123");
        std::cout << "Token: " << userToken2 << "\\n";
        std::cout << "Unique: " << (userToken1 != userToken2 ? "Yes" : "No") << "\\n\\n";
        
        // Test case 5: Generate token for different user
        std::cout << "Test 5 - Token for 'admin456':\\n";
        std::string userToken3 = TokenGenerator::generateUserToken("admin456");
        std::cout << "Token: " << userToken3 << "\\n";
        std::cout << "Different from user123: " << (userToken3 != userToken1 ? "Yes" : "No") << "\\n\\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
