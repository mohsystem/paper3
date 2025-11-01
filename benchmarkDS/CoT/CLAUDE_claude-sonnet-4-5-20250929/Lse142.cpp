
#include <iostream>
#include <string>
#include <regex>
#include <sqlite3.h>

class Lse142 {
private:
    static const std::regex EMAIL_PATTERN;
    
    static bool isValidEmail(const std::string& email) {
        return std::regex_match(email, EMAIL_PATTERN);
    }
    
    static std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(' ');
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(' ');
        return str.substr(first, (last - first + 1));
    }
    
public:
    static std::string unsubscribe(const std::string& emailInput, 
                                   const std::string& dbPath = "newsletter.db") {
        // Input validation
        if (emailInput.empty()) {
            return "Error: Email address is required";
        }
        
        std::string email = trim(emailInput);
        
        // Validate email format
        if (!isValidEmail(email)) {
            return "Error: Invalid email format";
        }
        
        sqlite3* db = nullptr;
        sqlite3_stmt* checkStmt = nullptr;
        sqlite3_stmt* deleteStmt = nullptr;
        
        try {
            // Secure database connection
            if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
                sqlite3_close(db);
                return "Error: Database connection failed";
            }
            
            // Check if email exists using parameterized query
            const char* checkQuery = "SELECT email FROM subscribers WHERE email = ?";
            if (sqlite3_prepare_v2(db, checkQuery, -1, &checkStmt, nullptr) != SQLITE_OK) {
                sqlite3_finalize(checkStmt);
                sqlite3_close(db);
                return "Error: Database operation failed";
            }
            
            sqlite3_bind_text(checkStmt, 1, email.c_str(), -1, SQLITE_STATIC);
            int result = sqlite3_step(checkStmt);
            
            if (result == SQLITE_ROW) {
                // Email exists, remove it using parameterized query
                const char* deleteQuery = "DELETE FROM subscribers WHERE email = ?";
                if (sqlite3_prepare_v2(db, deleteQuery, -1, &deleteStmt, nullptr) != SQLITE_OK) {
                    sqlite3_finalize(checkStmt);
                    sqlite3_finalize(deleteStmt);
                    sqlite3_close(db);
                    return "Error: Database operation failed";
                }
                
                sqlite3_bind_text(deleteStmt, 1, email.c_str(), -1, SQLITE_STATIC);
                
                if (sqlite3_step(deleteStmt) == SQLITE_DONE) {
                    sqlite3_finalize(checkStmt);
                    sqlite3_finalize(deleteStmt);
                    sqlite3_close(db);
                    return "Success: Email address has been unsubscribed";
                }
            } else {
                sqlite3_finalize(checkStmt);
                sqlite3_close(db);
                return "Info: Email address is not in the database";
            }
            
            sqlite3_finalize(checkStmt);
            sqlite3_finalize(deleteStmt);
            sqlite3_close(db);
            return "Error: Failed to unsubscribe";
            
        } catch (...) {
            if (checkStmt) sqlite3_finalize(checkStmt);
            if (deleteStmt) sqlite3_finalize(deleteStmt);
            if (db) sqlite3_close(db);
            return "Error: Database operation failed";
        }
    }
};

const std::regex Lse142::EMAIL_PATTERN("^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$");

int main() {
    // Test cases
    std::cout << "Test 1: " << Lse142::unsubscribe("user@example.com") << std::endl;
    std::cout << "Test 2: " << Lse142::unsubscribe("test@domain.org") << std::endl;
    std::cout << "Test 3: " << Lse142::unsubscribe("invalid-email") << std::endl;
    std::cout << "Test 4: " << Lse142::unsubscribe("") << std::endl;
    std::cout << "Test 5: " << Lse142::unsubscribe("nonexistent@email.com") << std::endl;
    
    return 0;
}
