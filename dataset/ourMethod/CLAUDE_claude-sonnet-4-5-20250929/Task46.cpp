
#include <iostream>
#include <string>
#include <regex>
#include <cstring>
#include <vector>
#include <memory>
#include <fstream>
#include <random>
#include <iomanip>
#include <sstream>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

// Security: Use PBKDF2 for password hashing with unique salt per user
// Security: Validate all inputs to prevent injection and buffer overflow
// Security: Use constant-time comparison for password verification
// Security: Clear sensitive data from memory after use

constexpr size_t SALT_SIZE = 16;
constexpr size_t HASH_SIZE = 32;
constexpr int PBKDF2_ITERATIONS = 210000;

class SecureString {
private:
    std::vector<unsigned char> data;
public:
    explicit SecureString(const std::string& str) {
        data.assign(str.begin(), str.end());
    }
    ~SecureString() {
        // Security: Clear sensitive data before destruction
        if (!data.empty()) {
            OPENSSL_cleanse(data.data(), data.size());
        }
    }
    const unsigned char* get() const { return data.data(); }
    size_t size() const { return data.size(); }
};

struct User {
    std::string name;
    std::string email;
    std::vector<unsigned char> salt;
    std::vector<unsigned char> passwordHash;
};

class UserDatabase {
private:
    std::vector<User> users;
    std::string dbFile;

    // Security: Validate name - alphanumeric, spaces, basic punctuation only
    bool validateName(const std::string& name) const {
        if (name.empty() || name.length() > 100) return false;
        std::regex namePattern("^[a-zA-Z0-9 .'-]{1,100}$");
        return std::regex_match(name, namePattern);
    }

    // Security: Validate email format to prevent injection
    bool validateEmail(const std::string& email) const {
        if (email.empty() || email.length() > 254) return false;
        std::regex emailPattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\\\.[a-zA-Z]{2,}$");
        return std::regex_match(email, emailPattern);
    }

    // Security: Validate password requirements
    bool validatePassword(const std::string& password) const {
        // Security: Enforce strong password policy
        if (password.length() < 12 || password.length() > 128) return false;
        
        bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
        for (char c : password) {
            if (std::isupper(c)) hasUpper = true;
            else if (std::islower(c)) hasLower = true;
            else if (std::isdigit(c)) hasDigit = true;
            else if (std::ispunct(c)) hasSpecial = true;
        }
        return hasUpper && hasLower && hasDigit && hasSpecial;
    }

    // Security: Generate cryptographically secure random salt
    bool generateSalt(std::vector<unsigned char>& salt) const {
        salt.resize(SALT_SIZE);
        // Security: Use OpenSSL's CSPRNG for salt generation\n        if (RAND_bytes(salt.data(), SALT_SIZE) != 1) {\n            return false;\n        }\n        return true;\n    }\n\n    // Security: Hash password using PBKDF2-HMAC-SHA256\n    bool hashPassword(const std::string& password, \n                     const std::vector<unsigned char>& salt,\n                     std::vector<unsigned char>& hash) const {\n        hash.resize(HASH_SIZE);\n        SecureString securePass(password);\n        \n        // Security: Use PBKDF2 with 210,000 iterations as per OWASP guidelines\n        if (PKCS5_PBKDF2_HMAC(reinterpret_cast<const char*>(securePass.get()), \n                              securePass.size(),\n                              salt.data(), salt.size(),\n                              PBKDF2_ITERATIONS,\n                              EVP_sha256(),\n                              HASH_SIZE,\n                              hash.data()) != 1) {\n            return false;\n        }\n        return true;\n    }\n\n    // Security: Constant-time comparison to prevent timing attacks\n    bool constantTimeCompare(const std::vector<unsigned char>& a,\n                            const std::vector<unsigned char>& b) const {\n        if (a.size() != b.size()) return false;\n        return CRYPTO_memcmp(a.data(), b.data(), a.size()) == 0;\n    }\n\n    std::string bytesToHex(const std::vector<unsigned char>& bytes) const {\n        std::ostringstream oss;\n        for (unsigned char byte : bytes) {\n            oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);\n        }\n        return oss.str();\n    }\n\n    std::vector<unsigned char> hexToBytes(const std::string& hex) const {\n        std::vector<unsigned char> bytes;\n        // Security: Validate hex string length\n        if (hex.length() % 2 != 0) return bytes;\n        \n        for (size_t i = 0; i < hex.length() && i + 1 < hex.length(); i += 2) {\n            std::string byteString = hex.substr(i, 2);\n            unsigned char byte = static_cast<unsigned char>(std::stoi(byteString, nullptr, 16));\n            bytes.push_back(byte);\n        }\n        return bytes;\n    }\n\npublic:\n    explicit UserDatabase(const std::string& filename) : dbFile(filename) {\n        loadFromFile();\n    }\n\n    // Security: Register user with validated inputs and secure password storage\n    bool registerUser(const std::string& name, const std::string& email, \n                     const std::string& password, std::string& errorMsg) {\n        // Security: Validate all inputs before processing\n        if (!validateName(name)) {\n            errorMsg = "Invalid name format. Use 1-100 alphanumeric characters, spaces, dots, hyphens, apostrophes.";\n            return false;\n        }\n\n        if (!validateEmail(email)) {\n            errorMsg = "Invalid email format.";\n            return false;\n        }\n\n        if (!validatePassword(password)) {\n            errorMsg = "Password must be 12-128 characters with uppercase, lowercase, digit, and special character.";\n            return false;\n        }\n\n        // Security: Check for duplicate email\n        for (const auto& user : users) {\n            if (user.email == email) {\n                errorMsg = "Email already registered.";\n                return false;\n            }\n        }\n\n        User newUser;\n        newUser.name = name;\n        newUser.email = email;\n\n        // Security: Generate unique salt for this user\n        if (!generateSalt(newUser.salt)) {\n            errorMsg = "Failed to generate salt.";\n            return false;\n        }\n\n        // Security: Hash password with salt\n        if (!hashPassword(password, newUser.salt, newUser.passwordHash)) {\n            errorMsg = "Failed to hash password.";\n            return false;\n        }\n\n        users.push_back(newUser);\n        saveToFile();\n        errorMsg = "User registered successfully.";\n        return true;\n    }\n\n    // Security: Verify password with constant-time comparison\n    bool verifyUser(const std::string& email, const std::string& password) const {\n        // Security: Validate inputs\n        if (!validateEmail(email)) return false;\n\n        for (const auto& user : users) {\n            if (user.email == email) {\n                std::vector<unsigned char> computedHash;\n                if (!hashPassword(password, user.salt, computedHash)) {\n                    return false;\n                }\n                // Security: Use constant-time comparison\n                return constantTimeCompare(user.passwordHash, computedHash);\n            }\n        }\n        return false;\n    }\n\n    void saveToFile() const {\n        std::ofstream outFile(dbFile);\n        if (!outFile.is_open()) return;\n\n        for (const auto& user : users) {\n            // Security: Store only non-sensitive data and hashed passwords\n            outFile << user.name << "|" \n                   << user.email << "|"\n                   << bytesToHex(user.salt) << "|"\n                   << bytesToHex(user.passwordHash) << "\\n";\n        }\n        outFile.close();\n    }\n\n    void loadFromFile() {\n        std::ifstream inFile(dbFile);\n        if (!inFile.is_open()) return;\n\n        std::string line;\n        while (std::getline(inFile, line)) {\n            size_t pos1 = line.find('|');\n            size_t pos2 = line.find('|', pos1 + 1);\n            size_t pos3 = line.find('|', pos2 + 1);

            // Security: Validate file format
            if (pos1 == std::string::npos || pos2 == std::string::npos || 
                pos3 == std::string::npos) continue;

            User user;
            user.name = line.substr(0, pos1);
            user.email = line.substr(pos1 + 1, pos2 - pos1 - 1);
            user.salt = hexToBytes(line.substr(pos2 + 1, pos3 - pos2 - 1));
            user.passwordHash = hexToBytes(line.substr(pos3 + 1));

            // Security: Validate loaded data
            if (validateName(user.name) && validateEmail(user.email) &&
                user.salt.size() == SALT_SIZE && user.passwordHash.size() == HASH_SIZE) {
                users.push_back(user);
            }
        }
        inFile.close();
    }

    size_t getUserCount() const { return users.size(); }
};

int main() {
    // Security: Initialize OpenSSL
    OpenSSL_add_all_algorithms();

    UserDatabase db("users.db");

    // Test case 1: Valid registration
    std::string error;
    bool result = db.registerUser("John Doe", "john@example.com", "SecurePass123!", error);
    std::cout << "Test 1 - Valid registration: " << (result ? "SUCCESS" : "FAILED") 
              << " - " << error << std::endl;

    // Test case 2: Weak password
    result = db.registerUser("Jane Smith", "jane@example.com", "weak", error);
    std::cout << "Test 2 - Weak password: " << (!result ? "SUCCESS" : "FAILED") 
              << " - " << error << std::endl;

    // Test case 3: Invalid email
    result = db.registerUser("Bob Wilson", "invalid-email", "StrongPass456!", error);
    std::cout << "Test 3 - Invalid email: " << (!result ? "SUCCESS" : "FAILED") 
              << " - " << error << std::endl;

    // Test case 4: Duplicate email
    result = db.registerUser("John Clone", "john@example.com", "AnotherPass789!", error);
    std::cout << "Test 4 - Duplicate email: " << (!result ? "SUCCESS" : "FAILED") 
              << " - " << error << std::endl;

    // Test case 5: Password verification
    bool verified = db.verifyUser("john@example.com", "SecurePass123!");
    std::cout << "Test 5 - Password verification: " << (verified ? "SUCCESS" : "FAILED") << std::endl;

    // Security: Clean up OpenSSL
    EVP_cleanup();

    std::cout << "\\nTotal users registered: " << db.getUserCount() << std::endl;

    return 0;
}
