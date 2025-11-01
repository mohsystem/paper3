
#include <iostream>
#include <string>
#include <cstring>
#include <regex>
#include <memory>
#include <cstdlib>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <postgresql/libpq-fe.h>

// Password policy constants
constexpr int MIN_PASSWORD_LENGTH = 12;
constexpr int MAX_PASSWORD_LENGTH = 128;
constexpr int MAX_USERNAME_LENGTH = 50;
constexpr int PBKDF2_ITERATIONS = 210000;
constexpr int SALT_LENGTH = 16;
constexpr int HASH_LENGTH = 32;

// Base64 encoding table
static const char base64_chars[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

/**
 * Base64 encode function
 */
std::string base64_encode(const unsigned char* data, size_t len) {
    std::string result;
    int i = 0;
    int j = 0;
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
        for(j = i; j < 3; j++)
            char_array_3[j] = '\\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

        for (j = 0; j < i + 1; j++)
            result += base64_chars[char_array_4[j]];

        while(i++ < 3)
            result += '=';
    }

    return result;
}

/**
 * Generates cryptographically secure random salt
 * Uses OpenSSL RAND_bytes for CSPRNG (CWE-330, CWE-759)
 */
bool generateSalt(unsigned char* salt, int length) {
    if (RAND_bytes(salt, length) != 1) {
        return false;
    }
    return true;
}

/**
 * Hashes password using PBKDF2-HMAC-SHA256 with unique salt
 * Prevents rainbow table attacks (CWE-759)
 * Uses strong KDF (CWE-916)
 */
std::string hashPassword(const std::string& password, const unsigned char* salt) {
    unsigned char hash[HASH_LENGTH];
    
    // Use PBKDF2-HMAC-SHA256
    if (PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),
                          salt, SALT_LENGTH,
                          PBKDF2_ITERATIONS,
                          EVP_sha256(),
                          HASH_LENGTH, hash) != 1) {
        return "";
    }
    
    // Combine salt and hash
    unsigned char combined[SALT_LENGTH + HASH_LENGTH];
    memcpy(combined, salt, SALT_LENGTH);
    memcpy(combined + SALT_LENGTH, hash, HASH_LENGTH);
    
    // Clear sensitive data
    OPENSSL_cleanse(hash, HASH_LENGTH);
    
    return base64_encode(combined, SALT_LENGTH + HASH_LENGTH);
}

/**
 * Validates username to prevent injection (CWE-89)
 * Enforces length and character whitelist (CWE-20)
 */
bool isValidUsername(const std::string& username) {
    if (username.empty() || username.length() > MAX_USERNAME_LENGTH) {
        return false;
    }
    
    // Whitelist: alphanumeric, underscore, hyphen only
    std::regex pattern("^[a-zA-Z0-9_-]+$");
    return std::regex_match(username, pattern);
}

/**
 * Validates password meets security policy (CWE-521)
 */
bool isValidPassword(const std::string& password) {
    if (password.length() < MIN_PASSWORD_LENGTH || 
        password.length() > MAX_PASSWORD_LENGTH) {
        return false;
    }
    
    bool hasUpper = false, hasLower = false;
    bool hasDigit = false, hasSpecial = false;
    
    for (char c : password) {
        if (std::isupper(c)) hasUpper = true;
        if (std::islower(c)) hasLower = true;
        if (std::isdigit(c)) hasDigit = true;
        if (std::string("!@#$%^&*()_+-=[]{}|;:,.<>?").find(c) != std::string::npos) {
            hasSpecial = true;
        }
    }
    
    return hasUpper && hasLower && hasDigit && hasSpecial;
}

/**
 * Registers user with secure password hashing
 * Uses parameterized queries to prevent SQL injection (CWE-89)
 */
bool registerUser(const std::string& username, const std::string& password) {
    // Input validation (CWE-20)
    if (!isValidUsername(username)) {
        std::cerr << "Invalid username format" << std::endl;
        return false;
    }
    
    if (!isValidPassword(password)) {
        std::cerr << "Password does not meet security requirements" << std::endl;
        return false;
    }
    
    // Generate unique salt (CWE-759, CWE-330)
    unsigned char salt[SALT_LENGTH];
    if (!generateSalt(salt, SALT_LENGTH)) {
        std::cerr << "Failed to generate salt" << std::endl;
        return false;
    }
    
    // Hash password with salt (CWE-916)
    std::string hashedPassword = hashPassword(password, salt);
    if (hashedPassword.empty()) {
        std::cerr << "Failed to hash password" << std::endl;
        return false;
    }
    
    // Database connection parameters - should be from environment
    const char* conninfo = std::getenv("DB_CONNINFO");
    if (!conninfo) {
        conninfo = "host=localhost dbname=testdb user=testuser password=testpass";
    }
    
    PGconn* conn = PQconnectdb(conninfo);
    
    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "Database connection failed" << std::endl;
        PQfinish(conn);
        return false;
    }
    
    // Use parameterized query to prevent SQL injection (CWE-89)
    const char* paramValues[2];
    paramValues[0] = username.c_str();
    paramValues[1] = hashedPassword.c_str();
    
    PGresult* res = PQexecParams(conn,
        "INSERT INTO users (username, password_hash) VALUES ($1, $2)",
        2,
        nullptr,
        paramValues,
        nullptr,
        nullptr,
        0);
    
    bool success = (PQresultStatus(res) == PGRES_COMMAND_OK);
    
    if (!success) {
        std::cerr << "Database operation failed" << std::endl;
    }
    
    // Clear sensitive data
    OPENSSL_cleanse(salt, SALT_LENGTH);
    
    PQclear(res);
    PQfinish(conn);
    
    return success;
}

int main() {
    // Test cases
    std::cout << "Test 1: Valid user" << std::endl;
    std::cout << registerUser("testuser1", "SecureP@ssw0rd123") << std::endl;
    
    std::cout << "\\nTest 2: Valid user with special chars" << std::endl;
    std::cout << registerUser("john_doe", "MyP@ssw0rd!2024") << std::endl;
    
    std::cout << "\\nTest 3: Invalid username (too long)" << std::endl;
    std::cout << registerUser(std::string(51, 'a'), "ValidP@ss123") << std::endl;
    
    std::cout << "\\nTest 4: Invalid password (too short)" << std::endl;
    std::cout << registerUser("alice", "Short1!") << std::endl;
    
    std::cout << "\\nTest 5: Invalid username (SQL injection attempt)" << std::endl;
    std::cout << registerUser("user'; DROP TABLE users--", "SecureP@ssw0rd123") << std::endl;
    
    return 0;
}
