
#include <iostream>
#include <string>
#include <sqlite3.h>
#include <regex>
#include <algorithm>
#include <cctype>

class Lse143 {
private:
    sqlite3* db;
    std::string db_path;
    
    void initDatabase() {
        int rc = sqlite3_open(db_path.c_str(), &db);
        if (rc) {
            std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
            return;
        }
        
        const char* sql = "CREATE TABLE IF NOT EXISTS subscribers ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                         "email TEXT UNIQUE NOT NULL)";
        
        char* errMsg = nullptr;
        rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        }
        sqlite3_close(db);
    }
    
    static bool isValidEmail(const std::string& email) {
        if (email.empty()) return false;
        
        std::regex pattern(R"(^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$)");
        return std::regex_match(email, pattern);
    }
    
    static std::string toLower(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(),
                      [](unsigned char c) { return std::tolower(c); });
        return str;
    }
    
    static std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \\t\\n\\r");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \\t\\n\\r");
        return str.substr(first, last - first + 1);
    }
    
public:
    Lse143(const std::string& dbPath = "users.db") : db(nullptr), db_path(dbPath) {
        initDatabase();
    }
    
    std::string unsubscribeUser(const std::string& email) {
        // Input validation
        if (email.empty()) {
            return "Invalid email parameter";
        }
        
        // Sanitize email input
        std::string cleanEmail = toLower(trim(email));
        
        // Validate email format
        if (!isValidEmail(cleanEmail)) {
            return "Invalid email format";
        }
        
        int rc = sqlite3_open(db_path.c_str(), &db);
        if (rc) {
            return "Database connection error";
        }
        
        // Check if email exists using parameterized query
        sqlite3_stmt* stmt;
        const char* checkSql = "SELECT COUNT(*) FROM subscribers WHERE email = ?";
        rc = sqlite3_prepare_v2(db, checkSql, -1, &stmt, nullptr);
        
        if (rc != SQLITE_OK) {
            sqlite3_close(db);
            return "Database error";
        }
        
        sqlite3_bind_text(stmt, 1, cleanEmail.c_str(), -1, SQLITE_TRANSIENT);
        rc = sqlite3_step(stmt);
        
        int count = 0;
        if (rc == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
        
        std::string result;
        if (count > 0) {
            // Email exists, delete it
            const char* deleteSql = "DELETE FROM subscribers WHERE email = ?";
            rc = sqlite3_prepare_v2(db, deleteSql, -1, &stmt, nullptr);
            
            if (rc == SQLITE_OK) {
                sqlite3_bind_text(stmt, 1, cleanEmail.c_str(), -1, SQLITE_TRANSIENT);
                rc = sqlite3_step(stmt);
                
                if (rc == SQLITE_DONE) {
                    result = "Successfully unsubscribed: " + cleanEmail;
                } else {
                    result = "Failed to unsubscribe";
                }
                sqlite3_finalize(stmt);
            } else {
                result = "Database error";
            }
        } else {
            result = "Email not subscribed: " + cleanEmail;
        }
        
        sqlite3_close(db);
        return result;
    }
    
    void addSubscriber(const std::string& email) {
        int rc = sqlite3_open(db_path.c_str(), &db);
        if (rc) return;
        
        std::string cleanEmail = toLower(trim(email));
        sqlite3_stmt* stmt;
        const char* sql = "INSERT OR IGNORE INTO subscribers (email) VALUES (?)";
        
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        if (rc == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, cleanEmail.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
        sqlite3_close(db);
    }
};

int main() {
    Lse143 service;
    
    // Test cases
    std::cout << "=== Test Case 1: Unsubscribe existing user ===" << std::endl;
    service.addSubscriber("user1@example.com");
    std::cout << service.unsubscribeUser("user1@example.com") << std::endl;
    
    std::cout << "\\n=== Test Case 2: Unsubscribe non-existing user ===" << std::endl;
    std::cout << service.unsubscribeUser("nonexistent@example.com") << std::endl;
    
    std::cout << "\\n=== Test Case 3: Invalid email format ===" << std::endl;
    std::cout << service.unsubscribeUser("invalid-email") << std::endl;
    
    std::cout << "\\n=== Test Case 4: Empty email ===" << std::endl;
    std::cout << service.unsubscribeUser("") << std::endl;
    
    std::cout << "\\n=== Test Case 5: Whitespace email ===" << std::endl;
    std::cout << service.unsubscribeUser("   ") << std::endl;
    
    return 0;
}
