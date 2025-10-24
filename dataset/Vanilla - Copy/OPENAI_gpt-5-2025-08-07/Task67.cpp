#include <iostream>
#include <string>
#include <sqlite3.h>

struct Customer {
    int id;
    std::string customerusername;
    std::string name;
    std::string email;
    std::string phone;
    std::string address;
};

static void execOrThrow(sqlite3* db, const char* sql) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQLite error: " << (errMsg ? errMsg : "unknown") << std::endl;
        sqlite3_free(errMsg);
        throw std::runtime_error("SQLite exec failed");
    }
}

sqlite3* initDatabase() {
    sqlite3* db = nullptr;
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
        std::cerr << "Failed to open DB\n";
        return nullptr;
    }
    execOrThrow(db, "CREATE TABLE customer ("
                    "id INTEGER PRIMARY KEY,"
                    "customerusername TEXT UNIQUE,"
                    "name TEXT,"
                    "email TEXT,"
                    "phone TEXT,"
                    "address TEXT)");
    execOrThrow(db, "INSERT INTO customer VALUES "
                    "(1, 'alice', 'Alice Johnson', 'alice@example.com', '111-222-3333', '123 Maple St'),"
                    "(2, 'bob', 'Bob Smith', 'bob@example.com', '222-333-4444', '456 Oak Ave'),"
                    "(3, 'charlie', 'Charlie Lee', 'charlie@example.com', '333-444-5555', '789 Pine Rd'),"
                    "(4, 'diana', 'Diana Prince', 'diana@example.com', '444-555-6666', '101 Cedar Blvd'),"
                    "(5, 'eric', 'Eric Yang', 'eric@example.com', '555-666-7777', '202 Birch Ln')");
    return db;
}

bool getCustomerByUsername(sqlite3* db, const std::string& customerusername, Customer& out) {
    const char* sql = "SELECT id, customerusername, name, email, phone, address FROM customer WHERE customerusername = ?";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_text(stmt, 1, customerusername.c_str(), -1, SQLITE_TRANSIENT);
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        out.id = sqlite3_column_int(stmt, 0);
        out.customerusername = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        out.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        out.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        out.phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        out.address = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        sqlite3_finalize(stmt);
        return true;
    }
    sqlite3_finalize(stmt);
    return false;
}

static std::string toJson(const Customer& c) {
    auto esc = [](const std::string& s) {
        std::string r;
        for (char ch : s) {
            if (ch == '\\' || ch == '"') r.push_back('\\');
            r.push_back(ch);
        }
        return r;
    };
    std::string j = "{";
    j += "\"id\":" + std::to_string(c.id) + ",";
    j += "\"customerusername\":\"" + esc(c.customerusername) + "\",";
    j += "\"name\":\"" + esc(c.name) + "\",";
    j += "\"email\":\"" + esc(c.email) + "\",";
    j += "\"phone\":\"" + esc(c.phone) + "\",";
    j += "\"address\":\"" + esc(c.address) + "\"";
    j += "}";
    return j;
}

int main(int argc, char** argv) {
    sqlite3* db = initDatabase();
    if (!db) return 1;

    if (argc > 1) {
        Customer c;
        if (getCustomerByUsername(db, argv[1], c)) {
            std::cout << toJson(c) << std::endl;
        } else {
            std::cout << "null" << std::endl;
        }
        sqlite3_close(db);
        return 0;
    }

    const char* tests[5] = {"alice", "bob", "charlie", "diana", "unknown"};
    for (int i = 0; i < 5; ++i) {
        Customer c;
        if (getCustomerByUsername(db, tests[i], c)) {
            std::cout << toJson(c) << std::endl;
        } else {
            std::cout << "null" << std::endl;
        }
    }
    sqlite3_close(db);
    return 0;
}