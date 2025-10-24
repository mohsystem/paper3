
#include <iostream>
#include <string>
#include <sqlite3.h>
#include <sstream>

class Task37 {
private:
    static const char* DB_NAME;
    
public:
    static void initializeDatabase() {
        sqlite3* db;
        char* errMsg = nullptr;
        
        int rc = sqlite3_open(DB_NAME, &db);
        if (rc) {
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
            return;
        }
        
        const char* createTableSQL = 
            "CREATE TABLE IF NOT EXISTS users ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "username TEXT NOT NULL UNIQUE,"
            "email TEXT,"
            "fullname TEXT)";
        
        rc = sqlite3_exec(db, createTableSQL, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        }
        
        const char* insertSQL = 
            "INSERT OR IGNORE INTO users (username, email, fullname) VALUES "
            "('john_doe', 'john@example.com', 'John Doe'),"
            "('jane_smith', 'jane@example.com', 'Jane Smith'),"
            "('bob_wilson', 'bob@example.com', 'Bob Wilson'),"
            "('alice_brown', 'alice@example.com', 'Alice Brown'),"
            "('charlie_davis', 'charlie@example.com', 'Charlie Davis')";
        
        rc = sqlite3_exec(db, insertSQL, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        }
        
        sqlite3_close(db);
    }
    
    static std::string getUserInfo(const std::string& username) {
        sqlite3* db;
        sqlite3_stmt* stmt;
        std::ostringstream result;
        
        int rc = sqlite3_open(DB_NAME, &db);
        if (rc) {
            result << "Can't open database: " << sqlite3_errmsg(db);
            return result.str();
        }
        
        const char* query = "SELECT * FROM users WHERE username = ?";
        
        rc = sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            result << "Failed to prepare statement: " << sqlite3_errmsg(db);
            sqlite3_close(db);
            return result.str();
        }
        
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            result << "User found:\\n";
            result << "ID: " << sqlite3_column_int(stmt, 0) << "\\n";
            result << "Username: " << sqlite3_column_text(stmt, 1) << "\\n";
            result << "Email: " << sqlite3_column_text(stmt, 2) << "\\n";
            result << "Full Name: " << sqlite3_column_text(stmt, 3) << "\\n";
        } else {
            result << "User not found: " << username;
        }
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        return result.str();
    }
};

const char* Task37::DB_NAME = "users.db";

int main() {
    Task37::initializeDatabase();
    
    std::cout << "Test Case 1:" << std::endl;
    std::cout << Task37::getUserInfo("john_doe") << std::endl;
    std::cout << "\\n" << std::string(50, '=') << "\\n" << std::endl;
    
    std::cout << "Test Case 2:" << std::endl;
    std::cout << Task37::getUserInfo("jane_smith") << std::endl;
    std::cout << "\\n" << std::string(50, '=') << "\\n" << std::endl;
    
    std::cout << "Test Case 3:" << std::endl;
    std::cout << Task37::getUserInfo("alice_brown") << std::endl;
    std::cout << "\\n" << std::string(50, '=') << "\\n" << std::endl;
    
    std::cout << "Test Case 4:" << std::endl;
    std::cout << Task37::getUserInfo("nonexistent_user") << std::endl;
    std::cout << "\\n" << std::string(50, '=') << "\\n" << std::endl;
    
    std::cout << "Test Case 5:" << std::endl;
    std::cout << Task37::getUserInfo("charlie_davis") << std::endl;
    
    return 0;
}
