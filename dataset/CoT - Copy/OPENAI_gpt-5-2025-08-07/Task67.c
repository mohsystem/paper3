/* Chain-of-Through Process in Code (C):
 * 1) Problem understanding: Retrieve a customer by 'customerusername' from 'customer' table; username from command-line; return structured data.
 * 2) Security requirements: Prepared statements, input validation, safe resource handling.
 * 3) Secure coding generation: Use sqlite3 with bound parameters, strict username validation, robust checks.
 * 4) Code review: Ensure no SQL injection, finalize statements, close DB, handle errors.
 * 5) Secure code output: Final secure C implementation below.
 */

#include <sqlite3.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    int id;
    char username[65];
    char fullname[256];
    char email[256];
    char created_at[64];
} Customer;

static bool is_valid_username(const char* username) {
    if (!username) return false;
    size_t len = strlen(username);
    if (len == 0 || len > 64) return false;
    for (size_t i = 0; i < len; i++) {
        char c = username[i];
        if (!((c >= 'A' && c <= 'Z') ||
              (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') ||
              c == '_' || c == '.' || c == '-')) {
            return false;
        }
    }
    return true;
}

static int ensure_schema_and_seed(sqlite3* db) {
    const char* create_sql =
        "CREATE TABLE IF NOT EXISTS customer ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "customerusername TEXT UNIQUE NOT NULL,"
        "fullname TEXT NOT NULL,"
        "email TEXT NOT NULL,"
        "created_at TEXT DEFAULT CURRENT_TIMESTAMP"
        ")";
    char* err = NULL;
    int rc = sqlite3_exec(db, create_sql, NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        if (err) sqlite3_free(err);
        return rc;
    }

    const char* upsert_sql =
        "INSERT INTO customer (customerusername, fullname, email) "
        "VALUES (?, ?, ?) "
        "ON CONFLICT(customerusername) DO UPDATE SET "
        "fullname=excluded.fullname, email=excluded.email";
    sqlite3_stmt* stmt = NULL;
    rc = sqlite3_prepare_v2(db, upsert_sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return rc;

    struct Seed { const char* u; const char* f; const char* e; } seeds[] = {
        {"alice","Alice Anderson","alice@example.com"},
        {"bob","Bob Brown","bob@example.com"},
        {"charlie","Charlie Clark","charlie@example.com"},
        {"dora","Dora Dawson","dora@example.net"},
        {"eve","Eve Edwards","eve@example.org"}
    };
    for (size_t i = 0; i < sizeof(seeds)/sizeof(seeds[0]); i++) {
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        sqlite3_bind_text(stmt, 1, seeds[i].u, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, seeds[i].f, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, seeds[i].e, -1, SQLITE_TRANSIENT);
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return rc;
        }
    }
    sqlite3_finalize(stmt);
    return SQLITE_OK;
}

static bool get_customer_by_username(sqlite3* db, const char* username, Customer* out) {
    if (!out) return false;
    if (!is_valid_username(username)) {
        return false;
    }
    const char* sql =
        "SELECT id, customerusername, fullname, email, COALESCE(created_at,'') "
        "FROM customer WHERE customerusername = ? LIMIT 1";
    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    bool found = false;
    if (rc == SQLITE_ROW) {
        out->id = sqlite3_column_int(stmt, 0);
        const unsigned char* u = sqlite3_column_text(stmt, 1);
        const unsigned char* f = sqlite3_column_text(stmt, 2);
        const unsigned char* e = sqlite3_column_text(stmt, 3);
        const unsigned char* c = sqlite3_column_text(stmt, 4);
        snprintf(out->username, sizeof(out->username), "%s", u ? (const char*)u : "");
        snprintf(out->fullname, sizeof(out->fullname), "%s", f ? (const char*)f : "");
        snprintf(out->email, sizeof(out->email), "%s", e ? (const char*)e : "");
        snprintf(out->created_at, sizeof(out->created_at), "%s", c ? (const char*)c : "");
        found = true;
    }
    sqlite3_finalize(stmt);
    return found;
}

int main(int argc, char** argv) {
    sqlite3* db = NULL;
    if (sqlite3_open("customers.db", &db) != SQLITE_OK) {
        printf("{\"error\":\"database_error\"}\n");
        if (db) sqlite3_close(db);
        return 0;
    }

    if (ensure_schema_and_seed(db) != SQLITE_OK) {
        printf("{\"error\":\"database_error\"}\n");
        sqlite3_close(db);
        return 0;
    }

    const char* tests[5] = {0};
    size_t count = 0;

    if (argc >= 2 && argv[1] && strlen(argv[1]) > 0) {
        tests[count++] = argv[1];
    }

    const char* defaults[] = {"alice","bob","charlie","not_exist_user_123","eve"};
    for (size_t i = 0; i < 5 && count < 5; i++) {
        const char* d = defaults[i];
        bool exists = false;
        for (size_t j = 0; j < count; j++) {
            if (tests[j] && strcmp(tests[j], d) == 0) { exists = true; break; }
        }
        if (!exists) tests[count++] = d;
    }
    while (count < 5) tests[count++] = "dora";

    for (size_t i = 0; i < 5; i++) {
        Customer c;
        if (get_customer_by_username(db, tests[i], &c)) {
            printf("{\"id\":%d,\"customerusername\":\"%s\",\"fullname\":\"%s\",\"email\":\"%s\",\"created_at\":\"%s\"}\n",
                   c.id, c.username, c.fullname, c.email, c.created_at);
        } else {
            printf("{\"customerusername\":\"%s\",\"result\":\"not_found_or_invalid\"}\n", tests[i]);
        }
    }

    sqlite3_close(db);
    return 0;
}