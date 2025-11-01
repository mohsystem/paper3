#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sqlite3.h>

// Securely checks if a username exists in the SQLite database at dbPath.
// Returns true if exists, false otherwise.
bool usernameExists(const char* dbPath, const char* username) {
    if (dbPath == NULL || username == NULL || username[0] == '\0') {
        return false;
    }

    sqlite3* db = NULL;
    if (sqlite3_open(dbPath, &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return false;
    }

    const char* sql = "SELECT 1 FROM users WHERE username = ? LIMIT 1";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_close(db);
        return false;
    }

    if (sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT) != SQLITE_OK) {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }

    int rc = sqlite3_step(stmt);
    bool exists = (rc == SQLITE_ROW);

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return exists;
}

static bool execSQL(sqlite3* db, const char* sql) {
    char* errMsg = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        if (errMsg) sqlite3_free(errMsg);
        return false;
    }
    return true;
}

int main(void) {
    const char* dbPath = "users_c.db";
    sqlite3* db = NULL;
    if (sqlite3_open(dbPath, &db) != SQLITE_OK) {
        printf("DB init failed\n");
        if (db) sqlite3_close(db);
        return 0;
    }

    bool ok = execSQL(db, "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT UNIQUE NOT NULL)");
    if (ok) ok = execSQL(db, "INSERT OR IGNORE INTO users (username) VALUES ('alice'), ('bob'), ('eve')");
    sqlite3_close(db);
    if (!ok) {
        printf("DB init failed\n");
        return 0;
    }

    const char* tests[5] = {
        "alice",
        "bob",
        "charlie",
        "",
        "Robert'); DROP TABLE users;--"
    };

    for (int i = 0; i < 5; ++i) {
        bool exists = usernameExists(dbPath, tests[i]);
        printf("usernameExists(\"%s\") = %s\n", tests[i], exists ? "true" : "false");
    }

    return 0;
}