#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"

static char* json_escape(const unsigned char* input) {
    if (!input) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    const unsigned char* p = input;
    size_t extra = 0;
    while (*p) {
        switch (*p) {
            case '\\': case '"': case '\b': case '\f': case '
': case '\r': case '\t':
                extra += 1; // will become two chars like 
, \"
                break;
            default:
                if (*p < 0x20) extra += 5; // \u00xx
                break;
        }
        p++;
    }
    size_t len = strlen((const char*)input);
    char* out = (char*)malloc(len + extra + 1);
    if (!out) return NULL;

    char* o = out;
    for (p = input; *p; ++p) {
        switch (*p) {
            case '\\': *o++='\\'; *o++='\\'; break;
            case '"':  *o++='\\'; *o++='"';  break;
            case '\b': *o++='\\'; *o++='b';  break;
            case '\f': *o++='\\'; *o++='f';  break;
            case '\n': *o++='\\'; *o++='n';  break;
            case '\r': *o++='\\'; *o++='r';  break;
            case '\t': *o++='\\'; *o++='t';  break;
            default:
                if (*p < 0x20) {
                    sprintf(o, "\\u%04x", *p);
                    o += 6;
                } else {
                    *o++ = (char)*p;
                }
        }
    }
    *o = '\0';
    return out;
}

int initDatabase(sqlite3* db) {
    const char* create_sql =
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY,"
        "username TEXT UNIQUE NOT NULL,"
        "full_name TEXT NOT NULL,"
        "email TEXT NOT NULL"
        ");";
    char* err = NULL;
    if (sqlite3_exec(db, create_sql, NULL, NULL, &err) != SQLITE_OK) {
        if (err) sqlite3_free(err);
        return 1;
    }

    const char* insert_sql = "INSERT OR IGNORE INTO users (id, username, full_name, email) VALUES (?, ?, ?, ?)";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, insert_sql, -1, &stmt, NULL) != SQLITE_OK) return 2;

    struct Row { int id; const char* u; const char* f; const char* e; } rows[] = {
        {1, "alice", "Alice Wonderland", "alice@example.com"},
        {2, "bob", "Bob Builder", "bob@builder.com"},
        {3, "carol", "Carol Singer", "carol@songs.org"},
        {4, "dave", "Dave Grohl", "dave@foofighters.com"}
    };

    for (int i = 0; i < 4; ++i) {
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        sqlite3_bind_int(stmt, 1, rows[i].id);
        sqlite3_bind_text(stmt, 2, rows[i].u, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, rows[i].f, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, rows[i].e, -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return 3;
        }
    }
    sqlite3_finalize(stmt);
    return 0;
}

// Returns a malloc-allocated JSON string or NULL if not found. Caller must free.
char* getUserByUsername(sqlite3* db, const char* username) {
    const char* sql = "SELECT id, username, full_name, email FROM users WHERE username = ?";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return NULL;
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);

    char* result = NULL;
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char* u = sqlite3_column_text(stmt, 1);
        const unsigned char* f = sqlite3_column_text(stmt, 2);
        const unsigned char* e = sqlite3_column_text(stmt, 3);

        char idbuf[32];
        snprintf(idbuf, sizeof(idbuf), "%d", id);

        char* uesc = json_escape(u);
        char* fesc = json_escape(f);
        char* eesc = json_escape(e);

        const char* prefix = "{\"id\":";
        const char* mid1 = ",\"username\":\"";
        const char* mid2 = "\",\"full_name\":\"";
        const char* mid3 = "\",\"email\":\"";
        const char* suffix = "\"}";

        size_t total = strlen(prefix) + strlen(idbuf) + strlen(mid1) + strlen(uesc)
                     + strlen(mid2) + strlen(fesc) + strlen(mid3) + strlen(eesc)
                     + strlen(suffix) + 1;

        result = (char*)malloc(total);
        if (result) {
            snprintf(result, total, "%s%s%s%s%s%s%s%s%s",
                     prefix, idbuf, mid1, uesc, mid2, fesc, mid3, eesc, suffix);
        }

        free(uesc);
        free(fesc);
        free(eesc);
    }

    sqlite3_finalize(stmt);
    return result;
}

int main(void) {
    sqlite3* db = NULL;
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
        fprintf(stderr, "Failed to open DB\n");
        return 1;
    }
    if (initDatabase(db) != 0) {
        fprintf(stderr, "Failed to init DB\n");
        sqlite3_close(db);
        return 1;
    }

    const char* tests[5] = {"alice", "bob", "carol", "dave", "nonexistent"};
    for (int i = 0; i < 5; ++i) {
        char* res = getUserByUsername(db, tests[i]);
        printf("Query for '%s': %s\n", tests[i], res ? res : "null");
        if (res) free(res);
    }

    sqlite3_close(db);
    return 0;
}