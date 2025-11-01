#include <iostream>
#include <string>
#include <sqlite3.h>
#include <cassert>

// Note: This code requires two external libraries:
// 1. cpp-httplib: A header-only HTTP library. Download "httplib.h" from its GitHub repository.
//    https://github.com/yhirose/cpp-httplib
// 2. SQLite3: The database library. Install it via your system's package manager
//    (e.g., `sudo apt-get install libsqlite3-dev` on Debian/Ubuntu).
//
// Compilation command:
// g++ -o Lse138 Lse138.cpp -lsqlite3 -lpthread -std=c++17
#include "httplib.h"

const char* DB_NAME = "user_moderators.db";

// Sets up the database and populates it with initial data
void setupDatabase() {
    sqlite3* db;
    char* zErrMsg = 0;
    int rc = sqlite3_open(DB_NAME, &db);

    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    const char* sql_setup =
        "DROP TABLE IF EXISTS users;"
        "CREATE TABLE users("
        " id INTEGER PRIMARY KEY, "
        " username TEXT NOT NULL, "
        " is_moderator BOOLEAN NOT NULL CHECK (is_moderator IN (0, 1)));"
        "INSERT INTO users (id, username, is_moderator) VALUES (1, 'alice', 1);"
        "INSERT INTO users (id, username, is_moderator) VALUES (2, 'bob', 0);"
        "INSERT INTO users (id, username, is_moderator) VALUES (3, 'charlie', 0);";

    rc = sqlite3_exec(db, sql_setup, 0, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
    }
    sqlite3_close(db);
}

/**
 * Securely checks if a user is a moderator using a prepared statement.
 * @param userId The ID of the user to check.
 * @return true if the user is a moderator, false otherwise.
 */
bool isUserModerator(const std::string& userId) {
    // Input validation: ensure userId contains only digits
    if (userId.empty() || userId.find_first_not_of("0123456789") != std::string::npos) {
        std::cerr << "Invalid userId format: " << userId << std::endl;
        return false;
    }

    sqlite3* db;
    sqlite3_stmt* stmt;
    bool isModerator = false;

    if (sqlite3_open(DB_NAME, &db) != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    // The query uses a placeholder (?) to prevent SQL injection
    const char* sql = "SELECT is_moderator FROM users WHERE id = ?";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        // Bind the integer id to the placeholder
        sqlite3_bind_int(stmt, 1, std::stoi(userId));

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            isModerator = (sqlite3_column_int(stmt, 0) == 1);
        }
    } else {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
    }

    // Finalize the statement and close the database to free resources
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return isModerator;
}

void runTests() {
    std::cout << "--- Running 5 Test Cases ---" << std::endl;
    // Test Case 1: User is a moderator
    assert(isUserModerator("1") == true);
    std::cout << "Test 1 (User 1 is Moderator): Passed" << std::endl;
    // Test Case 2: User is not a moderator
    assert(isUserModerator("2") == false);
    std::cout << "Test 2 (User 2 is Not Moderator): Passed" << std::endl;
    // Test Case 3: User does not exist
    assert(isUserModerator("99") == false);
    std::cout << "Test 3 (User 99 does not exist): Passed" << std::endl;
    // Test Case 4: Invalid input (non-numeric)
    assert(isUserModerator("abc") == false);
    std::cout << "Test 4 (Invalid input 'abc'): Passed" << std::endl;
    // Test Case 5: Invalid input (SQL injection attempt)
    assert(isUserModerator("1 OR 1=1") == false);
    std::cout << "Test 5 (SQLi attempt '1 OR 1=1'): Passed" << std::endl;
    std::cout << "--- Test Cases Finished ---" << std::endl << std::endl;
}

int main() {
    // 1. Set up the database
    setupDatabase();

    // 2. Run test cases on the core logic
    runTests();
    
    // 3. Set up and start the web server
    httplib::Server svr;

    svr.Get("/is_moderator", [](const httplib::Request& req, httplib::Response& res) {
        std::string response_json;
        if (req.has_param("userId")) {
            std::string userId = req.get_param_value("userId");
            bool isMod = isUserModerator(userId);
            response_json = "{\"userId\": \"" + userId + "\", \"isModerator\": " + (isMod ? "true" : "false") + "}";
            res.set_content(response_json, "application/json");
        } else {
            response_json = "{\"error\": \"userId parameter is required\"}";
            res.status = 400; // Bad Request
            res.set_content(response_json, "application/json");
        }
    });

    std::cout << "Server started on http://localhost:8081" << std::endl;
    std::cout << "Try visiting http://localhost:8081/is_moderator?userId=1" << std::endl;
    svr.listen("localhost", 8081);

    return 0;
}