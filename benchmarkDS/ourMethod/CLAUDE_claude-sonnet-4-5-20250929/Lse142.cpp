
#include <iostream>
#include <string>
#include <regex>
#include <sqlite3.h>
#include <cstring>
#include <memory>

// Rules#2: Maximum email length per RFC 5321
constexpr size_t MAX_EMAIL_LENGTH = 254;

// Rules#2: Strict email validation pattern
const std::regex EMAIL_PATTERN(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$)");

/**
 * Unsubscribe function with security controls
 * Rules#2: Validates input length and format
 * Rules#32: Uses parameterized queries to prevent SQL injection
 * Rules#11: Returns generic error messages
 */
std::string unsubscribe(const std::string& email) {
    // Rules#2: Validate length to prevent buffer issues
    if (email.length() > MAX_EMAIL_LENGTH) {
        return "Invalid email address";
    }
    
    // Rules#2: Validate format using allow list pattern
    if (!std::regex_match(email, EMAIL_PATTERN)) {
        return "Invalid email address";
    }
    
    // Rules#3: Load database path from environment or use default
    const char* db_path_env = std::getenv("DB_PATH");
    std::string db_path = db_path_env ? db_path_env : "subscribers.db";
    
    sqlite3* db = nullptr;
    sqlite3_stmt* check_stmt = nullptr;
    sqlite3_stmt* delete_stmt = nullptr;
    
    // Open database connection
    int rc = sqlite3_open(db_path.c_str(), &db);
    if (rc != SQLITE_OK) {
        // Rules#11: Log detailed error, return generic message
        std::cerr << "Database error: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return "Service error occurred";
    }
    
    // Begin transaction
    sqlite3_exec(db, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);
    
    try {
        // Rules#32: Use parameterized query to prevent SQL injection
        const char* check_query = "SELECT COUNT(*) FROM subscribers WHERE email = ?";
        rc = sqlite3_prepare_v2(db, check_query, -1, &check_stmt, nullptr);
        
        if (rc != SQLITE_OK) {
            throw std::runtime_error(sqlite3_errmsg(db));
        }
        
        // Bind email parameter safely
        sqlite3_bind_text(check_stmt, 1, email.c_str(), -1, SQLITE_TRANSIENT);
        
        rc = sqlite3_step(check_stmt);
        if (rc == SQLITE_ROW) {
            int count = sqlite3_column_int(check_stmt, 0);
            
            if (count > 0) {
                // Email exists, remove it
                // Rules#32: Parameterized query prevents SQL injection
                const char* delete_query = "DELETE FROM subscribers WHERE email = ?";
                rc = sqlite3_prepare_v2(db, delete_query, -1, &delete_stmt, nullptr);
                
                if (rc != SQLITE_OK) {
                    throw std::runtime_error(sqlite3_errmsg(db));
                }
                
                sqlite3_bind_text(delete_stmt, 1, email.c_str(), -1, SQLITE_TRANSIENT);
                
                rc = sqlite3_step(delete_stmt);
                if (rc == SQLITE_DONE) {
                    sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr);
                    // Rules#10: Do not log user email
                    
                    sqlite3_finalize(check_stmt);
                    sqlite3_finalize(delete_stmt);
                    sqlite3_close(db);
                    
                    return "Successfully unsubscribed";
                } else {
                    throw std::runtime_error("Delete failed");
                }
            } else {
                // Email not in database
                sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
                
                sqlite3_finalize(check_stmt);
                sqlite3_close(db);
                
                return "Email address not found in database";
            }
        } else {
            throw std::runtime_error("Query failed");
        }
    } catch (const std::exception& e) {
        // Rules#11: Log detailed error, return generic message
        std::cerr << "Error: " << e.what() << std::endl;
        sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
        
        if (check_stmt) sqlite3_finalize(check_stmt);
        if (delete_stmt) sqlite3_finalize(delete_stmt);
        sqlite3_close(db);
        
        return "Service error occurred";
    }
}

int main() {
    // Test cases
    std::cout << "Test 1 - Valid email: " << unsubscribe("user@example.com") << std::endl;
    std::cout << "Test 2 - Invalid format: " << unsubscribe("invalid-email") << std::endl;
    std::cout << "Test 3 - Empty string: " << unsubscribe("") << std::endl;
    std::cout << "Test 4 - Too long: " << unsubscribe(std::string(300, 'a') + "@example.com") << std::endl;
    std::cout << "Test 5 - SQL injection attempt: " << unsubscribe("user@example.com' OR '1'='1") << std::endl;
    
    return 0;
}
