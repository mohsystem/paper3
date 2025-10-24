#include <iostream>
#include <string>
#include <memory>
#include <regex>

// Third-party libraries are expected to be available.
// Please download and place them in your include path.
// cpp-httplib: https://github.com/yhirose/cpp-httplib
// nlohmann/json: https://github.com/nlohmann/json
#include "httplib.h"
#include "json.hpp"
#include "sqlite3.h"

// --- Database Setup and RAII Wrappers ---

// RAII wrapper for sqlite3 database connection
class SQLiteDB {
public:
    explicit SQLiteDB(const char* filename) {
        if (sqlite3_open(filename, &db_)) {
            std::cerr << "Can't open database: " << sqlite3_errmsg(db_) << std::endl;
            db_ = nullptr;
        }
    }

    ~SQLiteDB() {
        if (db_) {
            sqlite3_close(db_);
        }
    }

    sqlite3* get() const {
        return db_;
    }

    // Disable copy and assign
    SQLiteDB(const SQLiteDB&) = delete;
    SQLiteDB& operator=(const SQLiteDB&) = delete;

private:
    sqlite3* db_ = nullptr;
};

// RAII wrapper for sqlite3_stmt
class SQLiteStmt {
public:
    SQLiteStmt(sqlite3* db, const char* query) {
        if (sqlite3_prepare_v2(db, query, -1, &stmt_, nullptr) != SQLITE_OK) {
            std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
            stmt_ = nullptr;
        }
    }

    ~SQLiteStmt() {
        if (stmt_) {
            sqlite3_finalize(stmt_);
        }
    }

    sqlite3_stmt* get() const {
        return stmt_;
    }

    // Disable copy and assign
    SQLiteStmt(const SQLiteStmt&) = delete;
    SQLiteStmt& operator=(const SQLiteStmt&) = delete;

private:
    sqlite3_stmt* stmt_ = nullptr;
};

// Global database object
std::unique_ptr<SQLiteDB> g_db;

void initialize_database() {
    g_db = std::make_unique<SQLiteDB>("users.db");
    if (!g_db->get()) {
        exit(1);
    }

    char* err_msg = nullptr;
    const char* sql = "CREATE TABLE IF NOT EXISTS users("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "name TEXT NOT NULL, "
                      "email TEXT NOT NULL UNIQUE);";

    int rc = sqlite3_exec(g_db->get(), sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << err_msg << std::endl;
        sqlite3_free(err_msg);
        exit(1);
    }
}

// --- Input Validation ---
bool is_valid_email(const std::string& email) {
    if (email.length() > 254) return false;
    // Simple regex for email validation
    const std::regex pattern(R"((\w+)(\.{1}\w+)*@(\w+)(\.\w+)+)");
    return std::regex_match(email, pattern);
}

// --- API Endpoint Handler ---
void handle_user_creation(const httplib::Request& req, httplib::Response& res) {
    // Rule #3: Validate input format
    if (req.get_header_value("Content-Type") != "application/json") {
        res.set_content("Unsupported Media Type. Must be application/json.", "text/plain");
        res.status = 415;
        return;
    }

    nlohmann::json body;
    try {
        body = nlohmann::json::parse(req.body);
    } catch (const nlohmann::json::parse_error& e) {
        res.set_content("Invalid JSON format: " + std::string(e.what()), "text/plain");
        res.status = 400;
        return;
    }

    if (!body.contains("name") || !body.contains("email") || !body["name"].is_string() || !body["email"].is_string()) {
        res.set_content("Missing or invalid 'name' or 'email' fields.", "text/plain");
        res.status = 400;
        return;
    }

    std::string name = body["name"];
    std::string email = body["email"];

    // Rule #3: Validate input content
    if (name.empty() || name.length() > 100) {
        res.set_content("Name must be between 1 and 100 characters.", "text/plain");
        res.status = 400;
        return;
    }
    if (!is_valid_email(email)) {
        res.set_content("Invalid email format.", "text/plain");
        res.status = 400;
        return;
    }

    // Use prepared statements to prevent SQL injection
    SQLiteStmt stmt(g_db->get(), "INSERT INTO users (name, email) VALUES (?, ?);");
    if (!stmt.get()) {
        res.set_content("Database internal error.", "text/plain");
        res.status = 500;
        return;
    }

    sqlite3_bind_text(stmt.get(), 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt.get(), 2, email.c_str(), -1, SQLITE_STATIC);

    int rc = sqlite3_step(stmt.get());

    if (rc == SQLITE_DONE) {
        res.set_content("User created successfully.", "text/plain");
        res.status = 201;
    } else if (rc == SQLITE_CONSTRAINT) {
        res.set_content("Email already exists.", "text/plain");
        res.status = 409;
    } else {
        res.set_content("Failed to create user: " + std::string(sqlite3_errmsg(g_db->get())), "text/plain");
        res.status = 500;
    }
}

// --- Main Server ---
int main() {
    initialize_database();

    // NOTE: This is an HTTP server. For production use, you must use httplib::SSLServer
    // which requires providing a certificate and private key file to enable HTTPS.
    // Example: httplib::SSLServer svr("cert.pem", "key.pem");
    httplib::Server svr;

    svr.Post("/api/users", handle_user_creation);

    std::cout << "Server starting on http://localhost:8080..." << std::endl;
    std::cout << "You can use curl to test the endpoint." << std::endl;
    std::cout << "\n--- Test Cases ---\n";
    std::cout << "1. Create a new user (Success):\n";
    std::cout << "   curl -X POST -H \"Content-Type: application/json\" -d '{\"name\": \"John Doe\", \"email\": \"john.doe@example.com\"}' http://localhost:8080/api/users\n\n";
    std::cout << "2. Attempt to create a duplicate user (Conflict):\n";
    std::cout << "   curl -X POST -H \"Content-Type: application/json\" -d '{\"name\": \"John Doe\", \"email\": \"john.doe@example.com\"}' http://localhost:8080/api/users\n\n";
    std::cout << "3. Send invalid JSON (Bad Request):\n";
    std::cout << "   curl -X POST -H \"Content-Type: application/json\" -d '{\"name\": \"Jane Doe\", \"email\": \"jane.doe@example.com\"' http://localhost:8080/api/users\n\n";
    std::cout << "4. Send data with missing fields (Bad Request):\n";
    std::cout << "   curl -X POST -H \"Content-Type: application/json\" -d '{\"name\": \"Jane Doe\"}' http://localhost:8080/api/users\n\n";
    std::cout << "5. Send data with invalid email format (Bad Request):\n";
    std::cout << "   curl -X POST -H \"Content-Type: application/json\" -d '{\"name\": \"Invalid Email\", \"email\": \"invalid-email\"}' http://localhost:8080/api/users\n\n";
    
    svr.listen("0.0.0.0", 8080);

    return 0;
}