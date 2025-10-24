
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <sqlite3.h>

// Security: Using prepared statements to prevent SQL injection (CWE-89)
// All user input is treated as untrusted and properly parameterized
class DatabaseQuery {
private:
    sqlite3* db;
    std::string dbPath;

    // Security: Clear sensitive data from memory
    void clearSensitiveData(char* data, size_t len) {
        if (data != nullptr) {
            volatile char* p = data;
            for (size_t i = 0; i < len; ++i) {
                p[i] = 0;
            }
        }
    }

public:
    DatabaseQuery(const std::string& path) : db(nullptr), dbPath(path) {}

    ~DatabaseQuery() {
        if (db != nullptr) {
            sqlite3_close(db);
            db = nullptr;
        }
    }

    // Security: Validate database path to prevent path traversal (CWE-22)
    bool connect() {
        // Security: Validate path length to prevent buffer issues
        if (dbPath.empty() || dbPath.length() > 4096) {
            std::cerr << "Invalid database path" << std::endl;
            return false;
        }

        // Security: Check for path traversal attempts
        if (dbPath.find("..") != std::string::npos) {
            std::cerr << "Path traversal detected" << std::endl;
            return false;
        }

        int rc = sqlite3_open(dbPath.c_str(), &db);
        if (rc != SQLITE_OK) {
            std::cerr << "Cannot open database" << std::endl;
            return false;
        }
        return true;
    }

    // Security: Use prepared statements to prevent SQL injection
    bool queryUserByUsername(const std::string& username) {
        if (db == nullptr) {
            std::cerr << "Database not connected" << std::endl;
            return false;
        }

        // Security: Validate username input
        if (username.empty() || username.length() > 255) {
            std::cerr << "Invalid username length" << std::endl;
            return false;
        }

        // Security: Check for null bytes in username (CWE-158)
        if (username.find('\\0') != std::string::npos) {
            std::cerr << "Null byte in username" << std::endl;
            return false;
        }

        sqlite3_stmt* stmt = nullptr;
        // Security: Using parameterized query to prevent SQL injection (CWE-89)
        const char* sql = "SELECT id, username, email FROM users WHERE username = ?";
        
        int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "Failed to prepare statement" << std::endl;
            return false;
        }

        // Security: Bind parameter safely
        rc = sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        if (rc != SQLITE_OK) {
            std::cerr << "Failed to bind parameter" << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }

        bool found = false;
        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
            found = true;
            int id = sqlite3_column_int(stmt, 0);
            const unsigned char* user = sqlite3_column_text(stmt, 1);
            const unsigned char* email = sqlite3_column_text(stmt, 2);

            // Security: Validate pointers before use
            if (user != nullptr && email != nullptr) {
                std::cout << "ID: " << id 
                         << ", Username: " << user 
                         << ", Email: " << email << std::endl;
            }
        }

        if (!found) {
            std::cout << "No user found with username: " << username << std::endl;
        }

        sqlite3_finalize(stmt);
        return rc == SQLITE_DONE || found;
    }

    // Security: Initialize test database with safe operations
    bool initTestDatabase() {
        if (db == nullptr) return false;

        const char* createTable = 
            "CREATE TABLE IF NOT EXISTS users ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "username TEXT NOT NULL UNIQUE, "
            "email TEXT NOT NULL)";
        
        char* errMsg = nullptr;
        int rc = sqlite3_exec(db, createTable, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << (errMsg ? errMsg : "unknown") << std::endl;
            if (errMsg) sqlite3_free(errMsg);
            return false;
        }

        // Security: Using prepared statements even for inserts
        const char* insertSql = "INSERT OR IGNORE INTO users (username, email) VALUES (?, ?)";
        sqlite3_stmt* stmt = nullptr;
        
        const char* testUsers[][2] = {
            {"alice", "alice@example.com"},
            {"bob", "bob@example.com"},
            {"charlie", "charlie@example.com"},
            {"david", "david@example.com"},
            {"eve", "eve@example.com"}
        };

        for (int i = 0; i < 5; ++i) {
            rc = sqlite3_prepare_v2(db, insertSql, -1, &stmt, nullptr);
            if (rc == SQLITE_OK) {
                sqlite3_bind_text(stmt, 1, testUsers[i][0], -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 2, testUsers[i][1], -1, SQLITE_STATIC);
                sqlite3_step(stmt);
                sqlite3_finalize(stmt);
            }
        }

        return true;
    }
};

int main() {
    // Security: Use a temporary in-memory database for testing
    DatabaseQuery dbQuery(":memory:");

    if (!dbQuery.connect()) {
        std::cerr << "Failed to connect to database" << std::endl;
        return 1;
    }

    if (!dbQuery.initTestDatabase()) {
        std::cerr << "Failed to initialize test database" << std::endl;
        return 1;
    }

    // Test cases with security validation
    std::cout << "=== Test Case 1: Valid username ===" << std::endl;
    dbQuery.queryUserByUsername("alice");

    std::cout << "\\n=== Test Case 2: Another valid username ===" << std::endl;
    dbQuery.queryUserByUsername("bob");

    std::cout << "\\n=== Test Case 3: Non-existent user ===" << std::endl;
    dbQuery.queryUserByUsername("nonexistent");

    std::cout << "\\n=== Test Case 4: SQL injection attempt (prevented) ===" << std::endl;
    // Security: This injection attempt is safely handled by prepared statements
    dbQuery.queryUserByUsername("alice' OR '1'='1");

    std::cout << "\\n=== Test Case 5: Empty username ===" << std::endl;
    dbQuery.queryUserByUsername("");

    return 0;
}
