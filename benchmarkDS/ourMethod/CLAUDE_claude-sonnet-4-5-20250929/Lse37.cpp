
#include <iostream>
#include <string>
#include <cstring>
#include <memory>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <sqlite3.h>

// Constants for PBKDF2 (Rules#6, #7, #8)
constexpr int ITERATIONS = 210000;
constexpr int KEY_LENGTH = 32;
constexpr int SALT_LENGTH = 16;

// Securely clears sensitive data (Rules#1, #6)
void secure_zero(void* ptr, size_t len) {
    if (ptr == nullptr || len == 0) return;
    #ifdef _WIN32
        SecureZeroMemory(ptr, len);
    #else
        explicit_bzero(ptr, len);
    #endif
}

// Generates cryptographically secure random salt (Rules#8, #9)
bool generate_salt(unsigned char* salt, size_t length) {
    if (salt == nullptr || length == 0) return false;
    return RAND_bytes(salt, static_cast<int>(length)) == 1;
}

// Hashes password using PBKDF2-HMAC-SHA256 (Rules#1, #6, #7, #8)
bool hash_password(const std::string& password, const unsigned char* salt, 
                   size_t salt_len, unsigned char* out_hash, size_t hash_len) {
    if (password.empty() || salt == nullptr || out_hash == nullptr) {
        return false;
    }
    
    int result = PKCS5_PBKDF2_HMAC(
        password.c_str(), static_cast<int>(password.length()),
        salt, static_cast<int>(salt_len),
        ITERATIONS,
        EVP_sha256(),
        static_cast<int>(hash_len),
        out_hash
    );
    
    return result == 1;
}

// Validates input to prevent injection attacks (Rules#1, #13)
bool validate_input(const std::string& input) {
    if (input.empty() || input.length() > 255) {
        return false;
    }
    
    // Allow only alphanumeric and safe characters
    return std::all_of(input.begin(), input.end(), [](unsigned char c) {
        return std::isalnum(c) || c == '_' || c == '@' || c == '.' || c == '-';
    });
}

// Constant-time comparison to prevent timing attacks (Rules#6)
bool constant_time_compare(const unsigned char* a, const unsigned char* b, size_t len) {
    if (a == nullptr || b == nullptr) return false;
    
    unsigned char result = 0;
    for (size_t i = 0; i < len; i++) {
        result |= a[i] ^ b[i];
    }
    return result == 0;
}

// Base64 encode helper
std::string base64_encode(const unsigned char* data, size_t len) {
    static const char encoding_table[] = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string result;
    result.reserve(((len + 2) / 3) * 4);
    
    for (size_t i = 0; i < len; i += 3) {
        unsigned int val = data[i] << 16;
        if (i + 1 < len) val |= data[i + 1] << 8;
        if (i + 2 < len) val |= data[i + 2];
        
        result.push_back(encoding_table[(val >> 18) & 0x3F]);
        result.push_back(encoding_table[(val >> 12) & 0x3F]);
        result.push_back(i + 1 < len ? encoding_table[(val >> 6) & 0x3F] : '=');
        result.push_back(i + 2 < len ? encoding_table[val & 0x3F] : '=');
    }
    return result;
}

// Base64 decode helper
std::vector<unsigned char> base64_decode(const std::string& input) {
    static const unsigned char decoding_table[256] = {
        64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
        64,64,64,64,64,64,64,64,64,64,64,62,64,64,64,63,52,53,54,55,56,57,58,59,60,61,64,64,64,64,64,64,
        64,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,64,64,64,64,64,
        64,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,64,64,64,64,64,
        64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
        64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
        64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
        64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64
    };
    
    std::vector<unsigned char> result;
    unsigned int val = 0;
    int bits = -8;
    
    for (unsigned char c : input) {
        if (decoding_table[c] == 64) break;
        val = (val << 6) | decoding_table[c];
        bits += 6;
        if (bits >= 0) {
            result.push_back((val >> bits) & 0xFF);
            bits -= 8;
        }
    }
    return result;
}

// Authenticates user with parameterized queries (Rules#1, #2, #3, #4)
std::string authenticate_user(const std::string& username, const std::string& password) {
    // Input validation (Rules#1, #13)
    if (!validate_input(username) || password.empty()) {
        return "cant_login";
    }
    
    // Load database path from environment variable (Rules#3, #4)
    const char* db_path_env = std::getenv("DB_PATH");
    std::string db_path = db_path_env ? db_path_env : "users.db";
    
    sqlite3* db = nullptr;
    sqlite3_stmt* stmt = nullptr;
    unsigned char computed_hash[KEY_LENGTH] = {0};
    
    try {
        // Open database connection
        if (sqlite3_open_v2(db_path.c_str(), &db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK) {
            std::cerr << "Database error occurred" << std::endl;
            return "internal_server_error";
        }
        
        // Parameterized query prevents SQL injection (Rules#1, #2)
        const char* query = "SELECT password_hash, salt FROM users WHERE username = ?";
        if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
            sqlite3_close(db);
            std::cerr << "Database error occurred" << std::endl;
            return "internal_server_error";
        }
        
        // Bind username parameter
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        
        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            // Retrieve stored hash and salt
            const unsigned char* stored_hash_b64 = sqlite3_column_text(stmt, 0);
            const unsigned char* salt_b64 = sqlite3_column_text(stmt, 1);
            
            if (stored_hash_b64 == nullptr || salt_b64 == nullptr) {
                secure_zero(computed_hash, KEY_LENGTH);
                sqlite3_finalize(stmt);
                sqlite3_close(db);
                return "internal_server_error";
            }
            
            // Decode base64 salt
            std::vector<unsigned char> salt = base64_decode(
                reinterpret_cast<const char*>(salt_b64)
            );
            std::vector<unsigned char> stored_hash = base64_decode(
                reinterpret_cast<const char*>(stored_hash_b64)
            );
            
            // Hash provided password (Rules#6, #7, #8)
            if (!hash_password(password, salt.data(), salt.size(), 
                             computed_hash, KEY_LENGTH)) {
                secure_zero(computed_hash, KEY_LENGTH);
                sqlite3_finalize(stmt);
                sqlite3_close(db);
                return "internal_server_error";
            }
            
            // Constant-time comparison (Rules#6)
            bool match = constant_time_compare(computed_hash, stored_hash.data(), 
                                              std::min(KEY_LENGTH, static_cast<int>(stored_hash.size())));
            
            // Securely clear sensitive data (Rules#1, #6)
            secure_zero(computed_hash, KEY_LENGTH);
            
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            
            return match ? "success" : "cant_login";
            
        } else if (rc == SQLITE_DONE) {
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return "cant_login";
        } else {
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            std::cerr << "Database error occurred" << std::endl;
            return "internal_server_error";
        }
        
    } catch (...) {
        // Handle exceptions (Rules#17)
        secure_zero(computed_hash, KEY_LENGTH);
        if (stmt) sqlite3_finalize(stmt);
        if (db) sqlite3_close(db);
        std::cerr << "Authentication error occurred" << std::endl;
        return "internal_server_error";
    }
}

int main() {
    // Test cases
    std::cout << "Test 1 - Valid user: " << authenticate_user("validuser", "ValidPass123!") << std::endl;
    std::cout << "Test 2 - Invalid password: " << authenticate_user("validuser", "wrongpass") << std::endl;
    std::cout << "Test 3 - Invalid username: " << authenticate_user("invaliduser", "password") << std::endl;
    std::cout << "Test 4 - SQL injection attempt: " << authenticate_user("admin' OR '1'='1", "pass") << std::endl;
    std::cout << "Test 5 - Empty credentials: " << authenticate_user("", "") << std::endl;
    
    return 0;
}
