
#include <iostream>
#include <string>
#include <random>
#include <sstream>
#include <iomanip>
#include <sqlite3.h>
#include <openssl/sha.h>

std::string generate_salt() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    unsigned char salt[16];
    for (int i = 0; i < 16; i++) {
        salt[i] = dis(gen);
    }
    
    std::stringstream ss;
    for (int i = 0; i < 16; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)salt[i];
    }
    return ss.str();
}

std::string hash_password(const std::string& password, const std::string& salt) {
    std::string combined = salt + password;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)combined.c_str(), combined.length(), hash);
    
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

bool insert_user(const std::string& username, const std::string& password) {
    std::string salt = generate_salt();
    std::string hashed_password = hash_password(password, salt);
    
    sqlite3* db;
    char* errMsg = nullptr;
    
    if (sqlite3_open("users.db", &db) != SQLITE_OK) {
        std::cerr << "Error opening database" << std::endl;
        return false;
    }
    
    // Create table if not exists
    const char* createTableSQL = "CREATE TABLE IF NOT EXISTS users ("
                                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                "username TEXT NOT NULL UNIQUE,"
                                "password TEXT NOT NULL,"
                                "salt TEXT NOT NULL)";
    
    if (sqlite3_exec(db, createTableSQL, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Error creating table: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return false;
    }
    
    // Insert user
    sqlite3_stmt* stmt;
    const char* insertSQL = "INSERT INTO users (username, password, salt) VALUES (?, ?, ?)";
    
    if (sqlite3_prepare_v2(db, insertSQL, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Error preparing statement" << std::endl;
        sqlite3_close(db);
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hashed_password.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, salt.c_str(), -1, SQLITE_TRANSIENT);
    
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return success;
}

int main() {
    // Test cases
    std::cout << "Test 1: " << insert_user("alice", "password123") << std::endl;
    std::cout << "Test 2: " << insert_user("bob", "securePass456") << std::endl;
    std::cout << "Test 3: " << insert_user("charlie", "mySecret789") << std::endl;
    std::cout << "Test 4: " << insert_user("diana", "p@ssw0rd!") << std::endl;
    std::cout << "Test 5: " << insert_user("eve", "test12345") << std::endl;
    
    return 0;
}
