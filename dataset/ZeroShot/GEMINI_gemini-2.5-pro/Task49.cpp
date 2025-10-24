/*
 * DEPENDENCIES:
 * This code requires three external libraries:
 * 1. cpp-httplib: A single-header HTTP/HTTPS server library.
 *    - Download: https://github.com/yhirose/cpp-httplib
 *    - Place `httplib.h` in your include path or the same directory.
 * 2. nlohmann/json: A single-header JSON library for C++.
 *    - Download: https://github.com/nlohmann/json
 *    - Place the `json.hpp` file from the `single_include/nlohmann` directory
 *      into your include path or the same directory.
 * 3. SQLite3: The database engine.
 *    - On Linux (Debian/Ubuntu): sudo apt-get install libsqlite3-dev
 *    - On macOS (Homebrew): brew install sqlite
 *    - On Windows: Download precompiled binaries from https://www.sqlite.org/download.html
 *
 * COMPILATION:
 * g++ -std=c++17 -o Task49 Task49.cpp -lsqlite3 -pthread
 *
 * USAGE:
 * 1. Run the compiled executable: ./Task49
 * 2. The server will start on http://localhost:8080
 */
#include <iostream>
#include <string>
#include "httplib.h"      // Must be available in include path
#include "json.hpp"       // Must be available in include path
#include "sqlite3.h"

// Use nlohmann's json library
using json = nlohmann::json;

const char* DB_FILE = "user_database.db";

// Function to initialize the database
void setupDatabase() {
    sqlite3* db;
    char* zErrMsg = 0;
    int rc = sqlite3_open(DB_FILE, &db);

    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        exit(1);
    }

    const char* sql = "CREATE TABLE IF NOT EXISTS users ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "username TEXT NOT NULL UNIQUE,"
                      "email TEXT NOT NULL UNIQUE,"
                      "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
                      ");";

    rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
    }
    sqlite3_close(db);
}

int main(void) {
    setupDatabase();

    httplib::Server svr;

    svr.Post("/users", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Content-Type", "application/json");

        // 1. Parse JSON body
        json body;
        try {
            body = json::parse(req.body);
        } catch (json::parse_error& e) {
            res.status = 400;
            res.set_content("{\"error\":\"Bad Request: Malformed JSON\"}", "application/json");
            return;
        }

        // 2. Input Validation
        if (!body.contains("username") || !body.contains("email") ||
            !body["username"].is_string() || !body["email"].is_string()) {
            res.status = 400;
            res.set_content("{\"error\":\"Bad Request: 'username' and 'email' are required and must be strings\"}", "application/json");
            return;
        }

        std::string username = body["username"];
        std::string email = body["email"];

        if (username.length() > 50 || email.length() > 254 || username.empty() || email.empty()) {
            res.status = 400;
            res.set_content("{\"error\":\"Bad Request: Input is empty or exceeds maximum length\"}", "application/json");
            return;
        }
        // A simple check for '@' in email for basic validation
        if (email.find('@') == std::string::npos) {
            res.status = 400;
            res.set_content("{\"error\":\"Bad Request: Invalid email format\"}", "application/json");
            return;
        }

        // 3. Database Interaction
        sqlite3* db;
        sqlite3_stmt* stmt;
        int rc = sqlite3_open(DB_FILE, &db);
        if (rc) {
            res.status = 500;
            res.set_content("{\"error\":\"Internal Server Error: Could not open database\"}", "application/json");
            return;
        }

        const char* sql = "INSERT INTO users (username, email) VALUES (?, ?);";
        
        // --- Use prepared statements to prevent SQL Injection ---
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
            sqlite3_close(db);
            res.status = 500;
            res.set_content("{\"error\":\"Internal Server Error: Failed to prepare statement\"}", "application/json");
            return;
        }

        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_STATIC);

        rc = sqlite3_step(stmt);

        if (rc == SQLITE_DONE) {
            long long lastId = sqlite3_last_insert_rowid(db);
            json response_json;
            response_json["message"] = "User created successfully";
            response_json["user_id"] = lastId;
            res.status = 201;
            res.set_content(response_json.dump(), "application/json");
        } else if (rc == SQLITE_CONSTRAINT) {
            res.status = 409;
            res.set_content("{\"error\":\"Conflict: Username or email already exists\"}", "application/json");
        } else {
            res.status = 500;
            res.set_content("{\"error\":\"Internal Server Error: Failed to execute statement\"}", "application/json");
        }

        sqlite3_finalize(stmt);
        sqlite3_close(db);
    });
    
    std::cout << "Server starting on http://localhost:8080" << std::endl;
    std::cout << "API Endpoint: POST /users" << std::endl;
    std::cout << "--- 5 Test Cases (use a tool like curl) ---" << std::endl;
    std::cout << "1. Successful creation:" << std::endl;
    std::cout << "   curl -X POST -H \"Content-Type: application/json\" -d '{\"username\":\"alice\",\"email\":\"alice@example.com\"}' http://localhost:8080/users" << std::endl;
    std::cout << std::endl << "2. Duplicate username (409 Conflict):" << std::endl;
    std::cout << "   curl -X POST -H \"Content-Type: application/json\" -d '{\"username\":\"alice\",\"email\":\"alice2@example.com\"}' http://localhost:8080/users" << std::endl;
    std::cout << std::endl << "3. Duplicate email (409 Conflict):" << std::endl;
    std::cout << "   curl -X POST -H \"Content-Type: application/json\" -d '{\"username\":\"alice2\",\"email\":\"alice@example.com\"}' http://localhost:8080/users" << std::endl;
    std::cout << std::endl << "4. Missing data (400 Bad Request):" << std::endl;
    std::cout << "   curl -X POST -H \"Content-Type: application/json\" -d '{\"username\":\"bob\"}' http://localhost:8080/users" << std::endl;
    std::cout << std::endl << "5. Malformed JSON (400 Bad Request):" << std::endl;
    std::cout << "   curl -X POST -H \"Content-Type: application/json\" -d '{\"username\":\"carol\",\"email\":\"carol@example.com\"' http://localhost:8080/users" << std::endl;

    svr.listen("0.0.0.0", 8080);

    return 0;
}