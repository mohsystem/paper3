
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cstring>
#include <openssl/sha.h>
#include <openssl/evp.h>

class Task81 {
public:
    /**
     * Securely computes SHA-256 hash of a certificate
     * @param certBytes The certificate bytes
     * @param length Length of the certificate bytes
     * @return Hexadecimal string representation of the hash
     */
    static std::string computeCertificateHash(const unsigned char* certBytes, size_t length) {
        if (certBytes == nullptr || length == 0) {
            throw std::invalid_argument("Certificate bytes cannot be null or empty");
        }
        
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256(certBytes, length, hash);
        
        // Convert to hex string
        std::stringstream ss;
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        return ss.str();
    }
    
    /**
     * Constant-time string comparison to prevent timing attacks
     * @param a First string
     * @param b Second string
     * @return true if strings are equal, false otherwise
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
    
    /**
     * Normalizes hash string by removing colons and whitespace
     * @param hash The hash string to normalize
     * @return Normalized hash string
     */
    static std::string normalizeHash(std::string hash) {
        // Convert to lowercase
        std::transform(hash.begin(), hash.end(), hash.begin(), ::tolower);
        
        // Remove colons and whitespace
        hash.erase(std::remove_if(hash.begin(), hash.end(),
            [](char c) { return c == ':' || std::isspace(c); }), hash.end());
        
        return hash;
    }
    
    /**
     * Validates hash format
     * @param hash The hash string to validate
     * @return true if valid SHA-256 hash format, false otherwise
     */
    static bool isValidHashFormat(const std::string& hash) {
        if (hash.length() != 64) {
            return false;
        }
        
        for (char c : hash) {
            if (!std::isxdigit(c)) {
                return false;
            }
        }
        return true;
    }
    
    /**
     * Securely checks if certificate matches known hash using constant-time comparison
     * @param certBytes The certificate bytes to check
     * @param length Length of the certificate bytes
     * @param knownHash The known hash to compare against (hexadecimal string)
     * @return true if hashes match, false otherwise
     */
    static bool verifyCertificateHash(const unsigned char* certBytes, size_t length, 
                                      const std::string& knownHash) {
        try {
            if (knownHash.empty()) {
                throw std::invalid_argument("Known hash cannot be empty");
            }
            
            // Normalize known hash
            std::string normalizedKnownHash = normalizeHash(knownHash);
            
            // Validate known hash format
            if (!isValidHashFormat(normalizedKnownHash)) {
                throw std::invalid_argument("Invalid hash format. Expected 64 hexadecimal characters.");
            }
            
            std::string computedHash = computeCertificateHash(certBytes, length);
            
            // Use constant-time comparison to prevent timing attacks
            return constantTimeCompare(computedHash, normalizedKnownHash);
            
        } catch (const std::exception& e) {
            std::cerr << "Error during verification: " << e.what() << std::endl;
            return false;
        }
    }
};

int main() {
    std::cout << "=== SSL Certificate Hash Verification Tests ===" << std::endl << std::endl;
    
    // Test Case 1: Valid certificate with matching hash
    try {
        std::string testCert1 = "Test Certificate Data 1";
        const unsigned char* certBytes1 = reinterpret_cast<const unsigned char*>(testCert1.c_str());
        std::string hash1 = Task81::computeCertificateHash(certBytes1, testCert1.length());
        bool result1 = Task81::verifyCertificateHash(certBytes1, testCert1.length(), hash1);
        std::cout << "Test 1 - Valid certificate with matching hash:" << std::endl;
        std::cout << "  Computed Hash: " << hash1 << std::endl;
        std::cout << "  Result: " << (result1 ? "PASS (Hashes match)" : "FAIL") << std::endl;
        std::cout << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 1 failed: " << e.what() << std::endl;
    }
    
    // Test Case 2: Valid certificate with non-matching hash
    try {
        std::string testCert2 = "Test Certificate Data 2";
        const unsigned char* certBytes2 = reinterpret_cast<const unsigned char*>(testCert2.c_str());
        std::string wrongHash = "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
        bool result2 = Task81::verifyCertificateHash(certBytes2, testCert2.length(), wrongHash);
        std::cout << "Test 2 - Valid certificate with non-matching hash:" << std::endl;
        std::cout << "  Expected: FAIL (Hashes don't match)" << std::endl;
        std::cout << "  Result: " << (!result2 ? "PASS (Correctly identified mismatch)" : "FAIL") << std::endl;
        std::cout << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 2 failed: " << e.what() << std::endl;
    }
    
    // Test Case 3: Hash with colons (common format)
    try {
        std::string testCert3 = "Test Certificate Data 3";
        const unsigned char* certBytes3 = reinterpret_cast<const unsigned char*>(testCert3.c_str());
        std::string hash3 = Task81::computeCertificateHash(certBytes3, testCert3.length());
        
        // Add colons to hash
        std::string hashWithColons;
        for (size_t i = 0; i < hash3.length(); i += 2) {
            if (i > 0) hashWithColons += ":";
            hashWithColons += hash3.substr(i, 2);
        }
        
        bool result3 = Task81::verifyCertificateHash(certBytes3, testCert3.length(), hashWithColons);
        std::cout << "Test 3 - Hash with colon separators:" << std::endl;
        std::cout << "  Hash format: " << hashWithColons.substr(0, 20) << "..." << std::endl;
        std::cout << "  Result: " << (result3 ? "PASS (Correctly handled format)" : "FAIL") << std::endl;
        std::cout << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 3 failed: " << e.what() << std::endl;
    }
    
    // Test Case 4: Invalid hash format
    try {
        std::string testCert4 = "Test Certificate Data 4";
        const unsigned char* certBytes4 = reinterpret_cast<const unsigned char*>(testCert4.c_str());
        std::string invalidHash = "invalid_hash_format";
        bool result4 = Task81::verifyCertificateHash(certBytes4, testCert4.length(), invalidHash);
        std::cout << "Test 4 - Invalid hash format:" << std::endl;
        std::cout << "  Expected: FAIL (Invalid format)" << std::endl;
        std::cout << "  Result: " << (!result4 ? "PASS (Correctly rejected invalid format)" : "FAIL") << std::endl;
        std::cout << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 4 failed: " << e.what() << std::endl;
    }
    
    // Test Case 5: Empty certificate bytes
    try {
        const unsigned char* emptyCert = reinterpret_cast<const unsigned char*>("");
        std::string someHash = "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
        bool result5 = Task81::verifyCertificateHash(emptyCert, 0, someHash);
        std::cout << "Test 5 - Empty certificate:" << std::endl;
        std::cout << "  Expected: FAIL (Empty certificate)" << std::endl;
        std::cout << "  Result: " << (!result5 ? "PASS (Correctly handled empty input)" : "FAIL") << std::endl;
        std::cout << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 5 handled gracefully: " << e.what() << std::endl;
    }
    
    return 0;
}
