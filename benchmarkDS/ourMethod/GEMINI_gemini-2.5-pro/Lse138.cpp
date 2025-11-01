#include <iostream>
#include <string>
#include <stdexcept>
#include <sqlite3.h>
#include "crow.h" // Requires crow framework header-only library

/*
 * Note: This example requires the Crow C++ web framework and SQLite3.
 * 1. Download crow_all.h from the Crow github repository.
 * 2. Install sqlite3 library (e.g., `sudo apt-get install libsqlite3-dev` on Debian/Ubuntu).
 * 3. Compile with a command like:
 *    g++ -std=c++17 -o Lse138 Lse138.cpp -I/path/to/crow/include -lsqlite3 -lpthread -lboost_system
 *    (Boost is a dependency of Crow)
 */

const char* DB_NAME = "user_database.db";

void setupDatabase() {
    sqlite3* db;
    char* zErrMsg = 0;
    
    // Rule #7: In a real app, this path should come from a config file/env var.
    int rc = sqlite3_open(DB_NAME, &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        exit(1);
    }

    const char* dropSql = "DROP TABLE IF EXISTS users;";
    sqlite3_exec(db, dropSql, 0, 0, &zErrMsg);

    const char* sql = "CREATE TABLE IF NOT EXISTS users("
                      "id INTEGER PRIMARY KEY, "
                      "username TEXT NOT NULL, "
                      "is_moderator INTEGER NOT NULL);";
    rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
        sqlite3_close(db);
        exit(1);
    }
    
    const char* insertSql = "INSERT INTO users (id, username, is_moderator) VALUES "
                            "(1, 'Alice', 1), "
                            "(2, 'Bob', 0);";
    rc = sqlite3_exec(db, insertSql, 0, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
        sqlite3_close(db);
        exit(1);
    }
    
    std::cout << "Database initialized successfully." << std::endl;
    sqlite3_close(db);
}

bool isUserModerator(int user_id) {
    sqlite3* db;
    sqlite3_stmt* stmt = nullptr;
    bool is_moderator = false;
    bool user_found = false;

    if (sqlite3_open(DB_NAME, &db) != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        throw std::runtime_error("Database connection failed");
    }

    // Rule #2: Use prepared statements to prevent SQL Injection
    const char* sql = "SELECT is_moderator FROM users WHERE id = ?";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        throw std::runtime_error("Database query preparation failed");
    }

    sqlite3_bind_int(stmt, 1, user_id);

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        user_found = true;
        is_moderator = (sqlite3_column_int(stmt, 0) == 1);
    } else if (rc != SQLITE_DONE) {
        std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    if (!user_found) {
        throw std::out_of_range("User not found");
    }
    return is_moderator;
}

int main() {
    setupDatabase();
    
    crow::SimpleApp app;

    CROW_ROUTE(app, "/is_moderator")
    ([](const crow::request& req) {
        char* userIdStr = req.url_params.get("user_id");
        if (!userIdStr) {
            return crow::response(400, "{\"error\": \"user_id parameter is required\"}");
        }

        int userId;
        try {
            // Rule #1: Input validation
            size_t processed_chars;
            userId = std::stoi(userIdStr, &processed_chars);
            if (processed_chars != std::string(userIdStr).length()) {
                 return crow::response(400, "{\"error\": \"Invalid user_id format, non-integer characters found\"}");
            }
        } catch (const std::invalid_argument&) {
            return crow::response(400, "{\"error\": \"Invalid user_id format, must be an integer\"}");
        } catch (const std::out_of_range&) {
            return crow::response(400, "{\"error\": \"Invalid user_id format, number is out of range\"}");
        }

        try {
            bool isMod = isUserModerator(userId);
            crow::json::wvalue response_json;
            response_json["user_id"] = userId;
            response_json["is_moderator"] = isMod;
            return crow::response(200, response_json);
        } catch (const std::out_of_range&) {
            crow::json::wvalue err_json;
            err_json["error"] = "User with id " + std::to_string(userId) + " not found";
            return crow::response(404, err_json);
        } catch (const std::runtime_error&) {
            // Rule #10: Do not leak internal details
            crow::json::wvalue err_json;
            err_json["error"] = "Internal server error";
            return crow::response(500, err_json);
        }
    });
    
    int port = 18080;
    std::cout << "Server starting on port " << port << std::endl;
    std::cout << "\n--- Test Cases ---" << std::endl;
    std::cout << "1. Moderator user: curl \"http://localhost:18080/is_moderator?user_id=1\"" << std::endl;
    std::cout << "2. Non-moderator user: curl \"http://localhost:18080/is_moderator?user_id=2\"" << std::endl;
    std::cout << "3. Non-existent user: curl \"http://localhost:18080/is_moderator?user_id=99\"" << std::endl;
    std::cout << "4. Invalid input: curl \"http://localhost:18080/is_moderator?user_id=abc\"" << std::endl;
    std::cout << "5. Missing input: curl \"http://localhost:18080/is_moderator\"" << std::endl;

    app.port(port).run();

    return 0;
}