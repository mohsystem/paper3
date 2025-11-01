
#include <iostream>
#include <string>
#include <regex>
#include <sqlite3.h>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <cctype>

// Maximum email length to prevent DOS
const size_t MAX_EMAIL_LENGTH = 254;

// Email validation pattern - RFC 5322 simplified
const std::regex EMAIL_PATTERN(
    "^[a-zA-Z0-9_+&*-]+(?:\\\\.[a-zA-Z0-9_+&*-]+)*@(?:[a-zA-Z0-9-]+\\\\.)+[a-zA-Z]{2,7}$"
);

/**
 * Converts string to lowercase
 */
std::string toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

/**
 * Trims whitespace from both ends of string
 */
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \\t\\n\\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \\t\\n\\r");
    return str.substr(first, (last - first + 1));
}

/**
 * Validates email format and length
 */
bool isValidEmail(const std::string& email) {
    if (email.empty() || email.length() > MAX_EMAIL_LENGTH) {
        return false;
    }
    return std::regex_match(email, EMAIL_PATTERN);
}

/**
 * Initializes database with subscribers table
 */
void initializeDatabase(const char* dbPath) {
    sqlite3* db = nullptr;
    char* errMsg = nullptr;
    
    // Open database connection
    int rc = sqlite3_open(dbPath, &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Database initialization failed" << std::endl;
        if (db) sqlite3_close(db);
        return;
    }
    
    // Create table if not exists
    const char* createTableSQL = 
        "CREATE TABLE IF NOT EXISTS subscribers ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "email TEXT UNIQUE NOT NULL)";
    
    rc = sqlite3_exec(db, createTableSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Database initialization failed" << std::endl;
        sqlite3_free(errMsg);
    }
    
    sqlite3_close(db);
}

/**
 * Handles unsubscribe request
 */
std::string handleUnsubscribe(std::string email) {
    // Input validation - check for empty
    if (email.empty()) {
        return "Invalid request: email parameter is required";
    }
    
    // Trim and normalize input
    email = trim(email);
    email = toLower(email);
    
    // Validate email format and length
    if (!isValidEmail(email)) {
        return "Invalid email format";
    }
    
    // Get database path from environment or use default
    const char* dbPath = std::getenv("DB_PATH");
    if (!dbPath) {
        dbPath = "subscribers.db";
    }
    
    sqlite3* db = nullptr;
    sqlite3_stmt* stmt = nullptr;
    
    // Open database connection
    int rc = sqlite3_open(dbPath, &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Database error occurred" << std::endl;
        if (db) sqlite3_close(db);
        return "An error occurred processing your request";
    }
    
    // Check if email exists - using parameterized query to prevent SQL injection
    const char* checkSQL = "SELECT COUNT(*) FROM subscribers WHERE email = ?";
    rc = sqlite3_prepare_v2(db, checkSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Database error occurred" << std::endl;
        sqlite3_close(db);
        return "An error occurred processing your request";
    }
    
    // Bind email parameter
    sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_TRANSIENT);
    
    std::string result;
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        int count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        stmt = nullptr;
        
        if (count > 0) {
            // Email exists, proceed with deletion using parameterized query
            const char* deleteSQL = "DELETE FROM subscribers WHERE email = ?";
            rc = sqlite3_prepare_v2(db, deleteSQL, -1, &stmt, nullptr);
            if (rc != SQLITE_OK) {
                std::cerr << "Database error occurred" << std::endl;
                sqlite3_close(db);
                return "An error occurred processing your request";
            }
            
            sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_TRANSIENT);
            rc = sqlite3_step(stmt);
            
            if (rc == SQLITE_DONE && sqlite3_changes(db) > 0) {
                result = "Successfully unsubscribed";
            } else {
                result = "Unsubscribe failed";
            }
        } else {
            // Email not found in database
            result = "Email address is not subscribed";
        }
    } else {
        std::cerr << "Database error occurred" << std::endl;
        result = "An error occurred processing your request";
    }
    
    // Cleanup
    if (stmt) sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return result;
}

int main() {
    // Initialize database
    const char* dbPath = std::getenv("DB_PATH");
    if (!dbPath) {
        dbPath = "subscribers.db";
    }
    initializeDatabase(dbPath);
    
    // Test cases
    std::cout << "Test Case 1 - Valid email unsubscribe:" << std::endl;
    std::cout << handleUnsubscribe("test@example.com") << std::endl;
    
    std::cout << "\\nTest Case 2 - Email not in database:" << std::endl;
    std::cout << handleUnsubscribe("notfound@example.com") << std::endl;
    
    std::cout << "\\nTest Case 3 - Invalid email format:" << std::endl;
    std::cout << handleUnsubscribe("invalid-email") << std::endl;
    
    std::cout << "\\nTest Case 4 - Empty email:" << std::endl;
    std::cout << handleUnsubscribe("") << std::endl;
    
    std::cout << "\\nTest Case 5 - Email with whitespace:" << std::endl;
    std::cout << handleUnsubscribe("  test2@example.com  ") << std::endl;
    
    return 0;
}
