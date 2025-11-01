
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <sqlite3.h>

/* Rules#34: Define validation constants */
#define MIN_USERNAME_LEN 3
#define MAX_USERNAME_LEN 30
#define MIN_PASSWORD_LEN 8
#define MAX_PASSWORD_LEN 128
#define MAX_NAME_LEN 50
#define MAX_EMAIL_LEN 254
#define PBKDF2_ITERATIONS 210000
#define SALT_LENGTH 16
#define HASH_LENGTH 32

typedef struct {
    int success;
    char message[64];
} RegistrationResult;

/* Rules#2: Input validation with regex */
static int validate_username(const char* username) {
    if (!username) return 0;
    size_t len = strnlen(username, MAX_USERNAME_LEN + 1);
    if (len < MIN_USERNAME_LEN || len > MAX_USERNAME_LEN) return 0;
    
    regex_t regex;
    int result = regcomp(&regex, "^[a-zA-Z0-9_]+$", REG_EXTENDED);
    if (result != 0) return 0;
    
    result = regexec(&regex, username, 0, NULL, 0);
    regfree(&regex);
    return result == 0;
}

static int validate_password(const char* password) {
    if (!password) return 0;
    size_t len = strnlen(password, MAX_PASSWORD_LEN + 1);
    return len >= MIN_PASSWORD_LEN && len <= MAX_PASSWORD_LEN;
}

static int validate_name(const char* name) {
    if (!name) return 0;
    size_t len = strnlen(name, MAX_NAME_LEN + 1);
    if (len == 0 || len > MAX_NAME_LEN) return 0;
    
    regex_t regex;
    int result = regcomp(&regex, "^[a-zA-Z \\\\'\\\\-]+$", REG_EXTENDED);
    if (result != 0) return 0;
    
    result = regexec(&regex, name, 0, NULL, 0);
    regfree(&regex);
    return result == 0;
}

static int validate_email(const char* email) {
    if (!email) return 0;
    size_t len = strnlen(email, MAX_EMAIL_LEN + 1);
    if (len == 0 || len > MAX_EMAIL_LEN) return 0;
    
    regex_t regex;
    int result = regcomp(&regex, "^[a-zA-Z0-9._%+\\\\-]+@[a-zA-Z0-9.\\\\-]+\\\\.[a-zA-Z]{2,}$", REG_EXTENDED);
    if (result != 0) return 0;
    
    result = regexec(&regex, email, 0, NULL, 0);
    regfree(&regex);
    return result == 0;
}

/* Rules#5: Generate salt with CSPRNG */
static int generate_salt(unsigned char* salt, size_t salt_len) {
    if (!salt || salt_len != SALT_LENGTH) return 0;
    return RAND_bytes(salt, salt_len) == 1;
}

/* Rules#5: Hash password with PBKDF2-HMAC-SHA-256 with 210000 iterations */
static int hash_password(const char* password, const unsigned char* salt,
                         unsigned char* hash_out) {
    if (!password || !salt || !hash_out) return 0;
    
    /* Rules#5: Use PBKDF2-HMAC-SHA-256 */
    int result = PKCS5_PBKDF2_HMAC(password, strlen(password),
                                    salt, SALT_LENGTH,
                                    PBKDF2_ITERATIONS,
                                    EVP_sha256(),
                                    HASH_LENGTH, hash_out);
    return result == 1;
}

/* Convert bytes to hex string */
static void bytes_to_hex(const unsigned char* bytes, size_t len, char* hex_out) {
    for (size_t i = 0; i < len; i++) {
        snprintf(hex_out + (i * 2), 3, "%02x", bytes[i]);
    }
    hex_out[len * 2] = '\\0';
}

/* Rules#3: Get DB path from environment */
static const char* get_db_path(void) {
    const char* path = getenv("DB_PATH");
    return path ? path : "users.db";
}

/* Rules#34-42: Main registration function with full validation */
RegistrationResult register_user(const char* username, const char* password,
                                  const char* first_name, const char* last_name,
                                  const char* email) {
    RegistrationResult result = {0, "Registration failed"};
    sqlite3* db = NULL;
    sqlite3_stmt* check_stmt = NULL;
    sqlite3_stmt* insert_stmt = NULL;
    unsigned char salt[SALT_LENGTH];
    unsigned char hash[HASH_LENGTH];
    char salt_hex[SALT_LENGTH * 2 + 1];
    char hash_hex[HASH_LENGTH * 2 + 1];
    
    /* Rules#44: Initialize pointers */
    memset(salt, 0, sizeof(salt));
    memset(hash, 0, sizeof(hash));
    memset(salt_hex, 0, sizeof(salt_hex));
    memset(hash_hex, 0, sizeof(hash_hex));
    
    /* Rules#34: Validate all inputs at trust boundary */
    if (!validate_username(username)) goto cleanup;
    if (!validate_password(password)) goto cleanup;
    if (!validate_name(first_name)) goto cleanup;
    if (!validate_name(last_name)) goto cleanup;
    if (!validate_email(email)) goto cleanup;
    
    /* Rules#3: Get DB path from environment */
    const char* db_path = get_db_path();
    if (sqlite3_open(db_path, &db) != SQLITE_OK) goto cleanup;
    
    /* Rules#42: Use parameterized query to prevent SQL injection */
    const char* check_sql = "SELECT COUNT(*) FROM users WHERE username = ?";
    if (sqlite3_prepare_v2(db, check_sql, -1, &check_stmt, NULL) != SQLITE_OK) {
        goto cleanup;
    }
    
    if (sqlite3_bind_text(check_stmt, 1, username, -1, SQLITE_STATIC) != SQLITE_OK) {
        goto cleanup;
    }
    
    int count = 0;
    if (sqlite3_step(check_stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(check_stmt, 0);
    }
    
    if (count > 0) {
        /* Rules#11: Generic error - no info leaked */
        goto cleanup;
    }
    
    /* Rules#5: Generate salt with CSPRNG */
    if (!generate_salt(salt, SALT_LENGTH)) goto cleanup;
    
    /* Rules#5: Hash password */
    if (!hash_password(password, salt, hash)) goto cleanup;
    
    /* Convert to hex */
    bytes_to_hex(salt, SALT_LENGTH, salt_hex);
    bytes_to_hex(hash, HASH_LENGTH, hash_hex);
    
    /* Rules#42: Parameterized insert prevents SQL injection */
    const char* insert_sql = "INSERT INTO users (username, password_hash, salt, first_name, last_name, email) VALUES (?, ?, ?, ?, ?, ?)";
    if (sqlite3_prepare_v2(db, insert_sql, -1, &insert_stmt, NULL) != SQLITE_OK) {
        goto cleanup;
    }
    
    sqlite3_bind_text(insert_stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(insert_stmt, 2, hash_hex, -1, SQLITE_STATIC);
    sqlite3_bind_text(insert_stmt, 3, salt_hex, -1, SQLITE_STATIC);
    sqlite3_bind_text(insert_stmt, 4, first_name, -1, SQLITE_STATIC);
    sqlite3_bind_text(insert_stmt, 5, last_name, -1, SQLITE_STATIC);
    sqlite3_bind_text(insert_stmt, 6, email, -1, SQLITE_STATIC);
    
    if (sqlite3_step(insert_stmt) == SQLITE_DONE) {
        result.success = 1;
        strncpy(result.message, "Registration succeeded", sizeof(result.message) - 1);
    }
    
cleanup:
    /* Rules#9: Clear sensitive data before cleanup */
    memset_s(salt, sizeof(salt), 0, sizeof(salt));
    memset_s(hash, sizeof(hash), 0, sizeof(hash));
    
    /* Rules#7: Cleanup in reverse order */
    if (check_stmt) sqlite3_finalize(check_stmt);
    if (insert_stmt) sqlite3_finalize(insert_stmt);
    if (db) sqlite3_close(db);
    
    return result;
}

void init_database(void) {
    sqlite3* db = NULL;
    const char* db_path = get_db_path();
    
    if (sqlite3_open(db_path, &db) == SQLITE_OK) {
        const char* create_table = 
            "CREATE TABLE IF NOT EXISTS users ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "username TEXT UNIQUE NOT NULL,"
            "password_hash TEXT NOT NULL,"
            "salt TEXT NOT NULL,"
            "first_name TEXT NOT NULL,"
            "last_name TEXT NOT NULL,"
            "email TEXT NOT NULL"
            ")";
        sqlite3_exec(db, create_table, NULL, NULL, NULL);
        sqlite3_close(db);
    }
}

int main(void) {
    init_database();
    
    /* Test case 1: Valid registration */
    RegistrationResult r1 = register_user("john_doe", "SecurePass123!", "John", "Doe", "john@example.com");
    printf("Test 1 - Valid registration: %s\\n", r1.message);
    
    /* Test case 2: Duplicate username */
    RegistrationResult r2 = register_user("john_doe", "AnotherPass456!", "Jane", "Smith", "jane@example.com");
    printf("Test 2 - Duplicate username: %s\\n", r2.message);
    
    /* Test case 3: Invalid username (too short) */
    RegistrationResult r3 = register_user("ab", "ValidPass789!", "Bob", "Johnson", "bob@example.com");
    printf("Test 3 - Invalid username: %s\\n", r3.message);
    
    /* Test case 4: Invalid password (too short) */
    RegistrationResult r4 = register_user("alice_w", "short", "Alice", "Williams", "alice@example.com");
    printf("Test 4 - Invalid password: %s\\n", r4.message);
    
    /* Test case 5: Invalid email format */
    RegistrationResult r5 = register_user("charlie_b", "ValidPass999!", "Charlie", "Brown", "invalid-email");
    printf("Test 5 - Invalid email: %s\\n", r5.message);
    
    return 0;
}
