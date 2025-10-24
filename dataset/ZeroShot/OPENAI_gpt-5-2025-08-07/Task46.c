#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sqlite3.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

#define DB_PATH "task46_c.db"
#define ITERATIONS 120000
#define SALT_LENGTH 16
#define KEY_LENGTH 32

static int init_db(const char* db_path) {
    sqlite3* db = NULL;
    if (sqlite3_open(db_path, &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return 0;
    }
    const char* sql =
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "email TEXT NOT NULL UNIQUE,"
        "password_hash BLOB NOT NULL,"
        "salt BLOB NOT NULL,"
        "iterations INTEGER NOT NULL"
        ")";
    char* errmsg = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
    if (rc != SQLITE_OK) {
        if (errmsg) sqlite3_free(errmsg);
        sqlite3_close(db);
        return 0;
    }
    sqlite3_close(db);
    return 1;
}

static void trim(char* s) {
    if (!s) return;
    size_t len = strlen(s);
    size_t start = 0;
    while (start < len && (s[start] == ' ' || s[start] == '\t' || s[start] == '\r' || s[start] == '\n')) start++;
    size_t end = len;
    while (end > start && (s[end-1] == ' ' || s[end-1] == '\t' || s[end-1] == '\r' || s[end-1] == '\n')) end--;
    if (start > 0) memmove(s, s + start, end - start);
    s[end - start] = '\0';
}

static int valid_name(const char* name) {
    if (!name) return 0;
    char buf[256];
    snprintf(buf, sizeof(buf), "%s", name);
    buf[sizeof(buf)-1] = '\0';
    trim(buf);
    size_t len = strlen(buf);
    return len > 0 && len <= 100;
}

static int valid_email(const char* email) {
    if (!email) return 0;
    char buf[320];
    snprintf(buf, sizeof(buf), "%s", email);
    buf[sizeof(buf)-1] = '\0';
    trim(buf);
    // Simple validation: one '@', at least one dot after '@'
    int at_pos = -1;
    int dot_after = 0;
    for (int i = 0; buf[i]; i++) {
        if (buf[i] == '@') {
            if (at_pos != -1) return 0; // multiple '@'
            at_pos = i;
        } else if (buf[i] == '.' && at_pos != -1 && i > at_pos + 1) {
            dot_after = 1;
        }
    }
    return at_pos > 0 && dot_after;
}

static int valid_password(const char* password) {
    if (!password) return 0;
    size_t len = strlen(password);
    if (len < 8) return 0;
    int hasUpper=0, hasLower=0, hasDigit=0, hasSpecial=0;
    const char* specials = "!@#$%^&*()-_=+[]{}|;:'\",.<>?/`~\\";
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)password[i];
        if (isupper(c)) hasUpper = 1;
        else if (islower(c)) hasLower = 1;
        else if (isdigit(c)) hasDigit = 1;
        else {
            for (const char* p = specials; *p; ++p) {
                if (*p == c) { hasSpecial = 1; break; }
            }
        }
    }
    return hasUpper && hasLower && hasDigit && hasSpecial;
}

static int register_user(const char* db_path, const char* name, const char* email, const char* password) {
    if (!valid_name(name) || !valid_email(email) || !valid_password(password)) return 0;

    sqlite3* db = NULL;
    if (sqlite3_open(db_path, &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return 0;
    }

    unsigned char salt[SALT_LENGTH];
    if (RAND_bytes(salt, SALT_LENGTH) != 1) {
        sqlite3_close(db);
        return 0;
    }

    unsigned char key[KEY_LENGTH];
    if (!PKCS5_PBKDF2_HMAC(password, (int)strlen(password), salt, SALT_LENGTH, ITERATIONS, EVP_sha256(), KEY_LENGTH, key)) {
        sqlite3_close(db);
        return 0;
    }

    const char* sql = "INSERT INTO users(name, email, password_hash, salt, iterations) VALUES(?,?,?,?,?)";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_close(db);
        OPENSSL_cleanse(key, sizeof(key));
        OPENSSL_cleanse(salt, sizeof(salt));
        return 0;
    }

    char name_buf[256];
    char email_buf[320];
    snprintf(name_buf, sizeof(name_buf), "%s", name); name_buf[sizeof(name_buf)-1]='\0';
    snprintf(email_buf, sizeof(email_buf), "%s", email); email_buf[sizeof(email_buf)-1]='\0';
    trim(name_buf);
    trim(email_buf);
    for (char* p = email_buf; *p; ++p) *p = (char)tolower((unsigned char)*p);

    int ok = 0;
    if (sqlite3_bind_text(stmt, 1, name_buf, -1, SQLITE_TRANSIENT) == SQLITE_OK &&
        sqlite3_bind_text(stmt, 2, email_buf, -1, SQLITE_TRANSIENT) == SQLITE_OK &&
        sqlite3_bind_blob(stmt, 3, key, (int)sizeof(key), SQLITE_TRANSIENT) == SQLITE_OK &&
        sqlite3_bind_blob(stmt, 4, salt, (int)sizeof(salt), SQLITE_TRANSIENT) == SQLITE_OK &&
        sqlite3_bind_int(stmt, 5, ITERATIONS) == SQLITE_OK) {
        int rc = sqlite3_step(stmt);
        ok = (rc == SQLITE_DONE);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    OPENSSL_cleanse(key, sizeof(key));
    OPENSSL_cleanse(salt, sizeof(salt));

    return ok;
}

int main(void) {
    init_db(DB_PATH);
    printf("%d\n", register_user(DB_PATH, "Alice Smith", "alice@example.com", "Str0ng!Pass1"));
    printf("%d\n", register_user(DB_PATH, "Bob Johnson", "bob@example.com", "An0ther#Good2"));
    printf("%d\n", register_user(DB_PATH, "Charlie", "bad-email", "Weak")); // invalid
    printf("%d\n", register_user(DB_PATH, "Dana White", "dana.white@example.org", "Secur3$Pass4"));
    printf("%d\n", register_user(DB_PATH, "Eve Adams", "alice@example.com", "Str0ng!Pass1")); // duplicate
    return 0;
}