#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Requires SQLite3 development headers and library to compile/link.
// Example compile: gcc app.c -lsqlite3 -o app
#include <sqlite3.h>

sqlite3* connect_db() {
    sqlite3* db = NULL;
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
        return NULL;
    }
    return db;
}

int exec_sql(sqlite3* db, const char* sql) {
    char* err = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        if (err) sqlite3_free(err);
        return 0;
    }
    return 1;
}

int seed_db(sqlite3* db, const char* data[][2], size_t n) {
    if (!exec_sql(db, "CREATE TABLE users (user_id TEXT PRIMARY KEY, is_moderator INTEGER NOT NULL);")) return 0;
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, "INSERT INTO users(user_id, is_moderator) VALUES(?, ?);", -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }
    for (size_t i = 0; i < n; ++i) {
        sqlite3_bind_text(stmt, 1, data[i][0], -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, atoi(data[i][1]));
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return 0;
        }
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
    }
    sqlite3_finalize(stmt);
    return 1;
}

// Execute a query to check if the user is a moderator
int is_moderator(sqlite3* db, const char* user_id) {
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, "SELECT is_moderator FROM users WHERE user_id = ?;", -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }
    sqlite3_bind_text(stmt, 1, user_id, -1, SQLITE_TRANSIENT);
    int rc = sqlite3_step(stmt);
    int result = 0;
    if (rc == SQLITE_ROW) {
        int val = sqlite3_column_int(stmt, 0);
        result = (val != 0);
    } else {
        result = 0;
    }
    sqlite3_finalize(stmt);
    return result;
}

// Route handler called when the route is called
const char* handle_route(sqlite3* db, const char* user_id) {
    return is_moderator(db, user_id) ? "true" : "false";
}

// Minimal router abstraction
typedef const char* (*route_handler_t)(sqlite3*, const char*);

typedef struct {
    const char* path;
    route_handler_t handler;
} Route;

const char* call_route(Route* routes, size_t count, const char* path, sqlite3* db, const char* user_id) {
    for (size_t i = 0; i < count; ++i) {
        if (strcmp(routes[i].path, path) == 0) {
            return routes[i].handler(db, user_id);
        }
    }
    return "404";
}

int main() {
    sqlite3* db = connect_db();
    if (!db) {
        fprintf(stderr, "DB connection failed\n");
        return 1;
    }
    const char* data[][2] = {
        {"1","1"},{"2","0"},{"3","1"},{"4","0"},{"5","1"}
    };
    if (!seed_db(db, data, sizeof(data)/sizeof(data[0]))) {
        fprintf(stderr, "DB seed failed\n");
        sqlite3_close(db);
        return 1;
    }

    Route routes[] = {
        {"/isModerator", handle_route}
    };

    // 5 test cases
    printf("%s\n", call_route(routes, 1, "/isModerator", db, "1"));   // true
    printf("%s\n", call_route(routes, 1, "/isModerator", db, "2"));   // false
    printf("%s\n", call_route(routes, 1, "/isModerator", db, "3"));   // true
    printf("%s\n", call_route(routes, 1, "/isModerator", db, "4"));   // false
    printf("%s\n", call_route(routes, 1, "/isModerator", db, "999")); // false

    sqlite3_close(db);
    return 0;
}