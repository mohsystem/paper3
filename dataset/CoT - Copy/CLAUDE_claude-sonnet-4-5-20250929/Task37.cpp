
#include <iostream>
#include <string>
#include <sqlite3.h>
#include <regex>
#include <memory>

class Task37 {
private:
    std::string db_path;
    
    bool validateUsername(const std::string& username) {
        // Check if username is empty or too long
        if (username.empty() || username.length() > 50) {
            return false;
        }
        
        // Check if username contains only alphanumeric and underscore
        std::regex pattern("^[a-zA-Z0-9_]+$");
        return std::regex_match(username, pattern);
    }
    
public:
    struct UserInfo {
        int userId;
        std::string username;
        std::string email;
        std::string createdAt;
        bool found;
        
        UserInfo() : userId(0), found(false) {}
    };
    
    Task37(const std::string& path = "users.db") : db_path(path) {}
    
    UserInfo getUserByUsername(const std::string& username) {
        UserInfo userInfo;
        
        // Input validation
        if (!validateUsername(username)) {
            std::cerr << "Invalid username provided" << std::endl;
            return userInfo;
        }
        
        sqlite3* db = nullptr;
        sqlite3_stmt* stmt = nullptr;
        
        try {
            // Open database connection
            int rc = sqlite3_open(db_path.c_str(), &db);
            if (rc != SQLITE_OK) {
                std::cerr << "Cannot open database" << std::endl;
                return userInfo;
            }
            
            // Prepare parameterized query to prevent SQL injection
            const char* sql = "SELECT user_id, username, email, created_at FROM users WHERE username = ?";
            rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
            
            if (rc != SQLITE_OK) {
                std::cerr << "Failed to prepare statement" << std::endl;
                sqlite3_close(db);
                return userInfo;
            }
            
            // Bind parameter safely
            sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
            
            // Execute query
            rc = sqlite3_step(stmt);
            if (rc == SQLITE_ROW) {
                userInfo.userId = sqlite3_column_int(stmt, 0);
                userInfo.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                userInfo.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                userInfo.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
                userInfo.found = true;
            }
            
        } catch (const std::exception& e) {
            std::cerr << "Database error occurred" << std::endl;
        }
        
        // Clean up resources
        if (stmt) {
            sqlite3_finalize(stmt);
        }
        if (db) {
            sqlite3_close(db);
        }
        
        return userInfo;
    }
};

int main() {
    // Setup test database
    sqlite3* db;
    sqlite3_open("test_users.db", &db);
    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS users (user_id INTEGER PRIMARY KEY, username TEXT UNIQUE, email TEXT, created_at TEXT)", nullptr, nullptr, nullptr);
    
    const char* insert_sql = "INSERT OR IGNORE INTO users VALUES (?, ?, ?, ?)";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, insert_sql, -1, &stmt, nullptr);
    
    // Insert test data
    std::string test_data[][4] = {
        {"1", "john_doe", "john@example.com", "2024-01-01"},
        {"2", "alice", "alice@example.com", "2024-01-02"},
        {"3", "bob123", "bob@example.com", "2024-01-03"}
    };
    
    for (auto& data : test_data) {
        sqlite3_reset(stmt);
        sqlite3_bind_int(stmt, 1, std::stoi(data[0]));
        sqlite3_bind_text(stmt, 2, data[1].c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, data[2].c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, data[3].c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    // Test cases
    Task37 task("test_users.db");
    std::string testUsers[] = {"john_doe", "alice", "bob123", "admin", "test_user"};
    
    std::cout << "Testing User Retrieval:\\n" << std::endl;
    for (const auto& username : testUsers) {
        std::cout << "Searching for user: " << username << std::endl;
        Task37::UserInfo user = task.getUserByUsername(username);
        
        if (user.found) {
            std::cout << "Found: UserInfo{userId=" << user.userId 
                      << ", username='" << user.username 
                      << "', email='" << user.email 
                      << "', createdAt='" << user.createdAt << "'}" << std::endl;
        } else {
            std::cout << "User not found or error occurred" << std::endl;
        }
        std::cout << "---" << std::endl;
    }
    
    return 0;
}
