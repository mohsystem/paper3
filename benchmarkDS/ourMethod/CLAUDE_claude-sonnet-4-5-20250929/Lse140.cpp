
#include <iostream>
#include <string>
#include <cstring>
#include <sqlite3.h>
#include <cstdlib>

// Rules#2: Input validation function
bool validateUsername(const std::string& username) {
    // Check if empty
    if (username.empty()) {
        return false;
    }
    
    // Limit length to prevent DoS
    if (username.length() > 255) {
        return false;
    }
    
    return true;
}

// Check if username exists in database
// Returns true if exists, false otherwise
bool checkUsernameExists(const std::string& username) {
    // Rules#2: Validate input before processing
    if (!validateUsername(username)) {
        return false;
    }
    
    sqlite3* db = nullptr;
    sqlite3_stmt* stmt = nullptr;
    bool exists = false;
    
    // Rules#3: Load database path from environment, not hard coded
    const char* dbPath = std::getenv("DB_PATH");
    if (dbPath == nullptr) {
        dbPath = "database.db";
    }
    
    // Open database connection
    int rc = sqlite3_open(dbPath, &db);
    if (rc != SQLITE_OK) {
        // Rules#11: Don't leak internal details\n        std::cerr << "Database operation failed" << std::endl;\n        if (db != nullptr) {\n            sqlite3_close(db);\n        }\n        return false;\n    }\n    \n    // Rules#32: Use parameterized queries to prevent SQL injection\n    const char* sql = "SELECT COUNT(*) FROM users WHERE username = ?";\n    \n    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);\n    if (rc != SQLITE_OK) {\n        std::cerr << "Database operation failed" << std::endl;\n        sqlite3_close(db);\n        return false;\n    }\n    \n    // Bind username parameter safely\n    rc = sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);\n    if (rc != SQLITE_OK) {\n        std::cerr << "Database operation failed" << std::endl;\n        sqlite3_finalize(stmt);\n        sqlite3_close(db);\n        return false;\n    }\n    \n    // Execute query\n    rc = sqlite3_step(stmt);\n    if (rc == SQLITE_ROW) {\n        int count = sqlite3_column_int(stmt, 0);\n        exists = (count > 0);\n    }\n    \n    // Clean up resources\n    sqlite3_finalize(stmt);\n    sqlite3_close(db);\n    \n    return exists;\n}\n\nint main() {\n    // Test case 1: Valid username that exists\n    std::cout << "Test 1 (existing user): " << checkUsernameExists("alice") << std::endl;\n    \n    // Test case 2: Valid username that doesn't exist
    std::cout << "Test 2 (non-existing user): " << checkUsernameExists("nonexistent") << std::endl;
    
    // Test case 3: Empty username
    std::cout << "Test 3 (empty): " << checkUsernameExists("") << std::endl;
    
    // Test case 4: Very long username
    std::string longName(300, 'a');
    std::cout << "Test 4 (too long): " << checkUsernameExists(longName) << std::endl;
    
    // Test case 5: Username with special characters
    std::cout << "Test 5 (special chars): " << checkUsernameExists("user@test") << std::endl;
    
    return 0;
}
