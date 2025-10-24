// Note: To compile and run this code, you need to have the SQLite3 development library installed.
// Example compile command:
// g++ task37.cpp -o task37 -lsqlite3
#include <iostream>
#include <string>
#include <vector>
#include <cstdio> // For remove()
#include <sqlite3.h>

const char* DB_FILE = "user_database_cpp.db";

/**
 * A callback function for sqlite3_exec, used during setup.
 */
static int callback(void* NotUsed, int argc, char** argv, char** azColName) {
    return 0;
}

/**
 * Sets up the database by creating a table and inserting sample data.
 */
void setupDatabase(sqlite3* db) {
    char* zErrMsg = 0;
    const char* sql = "DROP TABLE IF EXISTS users;"
                      "CREATE TABLE users ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "username TEXT NOT NULL UNIQUE,"
                      "full_name TEXT NOT NULL,"
                      "email TEXT NOT NULL);"
                      "INSERT INTO users (username, full_name, email) VALUES "
                      "('alice', 'Alice Smith', 'alice@example.com'),"
                      "('bob', 'Bob Johnson', 'bob@example.com'),"
                      "('admin', 'Administrator', 'admin@example.org');";

    if (sqlite3_exec(db, sql, callback, 0, &zErrMsg) != SQLITE_OK) {
        std::cerr << "SQL error during setup: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
    } else {
        std::cout << "Database setup successful." << std::endl;
    }
}

/**
 * Retrieves user information using a secure prepared statement.
 */
void getUserInfo(sqlite3* db, const std::string& username) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT id, username, full_name, email FROM users WHERE username = ?;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    if (sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC) != SQLITE_OK) {
        std::cerr << "Failed to bind parameter: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return;
    }

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        std::cout << "Result: User found:" << std::endl;
        std::cout << "  ID: " << sqlite3_column_int(stmt, 0) << std::endl;
        std::cout << "  Username: " << sqlite3_column_text(stmt, 1) << std::endl;
        std::cout << "  Full Name: " << sqlite3_column_text(stmt, 2) << std::endl;
        std::cout << "  Email: " << sqlite3_column_text(stmt, 3) << std::endl;
    } else if (rc == SQLITE_DONE) {
        std::cout << "Result: User not found." << std::endl;
    } else {
        std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
}

int main() {
    sqlite3* db;
    
    if (sqlite3_open(DB_FILE, &db)) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    }

    setupDatabase(db);
    
    std::cout << "\n--- Running 5 Test Cases ---" << std::endl;

    std::vector<std::string> testUsernames = {
        "alice",
        "bob",
        "charlie",
        "admin",
        "' OR '1'='1"
    };

    for (size_t i = 0; i < testUsernames.size(); ++i) {
        std::cout << "\n[Test Case " << (i + 1) << "]" << std::endl;
        std::cout << "Searching for user: '" << testUsernames[i] << "'" << std::endl;
        getUserInfo(db, testUsernames[i]);
    }

    sqlite3_close(db);
    remove(DB_FILE);

    return 0;
}