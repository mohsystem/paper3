#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstdlib>
#include <memory>
#include <sqlite3.h>

// User data structure
struct UserInfo {
    int id;
    std::string fullName;
    std::string email;
};

// Custom deleter for sqlite3 to be used with unique_ptr for RAII
struct SQLiteDBDeleter {
    void operator()(sqlite3* db) const {
        if (db) {
            sqlite3_close(db);
        }
    }
};

// Custom deleter for sqlite3_stmt
struct SQLiteStmtDeleter {
    void operator()(sqlite3_stmt* stmt) const {
        if (stmt) {
            sqlite3_finalize(stmt);
        }
    }
};

using unique_sqlite_db_ptr = std::unique_ptr<sqlite3, SQLiteDBDeleter>;
using unique_sqlite_stmt_ptr = std::unique_ptr<sqlite3_stmt, SQLiteStmtDeleter>;

const size_t MAX_USERNAME_LENGTH = 64;

// Function to set up the database with a users table and sample data
void setup_database(sqlite3* db) {
    char* err_msg = nullptr;
    const char* sql = 
        "DROP TABLE IF EXISTS users;"
        "CREATE TABLE users(id INTEGER PRIMARY KEY, username TEXT NOT NULL UNIQUE, full_name TEXT, email TEXT);"
        "INSERT INTO users (username, full_name, email) VALUES ('alice', 'Alice Smith', 'alice@example.com');"
        "INSERT INTO users (username, full_name, email) VALUES ('bob', 'Bob Johnson', 'bob@example.com');";

    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        std::string errMsgStr = "SQL error: ";
        if(err_msg) {
            errMsgStr += err_msg;
            sqlite3_free(err_msg);
        }
        throw std::runtime_error(errMsgStr);
    }
}

// Function to retrieve user information based on username
std::vector<UserInfo> get_user_info(sqlite3* db, const std::string& username) {
    // Rule #1: Validate input length
    if (username.empty() || username.length() > MAX_USERNAME_LENGTH) {
        std::cerr << "Error: Invalid username provided." << std::endl;
        return {};
    }

    // Rule #7: Use constant format strings
    const char* sql = "SELECT id, full_name, email FROM users WHERE username = ?;";
    sqlite3_stmt* stmt_raw = nullptr;

    // Use prepared statements to prevent SQL injection (CWE-89)
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt_raw, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
    }
    unique_sqlite_stmt_ptr stmt(stmt_raw);

    // Bind the username parameter
    rc = sqlite3_bind_text(stmt.get(), 1, username.c_str(), -1, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to bind parameter: " + std::string(sqlite3_errmsg(db)));
    }

    std::vector<UserInfo> results;
    while ((rc = sqlite3_step(stmt.get())) == SQLITE_ROW) {
        UserInfo user;
        user.id = sqlite3_column_int(stmt.get(), 0);
        
        const unsigned char* fullName_raw = sqlite3_column_text(stmt.get(), 1);
        user.fullName = fullName_raw ? reinterpret_cast<const char*>(fullName_raw) : "";
        
        const unsigned char* email_raw = sqlite3_column_text(stmt.get(), 2);
        user.email = email_raw ? reinterpret_cast<const char*>(email_raw) : "";
        
        results.push_back(user);
    }

    if (rc != SQLITE_DONE) {
        throw std::runtime_error("Failed to step statement: " + std::string(sqlite3_errmsg(db)));
    }

    return results;
}

void run_test_case(sqlite3* db, const std::string& username) {
    std::cout << "--- Testing with username: \"" << username << "\" ---" << std::endl;
    try {
        std::vector<UserInfo> users = get_user_info(db, username);
        if (users.empty()) {
            std::cout << "No user found." << std::endl;
        } else {
            for (const auto& user : users) {
                std::cout << "Found user -> ID: " << user.id
                          << ", Name: " << user.fullName
                          << ", Email: " << user.email << std::endl;
            }
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
    }
    std::cout << std::endl;
}

int main() {
    // Rule #6: Avoid hardcoding credentials. Using an in-memory database for this example.
    // For a file-based DB, use environment variables:
    // const char* db_path = std::getenv("DB_PATH");
    // if (!db_path) { db_path = "users.db"; }
    const char* db_path = ":memory:";
    
    sqlite3* db_raw = nullptr;
    int rc = sqlite3_open(db_path, &db_raw);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db_raw) << std::endl;
        sqlite3_close(db_raw);
        return 1;
    }
    unique_sqlite_db_ptr db(db_raw);

    try {
        setup_database(db.get());
    } catch (const std::runtime_error& e) {
        std::cerr << "Database setup failed: " << e.what() << std::endl;
        return 1;
    }

    // --- 5 Test Cases ---

    // 1. Valid user who exists
    run_test_case(db.get(), "alice");

    // 2. Valid user who does not exist
    run_test_case(db.get(), "charlie");

    // 3. Potential SQL injection attempt
    run_test_case(db.get(), "admin' OR 1=1; --");

    // 4. Empty username (should be rejected by validation)
    run_test_case(db.get(), "");

    // 5. Username longer than max length (should be rejected by validation)
    std::string long_username(MAX_USERNAME_LENGTH + 1, 'a');
    run_test_case(db.get(), long_username);

    return 0;
}