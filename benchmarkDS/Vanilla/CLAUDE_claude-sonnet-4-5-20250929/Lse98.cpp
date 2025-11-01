
#include <iostream>
#include <string>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>

std::string hashPassword(const std::string& password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password.c_str(), password.length(), hash);
    
    std::stringstream ss;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

bool registerUser(const std::string& username, const std::string& password) {
    std::string hashedPassword = hashPassword(password);
    sqlite3* db;
    char* errMsg = 0;
    
    int rc = sqlite3_open("users.db", &db);
    if(rc) {
        std::cerr << "Error opening database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    const char* createTableSQL = "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT UNIQUE, password TEXT);";
    rc = sqlite3_exec(db, createTableSQL, 0, 0, &errMsg);
    
    if(rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return false;
    }
    
    sqlite3_stmt* stmt;
    const char* insertSQL = "INSERT INTO users (username, password) VALUES (?, ?);";
    rc = sqlite3_prepare_v2(db, insertSQL, -1, &stmt, 0);
    
    if(rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, hashedPassword.c_str(), -1, SQLITE_STATIC);
        rc = sqlite3_step(stmt);
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return rc == SQLITE_DONE;
}

int main() {
    std::cout << "Test Case 1: " << registerUser("alice", "password123") << std::endl;
    std::cout << "Test Case 2: " << registerUser("bob", "securePass456") << std::endl;
    std::cout << "Test Case 3: " << registerUser("charlie", "mySecret789") << std::endl;
    std::cout << "Test Case 4: " << registerUser("diana", "test@2024") << std::endl;
    std::cout << "Test Case 5: " << registerUser("eve", "strongPwd!") << std::endl;
    return 0;
}
