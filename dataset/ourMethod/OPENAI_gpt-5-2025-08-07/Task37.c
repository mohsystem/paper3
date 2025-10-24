#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sqlite3.h>

#define USERNAME_MAX 32
#define EMAIL_MAX 128
#define TS_MAX 32

struct User {
    int id;
    char username[USERNAME_MAX + 1];
    char email[EMAIL_MAX + 1];
    char created_at[TS_MAX + 1];
};

static int is_valid_username(const char* s) {
    if (s == NULL) return 0;
    size_t len = strlen(s);
    if (len < 3 || len > USERNAME_MAX) return 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)s[i];
        if (!(isalnum(c) || c == '_')) {
            return 0;
        }
    }
    return 1;
}

static void now_iso8601(char* out, size_t out_sz) {
    if (!out || out_sz == 0) return;
    time_t t = time(NULL);
    struct tm tmv;
#if defined(_WIN32)
    gmtime_s(&tmv, &t);
#else
    gmtime_r(&t, &tmv);
#endif
    if (strftime(out, out_sz, "%Y-%m-%dT%H:%M:%SZ", &tmv) == 0) {
        if (out_sz > 0) out[0] = '\0';
    }
}

static int init_db(sqlite3* db) {
    if (!db) return 0;
    const char* sql =
        "PRAGMA foreign_keys=ON;"
        "CREATE TABLE IF NOT EXISTS users ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  username TEXT NOT NULL UNIQUE CHECK(length(username) BETWEEN 3 AND 32),"
        "  email TEXT NOT NULL CHECK(length(email) BETWEEN 5 AND 128),"
        "  created_at TEXT NOT NULL"
        ");";
    char* errMsg = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        if (errMsg) sqlite3_free(errMsg);
        return 0;
    }
    return 1;
}

static int insert_user(sqlite3* db, const char* username, const char* email, const char* created_at) {
    if (!db || !username || !email || !created_at) return 0;
    if (!is_valid_username(username)) return 0;
    size_t email_len = strlen(email);
    if (email_len < 5 || email_len > EMAIL_MAX) return 0;

    const char* sql = "INSERT INTO users(username, email, created_at) VALUES (?1, ?2, ?3);";
    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK || !stmt) {
        if (stmt) sqlite3_finalize(stmt);
        return 0;
    }

    int ok = 0;
    do {
        if (sqlite3_bind_text(stmt, 1, username, (int)strlen(username), SQLITE_STATIC) != SQLITE_OK) break;
        if (sqlite3_bind_text(stmt, 2, email, (int)email_len, SQLITE_STATIC) != SQLITE_OK) break;
        if (sqlite3_bind_text(stmt, 3, created_at, (int)strlen(created_at), SQLITE_STATIC) != SQLITE_OK) break;
        if (sqlite3_step(stmt) != SQLITE_DONE) break;
        ok = 1;
    } while (0);

    sqlite3_finalize(stmt);
    return ok;
}

static int query_user(sqlite3* db, const char* username, struct User* out_user) {
    if (!db || !username || !out_user) return -1;
    if (!is_valid_username(username)) return 1; // treat invalid as not found

    const char* sql = "SELECT id, username, email, created_at FROM users WHERE username = ?1;";
    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK || !stmt) {
        if (stmt) sqlite3_finalize(stmt);
        return -1;
    }

    int result = 1; // not found by default
    do {
        if (sqlite3_bind_text(stmt, 1, username, (int)strlen(username), SQLITE_STATIC) != SQLITE_OK) {
            result = -1;
            break;
        }
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            // Safe copy with bounds
            out_user->id = sqlite3_column_int(stmt, 0);
            const unsigned char* u = sqlite3_column_text(stmt, 1);
            const unsigned char* e = sqlite3_column_text(stmt, 2);
            const unsigned char* c = sqlite3_column_text(stmt, 3);

            size_t ulen = u ? strlen((const char*)u) : 0;
            size_t elen = e ? strlen((const char*)e) : 0;
            size_t clen = c ? strlen((const char*)c) : 0;

            if (ulen > USERNAME_MAX) ulen = USERNAME_MAX;
            if (elen > EMAIL_MAX) elen = EMAIL_MAX;
            if (clen > TS_MAX) clen = TS_MAX;

            memcpy(out_user->username, u ? (const char*)u : "", ulen);
            out_user->username[ulen] = '\0';
            memcpy(out_user->email, e ? (const char*)e : "", elen);
            out_user->email[elen] = '\0';
            memcpy(out_user->created_at, c ? (const char*)c : "", clen);
            out_user->created_at[clen] = '\0';
            result = 0; // found
        } else if (rc == SQLITE_DONE) {
            result = 1; // not found
        } else {
            result = -1; // error
        }
    } while (0);

    sqlite3_finalize(stmt);
    return result;
}

int main(void) {
    sqlite3* db = NULL;
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
        printf("Failed to open SQLite database.\n");
        if (db) sqlite3_close(db);
        return 1;
    }

    if (!init_db(db)) {
        printf("Failed to initialize DB.\n");
        sqlite3_close(db);
        return 1;
    }

    char ts[TS_MAX + 1];
    now_iso8601(ts, sizeof(ts));

    if (!insert_user(db, "alice", "alice@example.com", ts)) { printf("Insert alice failed.\n"); }
    if (!insert_user(db, "bob", "bob@example.com", ts)) { printf("Insert bob failed.\n"); }
    if (!insert_user(db, "charlie", "charlie@example.com", ts)) { printf("Insert charlie failed.\n"); }

    const char* tests[5] = {
        "alice",
        "bob",
        "charlie",
        "doesnotexist",
        "bob' OR '1'='1" // invalid by validation
    };

    for (int i = 0; i < 5; i++) {
        const char* uname = tests[i];
        struct User u;
        int rc = query_user(db, uname, &u);
        if (rc == 0) {
            printf("Query username=\"%s\": FOUND -> id=%d, username=%s, email=%s, created_at=%s\n",
                   uname, u.id, u.username, u.email, u.created_at);
        } else if (rc == 1) {
            printf("Query username=\"%s\": NOT FOUND or INVALID INPUT\n", uname);
        } else {
            printf("Query username=\"%s\": ERROR\n", uname);
        }
    }

    sqlite3_close(db);
    return 0;
}