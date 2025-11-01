
#include <iostream>
#include <string>
#include <regex>
#include <sqlite3.h>

class Lse142 {
private:
    static const std::regex EMAIL_REGEX;
    
public:
    static std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \\t\\n\\r");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \\t\\n\\r");
        return str.substr(first, (last - first + 1));
    }
    
    static bool isValidEmail(const std::string& email) {
        return std::regex_match(email, EMAIL_REGEX);
    }
    
    static std::string unsubscribe(const std::string& email, sqlite3* db) {
        // Validate email
        if (email.empty()) {
            return "Error: Email address cannot be empty.";
        }
        
        std::string trimmedEmail = trim(email);
        
        if (!isValidEmail(trimmedEmail)) {
            return "Error: Invalid email address format.";
        }
        
        sqlite3_stmt* stmt;
        int rc;
        
        // Check if email exists
        const char* checkQuery = "SELECT COUNT(*) FROM subscribers WHERE email = ?";
        rc = sqlite3_prepare_v2(db, checkQuery, -1, &stmt, nullptr);
        
        if (rc != SQLITE_OK) {
            return "Error: Database operation failed.";
        }
        
        sqlite3_bind_text(stmt, 1, trimmedEmail.c_str(), -1, SQLITE_TRANSIENT);
        rc = sqlite3_step(stmt);
        
        int count = 0;
        if (rc == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
        
        if (count > 0) {
            // Email exists, delete it
            const char* deleteQuery = "DELETE FROM subscribers WHERE email = ?";
            rc = sqlite3_prepare_v2(db, deleteQuery, -1, &stmt, nullptr);
            
            if (rc != SQLITE_OK) {
                return "Error: Database operation failed.";
            }
            
            sqlite3_bind_text(stmt, 1, trimmedEmail.c_str(), -1, SQLITE_TRANSIENT);
            rc = sqlite3_step(stmt);
            sqlite3_finalize(stmt);
            
            if (rc == SQLITE_DONE) {
                return "Success: Email address has been unsubscribed.";
            }
        } else {
            return "Info: Email address not found in database.";
        }
        
        return "Error: Unsubscribe operation failed.";
    }
};

const std::regex Lse142::EMAIL_REGEX("^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$");

int main() {
    sqlite3* db;
    
    // Setup in-memory database
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
        std::cerr << "Cannot open database" << std::endl;
        return 1;
    }
    
    // Create table and insert test data
    char* errMsg;
    sqlite3_exec(db, "CREATE TABLE subscribers (email TEXT PRIMARY KEY)", nullptr, nullptr, &errMsg);
    sqlite3_exec(db, "INSERT INTO subscribers VALUES ('test1@example.com')", nullptr, nullptr, &errMsg);
    sqlite3_exec(db, "INSERT INTO subscribers VALUES ('test2@example.com')", nullptr, nullptr, &errMsg);
    sqlite3_exec(db, "INSERT INTO subscribers VALUES ('user@domain.com')", nullptr, nullptr, &errMsg);
    
    // Test Case 1: Valid email that exists
    std::cout << "Test 1: " << Lse142::unsubscribe("test1@example.com", db) << std::endl;
    
    // Test Case 2: Valid email that doesn't exist
    std::cout << "Test 2: " << Lse142::unsubscribe("nonexistent@example.com", db) << std::endl;
    
    // Test Case 3: Invalid email format
    std::cout << "Test 3: " << Lse142::unsubscribe("invalid-email", db) << std::endl;
    
    // Test Case 4: Empty email
    std::cout << "Test 4: " << Lse142::unsubscribe("", db) << std::endl;
    
    // Test Case 5: Valid email with whitespace
    std::cout << "Test 5: " << Lse142::unsubscribe("  test2@example.com  ", db) << std::endl;
    
    sqlite3_close(db);
    return 0;
}
