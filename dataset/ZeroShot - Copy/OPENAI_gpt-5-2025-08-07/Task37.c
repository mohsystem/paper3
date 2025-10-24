#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

// Return a newly-allocated JSON-escaped string wrapped in quotes. Caller must free.
static char* json_escape(const char* s) {
    if (!s) {
        char* n = (char*)malloc(5);
        if (n) strcpy(n, "null");
        return n;
    }
    size_t len = strlen(s);
    // Worst case every char needs escaping (e.g., \uXXXX) -> 6x + quotes + null
    size_t cap = len * 6 + 3;
    char* out = (char*)malloc(cap);
    if (!out) return NULL;
    char* p = out;
    *p++ = '"';
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)s[i];
        switch (c) {
            case '"':  memcpy(p, "\\\"", 2); p += 2; break;
            case '\\': memcpy(p, "\\\\", 2); p += 2; break;
            case '\b': memcpy(p, "\\b", 2);  p += 2; break;
            case '\f': memcpy(p, "\\f", 2);  p += 2; break;
            case '\n': memcpy(p, "\\n", 2);  p += 2; break;
            case '\r': memcpy(p, "\\r", 2);  p += 2; break;
            case '\t': memcpy(p, "\\t", 2);  p += 2; break;
            default:
                if (c < 0x20) {
                    // \u00XX
                    int n = snprintf(p, 7, "\\u%04x", c);
                    p += (n > 0 ? (size_t)n : 0);
                } else {
                    *p++ = (char)c;
                }
        }
    }
    *p++ = '"';
    *p = '\0';
    return out;
}

// Securely retrieve user info with parameterized query. Returns malloc'ed string or "NOT_FOUND" cloned string; caller must free.
char* get_user_info(sqlite3* db, const char* username) {
    if (!db || !username || username[0] == '\0') {
        char* nf = (char*)malloc(10);
        if (nf) strcpy(nf, "NOT_FOUND");
        return nf;
    }
    sqlite3_busy_timeout(db, 5000);
    const char* sql = "SELECT username, full_name, email, created_at FROM users WHERE username = ?1";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        char* nf = (char*)malloc(10);
        if (nf) strcpy(nf, "NOT_FOUND");
        return nf;
    }
    if (sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT) != SQLITE_OK) {
        sqlite3_finalize(stmt);
        char* nf = (char*)malloc(10);
        if (nf) strcpy(nf, "NOT_FOUND");
        return nf;
    }

    char* result = NULL;
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        const char* uname = (const char*)sqlite3_column_text(stmt, 0);
        const char* fname = (const char*)sqlite3_column_text(stmt, 1);
        const char* email = (const char*)sqlite3_column_text(stmt, 2);
        const char* ctime = (const char*)sqlite3_column_text(stmt, 3);

        char* j_uname = json_escape(uname);
        char* j_fname = json_escape(fname);
        char* j_email = json_escape(email);
        char* j_ctime = json_escape(ctime);
        if (!j_uname || !j_fname || !j_email || !j_ctime) {
            free(j_uname); free(j_fname); free(j_email); free(j_ctime);
            sqlite3_finalize(stmt);
            char* nf = (char*)malloc(10);
            if (nf) strcpy(nf, "NOT_FOUND");
            return nf;
        }

        const char* prefix = "{\"username\":";
        const char* mid1 = ",\"full_name\":";
        const char* mid2 = ",\"email\":";
        const char* mid3 = ",\"created_at\":";
        const char* suffix = "}";

        size_t total =
            strlen(prefix) + strlen(j_uname) +
            strlen(mid1) + strlen(j_fname) +
            strlen(mid2) + strlen(j_email) +
            strlen(mid3) + strlen(j_ctime) +
            strlen(suffix) + 1;

        result = (char*)malloc(total);
        if (result) {
            // Build JSON
            result[0] = '\0';
            strcat(result, prefix);
            strcat(result, j_uname);
            strcat(result, mid1);
            strcat(result, j_fname);
            strcat(result, mid2);
            strcat(result, j_email);
            strcat(result, mid3);
            strcat(result, j_ctime);
            strcat(result, suffix);
        } else {
            // allocation failed
            result = (char*)malloc(10);
            if (result) strcpy(result, "NOT_FOUND");
        }

        free(j_uname); free(j_fname); free(j_email); free(j_ctime);
    } else {
        result = (char*)malloc(10);
        if (result) strcpy(result, "NOT_FOUND");
    }

    sqlite3_finalize(stmt);
    return result;
}

static int exec_or_fail(sqlite3* db, const char* sql) {
    char* err = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        if (err) {
            fprintf(stderr, "SQL error: %s\n", err);
            sqlite3_free(err);
        }
    }
    return rc;
}

int main(void) {
    sqlite3* db = NULL;
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
        fprintf(stderr, "Cannot open in-memory DB\n");
        return 1;
    }

    if (exec_or_fail(db, "PRAGMA foreign_keys = ON") != SQLITE_OK) {
        sqlite3_close(db);
        return 1;
    }

    if (exec_or_fail(db,
        "CREATE TABLE users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT UNIQUE NOT NULL,"
        "full_name TEXT NOT NULL,"
        "email TEXT NOT NULL,"
        "created_at TEXT NOT NULL)"
    ) != SQLITE_OK) {
        sqlite3_close(db);
        return 1;
    }

    const char* ins = "INSERT INTO users (username, full_name, email, created_at) VALUES (?, ?, ?, ?)";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, ins, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_close(db);
        return 1;
    }

    struct Row { const char* u; const char* f; const char* e; const char* c; } rows[] = {
        {"alice", "Alice Anderson", "alice@example.com", "2023-01-01T00:00:00Z"},
        {"bob", "Bob Brown", "bob@example.com", "2023-02-02T00:00:00Z"},
        {"carol", "Carol Clark", "carol@example.com", "2023-03-03T00:00:00Z"},
        {"dave", "Dave Davis", "dave@example.com", "2023-04-04T00:00:00Z"},
    };

    for (size_t i = 0; i < sizeof(rows)/sizeof(rows[0]); i++) {
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        sqlite3_bind_text(stmt, 1, rows[i].u, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, rows[i].f, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, rows[i].e, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, rows[i].c, -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            fprintf(stderr, "Insert failed\n");
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return 1;
        }
    }
    sqlite3_finalize(stmt);

    const char* tests[] = {"alice", "bob", "doesnotexist", "carol", "alice' OR '1'='1"};
    for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); i++) {
        char* res = get_user_info(db, tests[i]);
        if (res) {
            printf("Query for '%s': %s\n", tests[i], res);
            free(res);
        } else {
            printf("Query for '%s': (null)\n", tests[i]);
        }
    }

    sqlite3_close(db);
    return 0;
}