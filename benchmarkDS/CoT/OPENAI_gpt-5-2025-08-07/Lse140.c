#include <sqlite3.h>
#include <stdio.h>

sqlite3* connect_db(const char* path) {
    sqlite3* db = NULL;
    if (sqlite3_open(path, &db) != SQLITE_OK) {
        fprintf(stderr, "Failed to connect: %s\n", sqlite3_errmsg(db));
        if (db) sqlite3_close(db);
        return NULL;
    }
    return db;
}

int check_username_exists(sqlite3* db, const char* username) {
    const char* sql = "SELECT 1 FROM users WHERE username = ? LIMIT 1";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Prepare failed: %s\n", sqlite3_errmsg(db));
        return 0;
    }
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    int exists = (rc == SQLITE_ROW) ? 1 : 0;
    sqlite3_finalize(stmt);
    return exists;
}

void setup_database(sqlite3* db) {
    const char* createSql = "CREATE TABLE IF NOT EXISTS users (username TEXT PRIMARY KEY)";
    char* errMsg = NULL;
    if (sqlite3_exec(db, createSql, NULL, NULL, &errMsg) != SQLITE_OK) {
        fprintf(stderr, "Create table error: %s\n", errMsg);
        sqlite3_free(errMsg);
    }

    const char* insertSql = "INSERT OR IGNORE INTO users (username) VALUES (?);";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, insertSql, -1, &stmt, NULL) == SQLITE_OK) {
        const char* users[] = {"alice", "bob", "charlie"};
        for (int i = 0; i < 3; ++i) {
            sqlite3_bind_text(stmt, 1, users[i], -1, SQLITE_TRANSIENT);
            sqlite3_step(stmt);
            sqlite3_reset(stmt);
            sqlite3_clear_bindings(stmt);
        }
        sqlite3_finalize(stmt);
    } else {
        fprintf(stderr, "Prepare insert failed: %s\n", sqlite3_errmsg(db));
    }
}

int main(void) {
    sqlite3* db = connect_db(":memory:");
    if (!db) return 1;

    setup_database(db);

    const char* tests[5] = {"alice", "bob", "charlie", "dave", "eve"}; // 5 test cases
    for (int i = 0; i < 5; ++i) {
        int exists = check_username_exists(db, tests[i]);
        printf("%s -> %s\n", tests[i], exists ? "true" : "false");
    }

    sqlite3_close(db);
    return 0;
}