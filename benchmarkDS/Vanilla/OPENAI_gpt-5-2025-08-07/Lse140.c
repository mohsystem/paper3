#include <stdio.h>
#include <sqlite3.h>

int user_exists(sqlite3* db, const char* username) {
    if (db == NULL || username == NULL) return 0;
    const char* sql = "SELECT 1 FROM users WHERE username = ? LIMIT 1";
    sqlite3_stmt* stmt = NULL;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }

    if (sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT) != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return 0;
    }

    int rc = sqlite3_step(stmt);
    int exists = (rc == SQLITE_ROW) ? 1 : 0;

    sqlite3_finalize(stmt);
    return exists;
}

int main() {
    sqlite3* db = NULL;
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
        printf("Cannot open database\n");
        return 1;
    }

    const char* create_sql = "CREATE TABLE users (username TEXT PRIMARY KEY);";
    const char* insert_sql =
        "INSERT INTO users(username) VALUES('alice');"
        "INSERT INTO users(username) VALUES('bob');"
        "INSERT INTO users(username) VALUES('carol');";

    char* err = NULL;
    if (sqlite3_exec(db, create_sql, NULL, NULL, &err) != SQLITE_OK) {
        printf("Create table error: %s\n", err ? err : "");
        sqlite3_free(err);
        sqlite3_close(db);
        return 1;
    }

    if (sqlite3_exec(db, insert_sql, NULL, NULL, &err) != SQLITE_OK) {
        printf("Insert error: %s\n", err ? err : "");
        sqlite3_free(err);
        sqlite3_close(db);
        return 1;
    }

    const char* tests[5] = {"alice", "bob", "dave", "", "Carol"}; // 5 test cases
    for (int i = 0; i < 5; i++) {
        int exists = user_exists(db, tests[i]);
        printf("user_exists(\"%s\") = %s\n", tests[i], exists ? "true" : "false");
    }

    sqlite3_close(db);
    return 0;
}