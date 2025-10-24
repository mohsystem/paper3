#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <cstdlib>
#include <sqlite3.h>

struct Customer {
    int id;
    std::string username;
    std::string full_name;
    std::string email;
    std::string created_at;
};

static bool isValidUsername(const std::string& u) {
    static const std::regex pat("^[A-Za-z0-9._-]{1,64}$");
    return std::regex_match(u, pat);
}

static int execNoResult(sqlite3* db, const char* sql) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        if (errMsg) sqlite3_free(errMsg);
    }
    return rc;
}

static void initSchemaAndSampleData(sqlite3* db) {
    const char* createTable =
        "CREATE TABLE IF NOT EXISTS customer ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT UNIQUE NOT NULL,"
        "full_name TEXT NOT NULL,"
        "email TEXT NOT NULL,"
        "created_at TEXT NOT NULL"
        ");";
    execNoResult(db, createTable);

    sqlite3_stmt* stmt = nullptr;
    const char* insertSQL = "INSERT OR IGNORE INTO customer(username, full_name, email, created_at) VALUES (?,?,?,?)";
    if (sqlite3_prepare_v2(db, insertSQL, -1, &stmt, nullptr) == SQLITE_OK) {
        auto insertOne = [&](const char* u, const char* f, const char* e, const char* c) {
            sqlite3_reset(stmt);
            sqlite3_bind_text(stmt, 1, u, -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, f, -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, e, -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 4, c, -1, SQLITE_TRANSIENT);
            sqlite3_step(stmt);
        };
        insertOne("alice", "Alice Johnson", "alice@example.com", "2023-01-10T09:15:00Z");
        insertOne("bob", "Bob Smith", "bob@example.com", "2023-02-12T10:20:00Z");
        insertOne("carol", "Carol White", "carol@example.com", "2023-03-15T11:25:00Z");
        insertOne("dave", "Dave Brown", "dave@example.com", "2023-04-18T12:30:00Z");
        insertOne("eve", "Eve Black", "eve@example.com", "2023-05-20T13:35:00Z");
        sqlite3_finalize(stmt);
    }
}

static sqlite3* getConnectionOrInitSample() {
    sqlite3* db = nullptr;
    const char* dbPath = std::getenv("DB_PATH");
    const char* openPath = dbPath && *dbPath ? dbPath : ":memory:";
    if (sqlite3_open_v2(openPath, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return nullptr;
    }
    if (!dbPath || !*dbPath) {
        initSchemaAndSampleData(db);
    }
    return db;
}

// Returns true and fills out if found; returns false otherwise.
static bool getCustomerByUsername(sqlite3* db, const std::string& username, Customer& out) {
    if (!db || !isValidUsername(username)) return false;

    const char* sql = "SELECT id, username, full_name, email, created_at FROM customer WHERE username = ?";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    bool found = false;
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        out.id = sqlite3_column_int(stmt, 0);
        out.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        out.full_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        out.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        out.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        found = true;
    }
    sqlite3_finalize(stmt);
    return found;
}

int main(int argc, char** argv) {
    sqlite3* db = getConnectionOrInitSample();
    if (!db) {
        std::cerr << "Failed to open database.\n";
        return 1;
    }

    if (argc > 1) {
        std::string username = argv[1];
        Customer c;
        if (getCustomerByUsername(db, username, c)) {
            std::cout << "Customer{id=" << c.id << ", username='" << c.username
                      << "', fullName='" << c.full_name << "', email='" << c.email
                      << "', createdAt='" << c.created_at << "'}\n";
        } else {
            std::cout << "No customer found for username: " << username << "\n";
        }
    } else {
        std::vector<std::string> tests = {"alice", "bob", "carol", "dave", "mallory"};
        for (const auto& t : tests) {
            Customer c;
            if (getCustomerByUsername(db, t, c)) {
                std::cout << "Customer{id=" << c.id << ", username='" << c.username
                          << "', fullName='" << c.full_name << "', email='" << c.email
                          << "', createdAt='" << c.created_at << "'}\n";
            } else {
                std::cout << "No customer found for username: " << t << "\n";
            }
        }
    }

    sqlite3_close(db);
    return 0;
}