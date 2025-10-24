
#include <iostream>
#include <string>
#include <sqlite3.h>
#include <cstring>

class Task37 {
private:
    static const char* DB_PATH;
    
public:
    // Retrieve user information securely using prepared statements
    static std::string getUserInfo(const std::string& username) {
        if (username.empty()) {
            return "Invalid username";
        }
        
        std::string result;
        sqlite3* db;
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_open(DB_PATH, &db);
        if (rc != SQLITE_OK) {
            result = "Cannot open database: " + std::string(sqlite3_errmsg(db));
            sqlite3_close(db);
            return result;
        }
        
        // Use parameterized query to prevent SQL injection
        const char* query = "SELECT id, username, email, created_date FROM users WHERE username = ?";
        
        rc = sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            result = "Failed to prepare statement: " + std::string(sqlite3_errmsg(db));
            sqlite3_close(db);
            return result;
        }
        
        // Bind parameter
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            result += "User ID: " + std::to_string(sqlite3_column_int(stmt, 0)) + "\\n";
            result += "Username: " + std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))) + "\\n";
            result += "Email: " + std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2))) + "\\n";
            result += "Created Date: " + std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));
        } else if (rc == SQLITE_DONE) {
            result = "User not found";
        } else {
            result = "Query execution error: " + std::string(sqlite3_errmsg(db));
        }
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        return result;
    }
    
    // Initialize database with test data
    static void initializeDatabase() {
        sqlite3* db;
        char* errMsg = nullptr;
        
        int rc = sqlite3_open(DB_PATH, &db);
        if (rc != SQLITE_OK) {
            std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
            return;
        }
        
        const char* createTableSQL = 
            "CREATE TABLE IF NOT EXISTS users ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "username TEXT NOT NULL UNIQUE, "
            "email TEXT NOT NULL, "
            "created_date TEXT NOT NULL)";
        
        rc = sqlite3_exec(db, createTableSQL, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
            sqlite3_close(db);
            return;
        }
        
        // Insert test data
        const char* testData[][3] = {
            {"john_doe", "john@example.com", "2024-01-15"},
            {"jane_smith", "jane@example.com", "2024-02-20"},
            {"bob_wilson", "bob@example.com", "2024-03-10"},
            {"alice_jones", "alice@example.com", "2024-04-05"},
            {"charlie_brown", "charlie@example.com", "2024-05-12"}
        };
        
        const char* insertSQL = "INSERT OR IGNORE INTO users (username, email, created_date) VALUES (?, ?, ?)";
        sqlite3_stmt* stmt;
        
        for (int i = 0; i < 5; i++) {
            sqlite3_prepare_v2(db, insertSQL, -1, &stmt, nullptr);
            sqlite3_bind_text(stmt, 1, testData[i][0], -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, testData[i][1], -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, testData[i][2], -1, SQLITE_TRANSIENT);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
        
        sqlite3_close(db);
    }
};

const char* Task37::DB_PATH = "users.db";

int main() {
    // Initialize database with test data
    Task37::initializeDatabase();
    
    std::cout << "=== Secure Database Query - Test Cases ===" << std::endl << std::endl;
    
    // Test Case 1: Valid username
    std::cout << "Test Case 1: Valid username 'john_doe'" << std::endl;
    std::cout << Task37::getUserInfo("john_doe") << std::endl;
    std::cout << std::endl << std::string(50, '=') << std::endl << std::endl;
    
    // Test Case 2: Another valid username
    std::cout << "Test Case 2: Valid username 'jane_smith'" << std::endl;
    std::cout << Task37::getUserInfo("jane_smith") << std::endl;
    std::cout << std::endl << std::string(50, '=') << std::endl << std::endl;
    
    // Test Case 3: Non-existent username
    std::cout << "Test Case 3: Non-existent username 'unknown_user'" << std::endl;
    std::cout << Task37::getUserInfo("unknown_user") << std::endl;
    std::cout << std::endl << std::string(50, '=') << std::endl << std::endl;
    
    // Test Case 4: SQL injection attempt (secured by prepared statement)
    std::cout << "Test Case 4: SQL injection attempt" << std::endl;
    std::cout << Task37::getUserInfo("' OR '1'='1") << std::endl;
    std::cout << std::endl << std::string(50, '=') << std::endl << std::endl;
    
    // Test Case 5: Empty username
    std::cout << "Test Case 5: Empty username" << std::endl;
    std::cout << Task37::getUserInfo("") << std::endl;
    std::cout << std::endl << std::string(50, '=') << std::endl << std::endl;
    
    return 0;
}
