/* Chain-of-Through secure solution in C (Single-file).
 * 1) Connect to SQLite, query by username, return via output struct.
 * 2) Security: prepared statements, no SQL concatenation, check return codes, finalize statements.
 * 3) Implement with sqlite3 C API.
 * 4) Reviewed for resource cleanup and safe error handling.
 * 5) Final secured code below.
 */

#include <sqlite3.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    char username[128];
    char full_name[256];
    char email[256];
    char role[64];
} User;

/* Returns 1 if found (and fills out), 0 if not found, -1 on error */
int get_user_info(sqlite3* db, const char* username, User* out) {
    if (!db || !username) return -1;
    const char* sql = "SELECT username, full_name, email, role FROM users WHERE username = ?;";
    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        if (stmt) sqlite3_finalize(stmt);
        return -1;
    }
    rc = sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        if (out) {
            const unsigned char* u = sqlite3_column_text(stmt, 0);
            const unsigned char* f = sqlite3_column_text(stmt, 1);
            const unsigned char* e = sqlite3_column_text(stmt, 2);
            const unsigned char* r = sqlite3_column_text(stmt, 3);
            /* Safe copying with truncation */
            snprintf(out->username, sizeof(out->username), "%s", u ? (const char*)u : "");
            snprintf(out->full_name, sizeof(out->full_name), "%s", f ? (const char*)f : "");
            snprintf(out->email, sizeof(out->email), "%s", e ? (const char*)e : "");
            snprintf(out->role, sizeof(out->role), "%s", r ? (const char*)r : "");
        }
        sqlite3_finalize(stmt);
        return 1;
    } else if (rc == SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return 0;
    } else {
        sqlite3_finalize(stmt);
        return -1;
    }
}

int seed_test_data(sqlite3* db) {
    const char* create_sql =
        "CREATE TABLE IF NOT EXISTS users ("
        "  username TEXT PRIMARY KEY,"
        "  full_name TEXT NOT NULL,"
        "  email TEXT NOT NULL,"
        "  role TEXT NOT NULL"
        ");";
    char* errMsg = NULL;
    int rc = sqlite3_exec(db, create_sql, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        if (errMsg) sqlite3_free(errMsg);
        return -1;
    }

    const char* insert_sql = "INSERT INTO users (username, full_name, email, role) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt = NULL;
    rc = sqlite3_prepare_v2(db, insert_sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        if (stmt) sqlite3_finalize(stmt);
        return -1;
    }

    /* Helper to insert one row safely */
    int insert_one(const char* u, const char* f, const char* e, const char* r) {
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        if (sqlite3_bind_text(stmt, 1, u, -1, SQLITE_TRANSIENT) != SQLITE_OK) return -1;
        if (sqlite3_bind_text(stmt, 2, f, -1, SQLITE_TRANSIENT) != SQLITE_OK) return -1;
        if (sqlite3_bind_text(stmt, 3, e, -1, SQLITE_TRANSIENT) != SQLITE_OK) return -1;
        if (sqlite3_bind_text(stmt, 4, r, -1, SQLITE_TRANSIENT) != SQLITE_OK) return -1;
        int rc2 = sqlite3_step(stmt);
        if (rc2 != SQLITE_DONE) return -1;
        return 0;
    }

    int ok = 0;
    ok |= insert_one("alice", "Alice Johnson", "alice@example.com", "user");
    ok |= insert_one("bob", "Bob Smith", "bob@example.com", "user");
    ok |= insert_one("charlie", "Charlie Rose", "charlie@example.com", "manager");
    ok |= insert_one("admin", "System Admin", "admin@example.com", "admin");
    ok |= insert_one("eve", "Eve Adams", "eve@example.com", "auditor");

    sqlite3_finalize(stmt);
    return ok == 0 ? 0 : -1;
}

int main(void) {
    sqlite3* db = NULL;
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
        printf("Database error occurred.\n");
        if (db) sqlite3_close(db);
        return 0;
    }

    if (seed_test_data(db) != 0) {
        printf("Database error occurred.\n");
        sqlite3_close(db);
        return 0;
    }

    const char* tests[5] = {
        "alice",
        "charlie",
        "nonexistent",
        "admin' OR '1'='1",
        "admin"
    };

    for (int i = 0; i < 5; ++i) {
        const char* uname = tests[i];
        User u;
        int res = get_user_info(db, uname, &u);
        if (res == 1) {
            printf("Query username: %s -> User{username='%s', fullName='%s', email='%s', role='%s'}\n",
                   uname, u.username, u.full_name, u.email, u.role);
        } else if (res == 0) {
            printf("Query username: %s -> Not found\n", uname);
        } else {
            printf("Error querying for username: %s\n", uname);
        }
    }

    sqlite3_close(db);
    return 0;
}