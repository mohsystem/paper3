#include <sqlite3.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define USERNAME_MAX 64
#define FULLNAME_MAX 128
#define EMAIL_MAX 256
#define CREATED_AT_MAX 32

struct Customer {
    int id;
    char username[USERNAME_MAX + 1];
    char fullname[FULLNAME_MAX + 1];
    char email[EMAIL_MAX + 1];
    char created_at[CREATED_AT_MAX + 1];
    int found; /* 1 if found, 0 otherwise */
};

static int is_valid_username(const char* u) {
    if (u == NULL) return 0;
    size_t len = strlen(u);
    if (len == 0 || len > USERNAME_MAX) return 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)u[i];
        if (!(isalnum(c) || c == '_' || c == '.' || c == '-')) return 0;
    }
    return 1;
}

static int exec_sql(sqlite3* db, const char* sql) {
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

static int open_db(const char* path, sqlite3** outdb) {
    if (!outdb) return SQLITE_MISUSE;
    *outdb = NULL;
    int rc = sqlite3_open(path, outdb);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(*outdb));
        if (*outdb) {
            sqlite3_close(*outdb);
            *outdb = NULL;
        }
    }
    return rc;
}

static int ensure_schema_and_seed(sqlite3* db) {
    const char* schema =
        "PRAGMA foreign_keys=ON;"
        "CREATE TABLE IF NOT EXISTS customer ("
        "  id INTEGER PRIMARY KEY,"
        "  customerusername TEXT UNIQUE NOT NULL CHECK (length(customerusername) <= 64),"
        "  fullname TEXT NOT NULL,"
        "  email TEXT NOT NULL,"
        "  created_at TEXT NOT NULL"
        ");";
    int rc = exec_sql(db, schema);
    if (rc != SQLITE_OK) return rc;

    const char* seed =
        "INSERT OR IGNORE INTO customer (id, customerusername, fullname, email, created_at) VALUES "
        "(1, 'alice',   'Alice Anderson',  'alice@example.com',   '2023-01-01T10:00:00Z'),"
        "(2, 'bob',     'Bob Brown',       'bob@example.com',     '2023-02-14T12:30:00Z'),"
        "(3, 'charlie', 'Charlie Clark',   'charlie@example.com', '2023-03-20T08:45:00Z'),"
        "(4, 'dora',    'Dora Davis',      'dora@example.com',    '2023-04-05T16:20:00Z');";
    rc = exec_sql(db, seed);
    return rc;
}

static void copy_col_text(char* dst, size_t dstsz, const unsigned char* src, int srclen) {
    if (!dst || dstsz == 0) return;
    if (!src || srclen <= 0) {
        dst[0] = '\0';
        return;
    }
    size_t n = (size_t)srclen;
    if (n >= dstsz) n = dstsz - 1;
    memcpy(dst, src, n);
    dst[n] = '\0';
}

static struct Customer fetch_customer(sqlite3* db, const char* username) {
    struct Customer c;
    memset(&c, 0, sizeof(c));
    if (!db) {
        fprintf(stderr, "Database handle is null.\n");
        return c;
    }
    if (!is_valid_username(username)) {
        fprintf(stderr, "Invalid username format. Allowed: alphanumeric, _, -, . length<=64\n");
        return c;
    }

    const char* sql =
        "SELECT id, customerusername, fullname, email, created_at "
        "FROM customer WHERE customerusername = ?1;";
    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return c;
    }

    rc = sqlite3_bind_text(stmt, 1, username, (int)strlen(username), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to bind parameter: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return c;
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        c.id = sqlite3_column_int(stmt, 0);

        const unsigned char* u = sqlite3_column_text(stmt, 1);
        int ulen = sqlite3_column_bytes(stmt, 1);
        copy_col_text(c.username, sizeof(c.username), u, ulen);

        const unsigned char* f = sqlite3_column_text(stmt, 2);
        int flen = sqlite3_column_bytes(stmt, 2);
        copy_col_text(c.fullname, sizeof(c.fullname), f, flen);

        const unsigned char* e = sqlite3_column_text(stmt, 3);
        int elen = sqlite3_column_bytes(stmt, 3);
        copy_col_text(c.email, sizeof(c.email), e, elen);

        const unsigned char* t = sqlite3_column_text(stmt, 4);
        int tlen = sqlite3_column_bytes(stmt, 4);
        copy_col_text(c.created_at, sizeof(c.created_at), t, tlen);

        c.found = 1;
    } else if (rc != SQLITE_DONE) {
        fprintf(stderr, "Query failed: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    return c;
}

int main(int argc, char* argv[]) {
    sqlite3* db = NULL;
    if (open_db("customers.db", &db) != SQLITE_OK || !db) {
        return 1;
    }
    if (ensure_schema_and_seed(db) != SQLITE_OK) {
        sqlite3_close(db);
        return 1;
    }

    const char* tests[5];
    if (argc >= 2) {
        tests[0] = argv[1];
        tests[1] = "alice";
        tests[2] = "bob";
        tests[3] = "charlie";
        tests[4] = "nonexistent";
    } else {
        tests[0] = "alice";
        tests[1] = "bob";
        tests[2] = "charlie";
        tests[3] = "dora";
        tests[4] = "nonexistent";
    }

    for (int i = 0; i < 5; i++) {
        struct Customer c = fetch_customer(db, tests[i]);
        if (c.found) {
            printf("FOUND: id=%d username=%s fullname=%s email=%s created_at=%s\n",
                   c.id, c.username, c.fullname, c.email, c.created_at);
        } else {
            printf("NOT FOUND: username=%s\n", tests[i]);
        }
    }

    sqlite3_close(db);
    return 0;
}