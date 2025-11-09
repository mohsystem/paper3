
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

/* Maximum input lengths to prevent buffer overflows */
#define MAX_USERNAME_LEN 64
#define MAX_PASSWORD_LEN 128
#define SALT_LEN 16
#define HASH_LEN 32
#define ENCODED_HASH_LEN ((HASH_LEN + SALT_LEN) * 2 + 1)

/* Secure memory clearing - prevents compiler optimization from removing memset */
static void secure_zero(void *ptr, size_t len) {
    if (ptr == NULL || len == 0) return;
    volatile unsigned char *p = ptr;
    while (len--) *p++ = 0;
}

/* Hash password using PBKDF2-HMAC-SHA256 with random salt */
static int hash_password(const char *password, char *output, size_t output_size) {
    unsigned char salt[SALT_LEN];
    unsigned char hash[HASH_LEN];
    int i;
    
    /* Validate inputs - Rules#2: Input validation */
    if (password == NULL || output == NULL || output_size < ENCODED_HASH_LEN) {
        return -1;
    }
    
    /* Generate random salt using CSPRNG - Rules#5: Cryptography */
    if (RAND_bytes(salt, SALT_LEN) != 1) {
        return -1;
    }
    
    /* Derive key using PBKDF2-HMAC-SHA256 with 210000 iterations - Rules#5 */
    if (PKCS5_PBKDF2_HMAC(password, (int)strlen(password),
                          salt, SALT_LEN,
                          210000,
                          EVP_sha256(),
                          HASH_LEN, hash) != 1) {
        secure_zero(salt, SALT_LEN);
        return -1;
    }
    
    /* Encode salt and hash as hex string for storage */
    for (i = 0; i < SALT_LEN; i++) {
        snprintf(output + (i * 2), 3, "%02x", salt[i]);
    }
    for (i = 0; i < HASH_LEN; i++) {
        snprintf(output + (SALT_LEN * 2) + (i * 2), 3, "%02x", hash[i]);
    }
    output[ENCODED_HASH_LEN - 1] = '\\0';
    
    /* Clear sensitive data - Rules#9: Sensitive data handling */
    secure_zero(salt, SALT_LEN);
    secure_zero(hash, HASH_LEN);
    
    return 0;
}

/* Validate username: alphanumeric only, max length - Rules#2: Input validation */
static int validate_username(const char *username) {
    size_t len;
    size_t i;
    
    if (username == NULL) return -1;
    
    len = strnlen(username, MAX_USERNAME_LEN + 1);
    if (len == 0 || len > MAX_USERNAME_LEN) {
        return -1;
    }
    
    /* Allow list: alphanumeric and underscore only - Rules#25: Use allow lists */
    for (i = 0; i < len; i++) {
        if (!((username[i] >= 'a' && username[i] <= 'z') ||
              (username[i] >= 'A' && username[i] <= 'Z') ||
              (username[i] >= '0' && username[i] <= '9') ||
              username[i] == '_')) {
            return -1;
        }
    }
    
    return 0;
}

/* Validate password: minimum length, max length - Rules#2: Input validation */
static int validate_password(const char *password) {
    size_t len;
    
    if (password == NULL) return -1;
    
    len = strnlen(password, MAX_PASSWORD_LEN + 1);
    if (len < 8 || len > MAX_PASSWORD_LEN) {
        return -1;
    }
    
    return 0;
}

/* Insert user into database using parameterized query - Rules#32 */
static int insert_user(const char *username, const char *password_hash) {
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int rc;
    int result = -1;
    
    /* Validate inputs */
    if (username == NULL || password_hash == NULL) {
        return -1;
    }
    
    /* Open database with restrictive flags */
    rc = sqlite3_open_v2("users.db", &db, 
                         SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database\\n"); /* Rules#11: Generic error */
        goto cleanup;
    }
    
    /* Create table if not exists */
    rc = sqlite3_exec(db, 
                      "CREATE TABLE IF NOT EXISTS users ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "username TEXT UNIQUE NOT NULL, "
                      "password_hash TEXT NOT NULL)",
                      NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to create table\\n");
        goto cleanup;
    }
    
    /* Prepare parameterized statement - Rules#32: Use parameterized queries */
    rc = sqlite3_prepare_v2(db,
                           "INSERT INTO users (username, password_hash) VALUES (?, ?)",
                           -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement\\n");
        goto cleanup;
    }
    
    /* Bind parameters - prevents SQL injection */
    rc = sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to bind username\\n");
        goto cleanup;
    }
    
    rc = sqlite3_bind_text(stmt, 2, password_hash, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to bind password hash\\n");
        goto cleanup;
    }
    
    /* Execute statement */
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
        printf("User registered successfully\\n");
        result = 0;
    } else if (rc == SQLITE_CONSTRAINT) {
        fprintf(stderr, "Username already exists\\n"); /* Rules#11: Generic error */
    } else {
        fprintf(stderr, "Failed to insert user\\n");
    }
    
cleanup:
    /* Clean up resources - Rules#8: Memory safety */
    if (stmt != NULL) {
        sqlite3_finalize(stmt);
    }
    if (db != NULL) {
        sqlite3_close(db);
    }
    
    return result;
}

/* Main registration function */
int register_user(const char *username, const char *password) {
    char password_hash[ENCODED_HASH_LEN];
    int result = -1;
    
    /* Validate inputs - Rules#2: Input validation */
    if (validate_username(username) != 0) {
        fprintf(stderr, "Invalid username\\n");
        return -1;
    }
    
    if (validate_password(password) != 0) {
        fprintf(stderr, "Invalid password\\n");
        return -1;
    }
    
    /* Hash password - Rules#5: Cryptography */
    if (hash_password(password, password_hash, sizeof(password_hash)) != 0) {
        fprintf(stderr, "Failed to hash password\\n");
        return -1;
    }
    
    /* Insert into database */
    result = insert_user(username, password_hash);
    
    /* Clear sensitive data - Rules#9: Sensitive data handling */
    secure_zero(password_hash, sizeof(password_hash));
    
    return result;
}

int main(void) {
    /* Test cases - Rules#10: Logging never logs secrets */
    printf("Test Case 1: Valid registration\\n");
    register_user("alice", "SecurePass123!");
    
    printf("\\nTest Case 2: Valid registration different user\\n");
    register_user("bob_user", "AnotherPass456");
    
    printf("\\nTest Case 3: Duplicate username (should fail)\\n");
    register_user("alice", "DifferentPass789");
    
    printf("\\nTest Case 4: Invalid username (special chars)\\n");
    register_user("user@invalid", "ValidPass123");
    
    printf("\\nTest Case 5: Invalid password (too short)\\n");
    register_user("charlie", "short");
    
    return 0;
}
