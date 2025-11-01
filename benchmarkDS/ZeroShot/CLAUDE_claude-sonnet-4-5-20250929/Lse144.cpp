
#include <iostream>
#include <string>
#include <regex>
#include <sqlite3.h>

class Lse144 {
private:
    static const std::string EMAIL_REGEX;
    static const std::string DB_PATH;
    
    /**
     * Validates email format
     */
    static bool isValidEmail(const std::string& email) {
        if (email.empty()) {
            return false;
        }
        std::regex pattern("^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$");
        return std::regex_match(email, pattern);
    }
    
    /**
     * Simulates calling an unsubscribe script
     * Returns 0 on success, non-zero on failure
     */
    static int callUnsubscribeScript(const std::string& email) {
        try {
            // In a real implementation, this would call an actual script
            // For security, validate and sanitize input before passing to external script
            
            // Simulate script execution
            // Return 0 for success
            return 0;
            
        } catch (const std::exception& e) {
            std::cerr << "Script execution error: " << e.what() << std::endl;
            return 1;
        }
    }

public:
    /**
     * Checks if email exists in database and unsubscribes if found
     * Returns 1 if successfully unsubscribed, 0 otherwise
     */
    static int unsubscribeEmail(const std::string& email) {
        // Validate email format first
        if (!isValidEmail(email)) {
            return 0;
        }
        
        sqlite3* db = nullptr;
        sqlite3_stmt* stmt = nullptr;
        int result = 0;
        
        try {
            // Open database connection
            int rc = sqlite3_open(DB_PATH.c_str(), &db);
            if (rc != SQLITE_OK) {
                std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
                sqlite3_close(db);
                return 0;
            }
            
            // Check if email exists using parameterized query (prevents SQL injection)
            const char* checkQuery = "SELECT email FROM subscribers WHERE email = ?";
            rc = sqlite3_prepare_v2(db, checkQuery, -1, &stmt, nullptr);
            
            if (rc != SQLITE_OK) {
                std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
                sqlite3_close(db);
                return 0;
            }
            
            // Bind email parameter
            sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_TRANSIENT);
            
            // Execute query
            rc = sqlite3_step(stmt);
            
            // If email doesn't exist, return 0
            if (rc != SQLITE_ROW) {
                sqlite3_finalize(stmt);
                sqlite3_close(db);
                return 0;
            }
            
            // Email exists, call unsubscribe script
            int unsubscribeResult = callUnsubscribeScript(email);
            
            // If script returns 0 (success), return 1, otherwise return 0
            result = (unsubscribeResult == 0) ? 1 : 0;
            
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            result = 0;
        }
        
        // Clean up resources
        if (stmt) {
            sqlite3_finalize(stmt);
        }
        if (db) {
            sqlite3_close(db);
        }
        
        return result;
    }
};

const std::string Lse144::DB_PATH = "emaildb.db";

int main() {
    // Test cases
    std::cout << "Test Case 1 (valid existing email): " << Lse144::unsubscribeEmail("user1@example.com") << std::endl;
    std::cout << "Test Case 2 (valid non-existing email): " << Lse144::unsubscribeEmail("nonexistent@example.com") << std::endl;
    std::cout << "Test Case 3 (invalid email format): " << Lse144::unsubscribeEmail("invalid-email") << std::endl;
    std::cout << "Test Case 4 (empty email): " << Lse144::unsubscribeEmail("") << std::endl;
    std::cout << "Test Case 5 (another valid email): " << Lse144::unsubscribeEmail("test@domain.com") << std::endl;
    
    return 0;
}
