#include <sqlite3.h>
#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <limits>

struct Customer {
    int id = 0;
    std::string username;
    std::string fullname;
    std::string email;
    std::string created_at;
    bool found = false;
};

static bool is_valid_username(const std::string& u) {
    if (u.empty() || u.size() > 64) return false;
    for (unsigned char c : u) {
        if (!(std::isalnum(c) || c == '_' || c == '.' || c == '-')) return false;
    }
    return true;
}

static int exec_sql(sqlite3* db, const char* sql) {
    char* err = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        if (err) {
            std::cerr << "SQL error: " << err << "\n";
            sqlite3_free(err);
        }
    }
    return rc;
}

static sqlite3* open_db(const std::string& path) {
    sqlite3* db = nullptr;
    int rc = sqlite3_open(path.c_str(), &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << "\n";
        if (db) sqlite3_close(db);
        return nullptr;
    }
    return db;
}

static int ensure_schema_and_seed(sqlite3* db) {
    const char* schema =
        "PRAGMA foreign_keys=ON;"
        "CREATE TABLE IF NOT EXISTS customer ("
        "  id INTEGER PRIMARY KEY,"
        "  customerusername TEXT UNIQUE NOT NULL CHECK (length(customerusername) <= 64),"
        "  fullname TEXT NOT NULL,"
        "  email TEXT NOT NULL,"
        "  created_at TEXT NOT NULL"
        ");";
    int rc = exec_sql(db, schema);
    if (rc != SQLITE_OK) return rc;

    const char* seed =
        "INSERT OR IGNORE INTO customer (id, customerusername, fullname, email, created_at) VALUES "
        "(1, 'alice',   'Alice Anderson',  'alice@example.com',   '2023-01-01T10:00:00Z'),"
        "(2, 'bob',     'Bob Brown',       'bob@example.com',     '2023-02-14T12:30:00Z'),"
        "(3, 'charlie', 'Charlie Clark',   'charlie@example.com', '2023-03-20T08:45:00Z'),"
        "(4, 'dora',    'Dora Davis',      'dora@example.com',    '2023-04-05T16:20:00Z');";
    rc = exec_sql(db, seed);
    return rc;
}

static Customer fetch_customer(sqlite3* db, const std::string& username) {
    Customer cust;
    if (!db) {
        std::cerr << "Database handle is null.\n";
        return cust;
    }
    if (!is_valid_username(username)) {
        std::cerr << "Invalid username format. Allowed: alphanumeric, _, -, . length<=64\n";
        return cust;
    }

    const char* sql =
        "SELECT id, customerusername, fullname, email, created_at "
        "FROM customer WHERE customerusername = ?1;";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << "\n";
        return cust;
    }

    rc = sqlite3_bind_text(stmt, 1, username.c_str(), static_cast<int>(username.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to bind parameter: " << sqlite3_errmsg(db) << "\n";
        sqlite3_finalize(stmt);
        return cust;
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        cust.id = sqlite3_column_int(stmt, 0);

        const unsigned char* u = sqlite3_column_text(stmt, 1);
        int ulen = sqlite3_column_bytes(stmt, 1);
        cust.username.assign(reinterpret_cast<const char*>(u ? u : reinterpret_cast<const unsigned char*>("")), ulen);

        const unsigned char* f = sqlite3_column_text(stmt, 2);
        int flen = sqlite3_column_bytes(stmt, 2);
        cust.fullname.assign(reinterpret_cast<const char*>(f ? f : reinterpret_cast<const unsigned char*>("")), flen);

        const unsigned char* e = sqlite3_column_text(stmt, 3);
        int elen = sqlite3_column_bytes(stmt, 3);
        cust.email.assign(reinterpret_cast<const char*>(e ? e : reinterpret_cast<const unsigned char*>("")), elen);

        const unsigned char* c = sqlite3_column_text(stmt, 4);
        int clen = sqlite3_column_bytes(stmt, 4);
        cust.created_at.assign(reinterpret_cast<const char*>(c ? c : reinterpret_cast<const unsigned char*>("")), clen);

        cust.found = true;
    } else if (rc != SQLITE_DONE) {
        std::cerr << "Query failed: " << sqlite3_errmsg(db) << "\n";
    }

    sqlite3_finalize(stmt);
    return cust;
}

int main(int argc, char* argv[]) {
    sqlite3* db = open_db("customers.db");
    if (!db) return 1;
    if (ensure_schema_and_seed(db) != SQLITE_OK) {
        sqlite3_close(db);
        return 1;
    }

    std::vector<std::string> tests;
    if (argc >= 2) {
        tests.push_back(argv[1]);
        tests.push_back("alice");
        tests.push_back("bob");
        tests.push_back("charlie");
        tests.push_back("nonexistent");
    } else {
        tests = {"alice", "bob", "charlie", "dora", "nonexistent"};
    }

    for (const auto& name : tests) {
        Customer c = fetch_customer(db, name);
        if (c.found) {
            std::cout << "FOUND: id=" << c.id
                      << " username=" << c.username
                      << " fullname=" << c.fullname
                      << " email=" << c.email
                      << " created_at=" << c.created_at << "\n";
        } else {
            std::cout << "NOT FOUND: username=" << name << "\n";
        }
    }

    sqlite3_close(db);
    return 0;
}