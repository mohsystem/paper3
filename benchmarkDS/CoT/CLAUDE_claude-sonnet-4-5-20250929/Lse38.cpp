
#include <iostream>
#include <string>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <cstring>

class Lse38 {
private:
    std::string dbPath;
    
    static std::string base64Encode(const unsigned char* buffer, size_t length) {
        BIO *bio, *b64;
        BUF_MEM *bufferPtr;
        
        b64 = BIO_new(BIO_f_base64());
        bio = BIO_new(BIO_s_mem());
        bio = BIO_push(b64, bio);
        
        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
        BIO_write(bio, buffer, length);
        BIO_flush(bio);
        BIO_get_mem_ptr(bio, &bufferPtr);
        
        std::string result(bufferPtr->data, bufferPtr->length);
        BIO_free_all(bio);
        
        return result;
    }
    
public:
    Lse38(const std::string& path = "users.db") : dbPath(path) {}
    
    static std::string hashPassword(const std::string& password) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256(reinterpret_cast<const unsigned char*>(password.c_str()), 
               password.length(), hash);
        return base64Encode(hash, SHA256_DIGEST_LENGTH);
    }
    
    std::string authenticateUser(const std::string& username, const std::string& password) {
        if (username.empty() || password.empty()) {
            return "login";
        }
        
        std::string hashedPassword = hashPassword(password);
        sqlite3* db;
        sqlite3_stmt* stmt;
        
        if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
            std::cerr << "Database error: " << sqlite3_errmsg(db) << std::endl;
            return "login";
        }
        
        const char* query = "SELECT password_hash FROM users WHERE username = ?";
        
        if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return "login";
        }
        
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        
        std::string result = "login";
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const char* storedHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            if (storedHash && hashedPassword == storedHash) {
                result = "dashboard";
            }
        }
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        return result;
    }
};

int main() {
    Lse38 lse38;
    
    std::cout << "Test Case 1 - Valid user:" << std::endl;
    std::cout << "Redirect to: " << lse38.authenticateUser("admin", "admin123") << std::endl;
    
    std::cout << "\\nTest Case 2 - Invalid password:" << std::endl;
    std::cout << "Redirect to: " << lse38.authenticateUser("admin", "wrongpass") << std::endl;
    
    std::cout << "\\nTest Case 3 - Non-existent user:" << std::endl;
    std::cout << "Redirect to: " << lse38.authenticateUser("nonexistent", "password") << std::endl;
    
    std::cout << "\\nTest Case 4 - Empty credentials:" << std::endl;
    std::cout << "Redirect to: " << lse38.authenticateUser("", "") << std::endl;
    
    std::cout << "\\nTest Case 5 - Whitespace credentials:" << std::endl;
    std::cout << "Redirect to: " << lse38.authenticateUser("   ", "   ") << std::endl;
    
    return 0;
}
