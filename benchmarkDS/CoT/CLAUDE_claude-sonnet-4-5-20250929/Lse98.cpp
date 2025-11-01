
#include <iostream>
#include <string>
#include <regex>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <iomanip>
#include <sstream>
#include <cstring>

class Lse98 {
private:
    static const std::string DB_NAME;
    static const int MIN_PASSWORD_LENGTH = 8;
    
    static std::string bytesToHex(const unsigned char* data, size_t length) {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (size_t i = 0; i < length; ++i) {
            ss << std::setw(2) << static_cast<int>(data[i]);
        }
        return ss.str();
    }
    
    static std::string hexToBytes(const std::string& hex) {
        std::string bytes;
        for (size_t i = 0; i < hex.length(); i += 2) {
            std::string byteString = hex.substr(i, 2);
            char byte = static_cast<char>(strtol(byteString.c_str(), nullptr, 16));
            bytes.push_back(byte);
        }
        return bytes;
    }
    
public:
    struct RegistrationResult {
        bool success;
        std::string message;
    };
    
    static void initializeDatabase() {
        sqlite3* db;
        if (sqlite3_open(DB_NAME.c_str(), &db) == SQLITE_OK) {
            const char* sql = "CREATE TABLE IF NOT EXISTS users ("
                            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                            "username TEXT UNIQUE NOT NULL,"
                            "password_hash TEXT NOT NULL,"
                            "salt TEXT NOT NULL,"
                            "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)";
            
            char* errMsg = nullptr;
            sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
            if (errMsg) sqlite3_free(errMsg);
            sqlite3_close(db);
        }
    }
    
    static std::string hashPassword(const std::string& password, const std::string& salt) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        std::string saltBytes = hexToBytes(salt);
        std::string combined = saltBytes + password;
        
        SHA256(reinterpret_cast<const unsigned char*>(combined.c_str()), 
               combined.length(), hash);
        
        return bytesToHex(hash, SHA256_DIGEST_LENGTH);
    }
    
    static std::string generateSalt() {
        unsigned char salt[16];
        RAND_bytes(salt, 16);
        return bytesToHex(salt, 16);
    }
    
    static bool validateUsername(const std::string& username) {
        std::regex pattern("^[a-zA-Z0-9_]{3,20}$");
        return std::regex_match(username, pattern);
    }
    
    static bool validatePassword(const std::string& password) {
        return password.length() >= MIN_PASSWORD_LENGTH;
    }
    
    static RegistrationResult registerUser(const std::string& username, 
                                          const std::string& password) {
        if (!validateUsername(username)) {
            return {false, "Invalid username format"};
        }
        
        if (!validatePassword(password)) {
            return {false, "Password must be at least 8 characters"};
        }
        
        std::string salt = generateSalt();
        std::string hashedPassword = hashPassword(password, salt);
        
        sqlite3* db;
        if (sqlite3_open(DB_NAME.c_str(), &db) != SQLITE_OK) {
            return {false, "Database connection failed"};
        }
        
        const char* sql = "INSERT INTO users (username, password_hash, salt) VALUES (?, ?, ?)";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, hashedPassword.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, salt.c_str(), -1, SQLITE_TRANSIENT);
            
            int result = sqlite3_step(stmt);
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            
            if (result == SQLITE_DONE) {
                return {true, "Registration successful"};
            } else if (result == SQLITE_CONSTRAINT) {
                return {false, "Username already exists"};
            }
        }
        
        sqlite3_close(db);
        return {false, "Registration failed"};
    }
};

const std::string Lse98::DB_NAME = "users.db";

int main() {
    Lse98::initializeDatabase();
    
    std::cout << "=== User Registration Test Cases ===" << std::endl << std::endl;
    
    auto result1 = Lse98::registerUser("john_doe", "SecurePass123");
    std::cout << "Test 1 - Valid registration: " << result1.message << std::endl;
    
    auto result2 = Lse98::registerUser("john_doe", "AnotherPass456");
    std::cout << "Test 2 - Duplicate username: " << result2.message << std::endl;
    
    auto result3 = Lse98::registerUser("ab", "ValidPassword1");
    std::cout << "Test 3 - Invalid username: " << result3.message << std::endl;
    
    auto result4 = Lse98::registerUser("jane_doe", "short");
    std::cout << "Test 4 - Weak password: " << result4.message << std::endl;
    
    auto result5 = Lse98::registerUser("alice_smith", "Str0ng!P@ssw0rd");
    std::cout << "Test 5 - Valid with special chars: " << result5.message << std::endl;
    
    return 0;
}
