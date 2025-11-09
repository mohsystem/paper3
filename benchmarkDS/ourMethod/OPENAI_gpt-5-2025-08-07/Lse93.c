// Secure user insertion with password hashing using libsodium and SQLite3.
// Build example (Linux/macOS):
//   cc -Wall -Wextra -O2 secure_insert.c -lsqlite3 -lsodium -o secure_insert
// Run:
//   ./secure_insert

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#include <sqlite3.h>
#include <sodium.h>

#define USERNAME_MIN_LEN 3
#define USERNAME_MAX_LEN 64
#define PASSWORD_MIN_LEN 12
#define PASSWORD_MAX_LEN 256

// Application error codes
enum {
    APP_OK = 0,
    APP_ERR_INVALID_INPUT = 1,
    APP_ERR_WEAK_PASSWORD = 2,
    APP_ERR_DB = 3,
    APP_ERR_DUPLICATE = 4,
    APP_ERR_INTERNAL = 5
};

// Validate username: length and allowed characters (allow-list)
int validate_username(const char *username) {
    if (username == NULL) return APP_ERR_INVALID_INPUT;

    size_t len = strnlen(username, USERNAME_MAX_LEN + 1);
    if (len < USERNAME_MIN_LEN || len > USERNAME_MAX_LEN) {
        return APP_ERR_INVALID_INPUT;
    }

    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)username[i];
        // Allowed: A-Z a-z 0-9 _ . -
        if (!((c >= 'a' && c <= 'z') ||
              (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') ||
              c == '_' || c == '.' || c == '-')) {
            return APP_ERR_INVALID_INPUT;
        }
    }
    return APP_OK;
}

// Validate password policy: min length, and presence of upper, lower, digit, special
int validate_password_policy(const char *password) {
    if (password == NULL) return APP_ERR_INVALID_INPUT;

    size_t len = strnlen(password, PASSWORD_MAX_LEN + 1);
    if (len < PASSWORD_MIN_LEN || len > PASSWORD_MAX_LEN) {
        return APP_ERR_WEAK_PASSWORD;
    }

    bool has_lower = false, has_upper = false, has_digit = false, has_special = false;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)password[i];
        if (c >= 'a' && c <= 'z') has_lower = true;
        else if (c >= 'A' && c <= 'Z') has_upper = true;
        else if (c >= '0' && c <= '9') has_digit = true;
        else if (c >= 33 && c <= 126) has_special = true; // printable specials
        // Disallow control characters and spaces to avoid ambiguity
        else return APP_ERR_WEAK_PASSWORD;
    }

    if (!(has_lower && has_upper && has_digit && has_special)) {
        return APP_ERR_WEAK_PASSWORD;
    }
    return APP_OK;
}

// Hash password with Argon2id (libsodium crypto_pwhash_str), using strong parameters
int hash_password(const char *password, char *out_hash, size_t out_size) {
    if (!password || !out_hash || out_size < crypto_pwhash_STRBYTES) {
        return APP_ERR_INTERNAL;
    }

    // Derive a secure hash using Argon2id with SENSITIVE limits.
    // libsodium generates a random salt internally and encodes parameters + salt in out_hash.
    if (crypto_pwhash_str(
            out_hash,
            password,
            (unsigned long long)strlen(password),
            crypto_pwhash_OPSLIMIT_SENSITIVE,
            crypto_pwhash_MEMLIMIT_SENSITIVE) != 0) {
        // Out of memory or internal failure
        return APP_ERR_INTERNAL;
    }

    return APP_OK;
}

// Setup SQLite database and create users table
int setup_database(sqlite3 **out_db) {
    if (!out_db) return APP_ERR_INTERNAL;
    *out_db = NULL;

    // In-memory DB for demonstration; replace ":memory:" with a secure file path as needed.
    int rc = sqlite3_open_v2(":memory:", out_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (rc != SQLITE_OK) {
        if (*out_db) sqlite3_close(*out_db);
        *out_db = NULL;
        return APP_ERR_DB;
    }

    const char *create_sql =
        "PRAGMA foreign_keys = ON; "
        "CREATE TABLE IF NOT EXISTS users ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  username TEXT NOT NULL UNIQUE,"
        "  password_hash TEXT NOT NULL,"
        "  created_at TEXT NOT NULL"
        ");";

    char *errmsg = NULL;
    rc = sqlite3_exec(*out_db, create_sql, NULL, NULL, &errmsg);
    if (rc != SQLITE_OK) {
        if (errmsg) {
            // Do not print attacker-controlled data; sqlite error messages are internal.
            sqlite3_free(errmsg);
        }
        sqlite3_close(*out_db);
        *out_db = NULL;
        return APP_ERR_DB;
    }

    return APP_OK;
}

// Insert user with hashed password using a parameterized statement
int insert_user(sqlite3 *db, const char *username, const char *password) {
    if (!db || !username || !password) return APP_ERR_INVALID_INPUT;

    int rc = validate_username(username);
    if (rc != APP_OK) return rc;

    rc = validate_password_policy(password);
    if (rc != APP_OK) return rc;

    char hash[crypto_pwhash_STRBYTES];
    rc = hash_password(password, hash, sizeof(hash));
    if (rc != APP_OK) {
        sodium_memzero(hash, sizeof(hash));
        return rc;
    }

    const char *insert_sql =
        "INSERT INTO users (username, password_hash, created_at) "
        "VALUES (?, ?, strftime('%Y-%m-%dT%H:%M:%SZ','now'));";

    sqlite3_stmt *stmt = NULL;
    int s_rc = sqlite3_prepare_v2(db, insert_sql, -1, &stmt, NULL);
    if (s_rc != SQLITE_OK) {
        sodium_memzero(hash, sizeof(hash));
        return APP_ERR_DB;
    }

    // Bind parameters safely; never concatenate user input into SQL.
    s_rc = sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    if (s_rc == SQLITE_OK) {
        s_rc = sqlite3_bind_text(stmt, 2, hash, -1, SQLITE_TRANSIENT);
    }

    // Clear the in-memory hash as soon as it's no longer needed
    sodium_memzero(hash, sizeof(hash));

    if (s_rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return APP_ERR_DB;
    }

    s_rc = sqlite3_step(stmt);
    int final_rc = APP_OK;
    if (s_rc != SQLITE_DONE) {
        if (s_rc == SQLITE_CONSTRAINT || sqlite3_errcode(db) == SQLITE_CONSTRAINT) {
            final_rc = APP_ERR_DUPLICATE; // likely UNIQUE(username) violation
        } else {
            final_rc = APP_ERR_DB;
        }
    }

    sqlite3_finalize(stmt);
    return final_rc;
}

// Helper: count users in DB (no user input used)
int count_users(sqlite3 *db, int *out_count) {
    if (!db || !out_count) return APP_ERR_INTERNAL;
    *out_count = 0;

    const char *sql = "SELECT COUNT(*) FROM users;";
    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return APP_ERR_DB;

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        *out_count = sqlite3_column_int(stmt, 0);
        rc = SQLITE_DONE;
    }
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE) ? APP_OK : APP_ERR_DB;
}

int main(void) {
    // Initialize libsodium once
    if (sodium_init() < 0) {
        // Failed to initialize the library
        fprintf(stderr, "Initialization failed.\n");
        return EXIT_FAILURE;
    }

    sqlite3 *db = NULL;
    if (setup_database(&db) != APP_OK) {
        fprintf(stderr, "Database setup failed.\n");
        return EXIT_FAILURE;
    }

    // Five test cases
    struct TestCase {
        const char *username;
        const char *password;
        const char *desc;
    } tests[5] = {
        { "alice",   "Str0ng!Passw0rd",        "Valid user 1" },
        { "bob-01",  "Another$trongP4ss",      "Valid user 2" },
        { "alice",   "DifferentP@ssw0rd!",     "Duplicate username" },
        { "charlie", "short",                  "Weak password (too short)" },
        { "bad user","YetAn0ther$trongPass",   "Invalid username (space)" }
    };

    for (int i = 0; i < 5; i++) {
        int rc = insert_user(db, tests[i].username, tests[i].password);
        // Use constant format strings, avoid echoing sensitive inputs.
        if (rc == APP_OK) {
            printf("Test %d [%s]: OK\n", i + 1, tests[i].desc);
        } else if (rc == APP_ERR_DUPLICATE) {
            printf("Test %d [%s]: FAILED (duplicate username)\n", i + 1, tests[i].desc);
        } else if (rc == APP_ERR_WEAK_PASSWORD) {
            printf("Test %d [%s]: FAILED (weak password)\n", i + 1, tests[i].desc);
        } else if (rc == APP_ERR_INVALID_INPUT) {
            printf("Test %d [%s]: FAILED (invalid input)\n", i + 1, tests[i].desc);
        } else {
            printf("Test %d [%s]: FAILED (internal/db error)\n", i + 1, tests[i].desc);
        }
    }

    int total = 0;
    if (count_users(db, &total) == APP_OK) {
        printf("Total users inserted: %d\n", total);
    } else {
        printf("Could not count users.\n");
    }

    sqlite3_close(db);
    return EXIT_SUCCESS;
}