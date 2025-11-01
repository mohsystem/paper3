
#include <iostream>
#include <string>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <cstring>

class Lse34 {
private:
    static const char* DB_NAME;
    
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
    static std::string hashPassword(const std::string& password) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((unsigned char*)password.c_str(), password.length(), hash);
        return base64Encode(hash, SHA256_DIGEST_LENGTH);
    }
    
    static void initializeDatabase() {
        sqlite3* db;
        char* errMsg = nullptr;
        
        if (sqlite3_open(DB_NAME, &db) != SQLITE_OK) {
            std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
            return;
        }
        
        const char* createTableSQL = 
            "CREATE TABLE IF NOT EXISTS users ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "username TEXT UNIQUE NOT NULL,"
            "password TEXT NOT NULL)";
        
        sqlite3_exec(db, createTableSQL, nullptr, nullptr, &errMsg);
        sqlite3_exec(db, "DELETE FROM users", nullptr, nullptr, &errMsg);
        
        std::string testUsers[][2] = {
            {"admin", "admin123"},
            {"testuser", "password"},
            {"john", "john123"},
            {"jane", "jane456"},
            {"guest", "guest"}
        };
        
        for (const auto& user : testUsers) {
            std::string hashedPwd = hashPassword(user[1]);
            sqlite3_stmt* stmt;
            const char* sql = "INSERT OR IGNORE INTO users (username, password) VALUES (?, ?)";
            
            sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
            sqlite3_bind_text(stmt, 1, user[0].c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, hashedPwd.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
        
        sqlite3_close(db);
    }
    
    static std::string authenticateUser(const std::string& username, const std::string& password) {
        if (username.empty() || password.empty()) {
            return "redirect:/login?error=empty";
        }
        
        std::string hashedPassword = hashPassword(password);
        sqlite3* db;
        
        if (sqlite3_open(DB_NAME, &db) != SQLITE_OK) {
            return "redirect:/login?error=system";
        }
        
        sqlite3_stmt* stmt;
        const char* sql = "SELECT username FROM users WHERE username = ? AND password = ?";
        
        sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, hashedPassword.c_str(), -1, SQLITE_TRANSIENT);
        
        int result = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        if (result == SQLITE_ROW) {
            return "redirect:/dashboard";
        } else {
            return "redirect:/login?error=invalid";
        }
    }
};

const char* Lse34::DB_NAME = "users.db";

int main() {
    Lse34::initializeDatabase();
    
    std::cout << "=== Secure Login System Test Cases ===\\n\\n";
    
    std::cout << "Test 1 - Valid login (admin/admin123):\\n";
    std::cout << "Result: " << Lse34::authenticateUser("admin", "admin123") << "\\n\\n";
    
    std::cout << "Test 2 - Invalid password (admin/wrongpass):\\n";
    std::cout << "Result: " << Lse34::authenticateUser("admin", "wrongpass") << "\\n\\n";
    
    std::cout << "Test 3 - Non-existent user (hacker/hack123):\\n";
    std::cout << "Result: " << Lse34::authenticateUser("hacker", "hack123") << "\\n\\n";
    
    std::cout << "Test 4 - Empty credentials:\\n";
    std::cout << "Result: " << Lse34::authenticateUser("", "") << "\\n\\n";
    
    std::cout << "Test 5 - SQL Injection attempt (admin' OR '1'='1/anything):\\n";
    std::cout << "Result: " << Lse34::authenticateUser("admin' OR '1'='1", "anything") << "\\n\\n";
    
    return 0;
}
