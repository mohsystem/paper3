
#include <iostream>
#include <string>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>
#include <cstring>

const char* DB_PATH = "users.db";

std::string hashPassword(const std::string& password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password.c_str(), password.length(), hash);
    
    std::stringstream ss;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

void initializeDatabase() {
    sqlite3* db;
    char* errMsg = nullptr;
    
    int rc = sqlite3_open(DB_PATH, &db);
    if (rc) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        return;
    }
    
    const char* sql = "CREATE TABLE IF NOT EXISTS users ("
                     "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                     "username TEXT UNIQUE NOT NULL,"
                     "password_hash TEXT NOT NULL)";
    
    rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    
    sqlite3_close(db);
}

bool registerUser(const std::string& username, const std::string& password) {
    if (username.empty() || password.empty()) {
        return false;
    }
    
    std::string hashedPassword = hashPassword(password);
    sqlite3* db;
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_open(DB_PATH, &db);
    if (rc) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    const char* sql = "INSERT INTO users (username, password_hash) VALUES (?, ?)";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hashedPassword.c_str(), -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    if (rc != SQLITE_DONE) {
        std::cerr << "Registration error" << std::endl;
        return false;
    }
    
    return true;
}

int main() {
    initializeDatabase();
    
    // Test case 1: Valid registration
    std::cout << "Test 1 - Valid registration: " 
              << (registerUser("john_doe", "password123") ? "true" : "false") << std::endl;
    
    // Test case 2: Another valid registration
    std::cout << "Test 2 - Valid registration: " 
              << (registerUser("jane_smith", "securePass456") ? "true" : "false") << std::endl;
    
    // Test case 3: Duplicate username (should fail)
    std::cout << "Test 3 - Duplicate username: " 
              << (registerUser("john_doe", "differentPassword") ? "true" : "false") << std::endl;
    
    // Test case 4: Empty username (should fail)
    std::cout << "Test 4 - Empty username: " 
              << (registerUser("", "password789") ? "true" : "false") << std::endl;
    
    // Test case 5: Empty password (should fail)
    std::cout << "Test 5 - Empty password: " 
              << (registerUser("bob_jones", "") ? "true" : "false") << std::endl;
    
    return 0;
}
