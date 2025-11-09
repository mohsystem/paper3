
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

// Maximum input lengths to prevent buffer overflows
#define MAX_USERNAME_LEN 64
#define MAX_PASSWORD_LEN 128
#define SALT_LEN 16
#define HASH_LEN 32

// Securely clear sensitive data from memory
static void secure_zero(void *ptr, size_t len) {
    if (ptr == NULL || len == 0) return;
    volatile unsigned char *p = ptr;
    while (len--) *p++ = 0;
}

// Generate cryptographically secure random salt
static int generate_salt(unsigned char *salt, size_t len) {
    // Use CSPRNG to generate salt - Rules#5
    if (RAND_bytes(salt, len) != 1) {
        return -1;
    }
    return 0;
}

// Hash password using PBKDF2-HMAC-SHA256 - Rules#5
static int hash_password(const char *password, const unsigned char *salt,
                        unsigned char *hash, size_t hash_len) {
    if (password == NULL || salt == NULL || hash == NULL) {
        return -1;
    }
    
    size_t password_len = strlen(password);
    
    // Use PBKDF2 with at least 210000 iterations - Rules#5
    if (PKCS5_PBKDF2_HMAC(password, password_len, salt, SALT_LEN,
                          210000, EVP_sha256(), hash_len, hash) != 1) {
        return -1;
    }
    
    return 0;
}

// Validate username: alphanumeric only, proper length - Rules#2, Rules#24, Rules#25
static int validate_username(const char *username) {
    if (username == NULL) return -1;
    
    size_t len = strnlen(username, MAX_USERNAME_LEN + 1);
    
    // Check length bounds - Rules#2
    if (len == 0 || len > MAX_USERNAME_LEN) {
        return -1;
    }
    
    // Allow list validation: alphanumeric and underscore only - Rules#25
    for (size_t i = 0; i < len; i++) {
        char c = username[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '_')) {
            return -1;
        }
    }
    
    return 0;
}

// Validate password: check length only - Rules#2, Rules#24
static int validate_password(const char *password) {
    if (password == NULL) return -1;
    
    size_t len = strnlen(password, MAX_PASSWORD_LEN + 1);
    
    // Enforce minimum and maximum length - Rules#2
    if (len < 8 || len > MAX_PASSWORD_LEN) {
        return -1;
    }
    
    return 0;
}

// Insert user into database with parameterized query - Rules#32
static int insert_user(sqlite3 *db, const char *username,
                      const unsigned char *salt, const unsigned char *hash) {
    sqlite3_stmt *stmt = NULL;
    int rc;
    
    // Use parameterized query to prevent SQL injection - Rules#32
    const char *sql = "INSERT INTO users (username, salt, hash) VALUES (?, ?, ?)";
    
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        return -1;
    }
    
    // Bind parameters safely - Rules#32
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_blob(stmt, 2, salt, SALT_LEN, SQLITE_STATIC);
    sqlite3_bind_blob(stmt, 3, hash, HASH_LEN, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        return -1;
    }
    
    return 0;
}

// Main function to register user
static int register_user(sqlite3 *db, const char *username, const char *password) {
    unsigned char salt[SALT_LEN];
    unsigned char hash[HASH_LEN];
    int result = -1;
    
    // Initialize arrays - Rules#8, Rules#36
    memset(salt, 0, sizeof(salt));
    memset(hash, 0, sizeof(hash));
    
    // Validate inputs - Rules#2, Rules#24
    if (validate_username(username) != 0) {
        fprintf(stderr, "Invalid username\\n"); // Rules#11
        goto cleanup;
    }
    
    if (validate_password(password) != 0) {
        fprintf(stderr, "Invalid password\\n"); // Rules#11
        goto cleanup;
    }
    
    // Generate salt - Rules#5
    if (generate_salt(salt, SALT_LEN) != 0) {
        fprintf(stderr, "Failed to generate salt\\n"); // Rules#11
        goto cleanup;
    }
    
    // Hash password - Rules#5
    if (hash_password(password, salt, hash, HASH_LEN) != 0) {
        fprintf(stderr, "Failed to hash password\\n"); // Rules#11
        goto cleanup;
    }
    
    // Insert into database - Rules#32
    if (insert_user(db, username, salt, hash) != 0) {
        fprintf(stderr, "Failed to insert user\\n"); // Rules#11
        goto cleanup;
    }
    
    result = 0;
    
cleanup:
    // Clear sensitive data - Rules#9, Rules#40
    secure_zero(salt, sizeof(salt));
    secure_zero(hash, sizeof(hash));
    
    return result;
}

int main(void) {
    sqlite3 *db = NULL;
    char username[MAX_USERNAME_LEN + 1];
    char password[MAX_PASSWORD_LEN + 1];
    int rc;
    
    // Initialize buffers - Rules#8, Rules#36
    memset(username, 0, sizeof(username));
    memset(password, 0, sizeof(password));
    
    // Open database
    rc = sqlite3_open("users.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database\\n"); // Rules#11
        return 1;
    }
    
    // Create table if not exists
    const char *create_table = "CREATE TABLE IF NOT EXISTS users ("
                              "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                              "username TEXT UNIQUE NOT NULL,"
                              "salt BLOB NOT NULL,"
                              "hash BLOB NOT NULL)";
    
    rc = sqlite3_exec(db, create_table, NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to create table\\n"); // Rules#11
        sqlite3_close(db);
        return 1;
    }
    
    // Test case 1: Valid user
    printf("Test 1: Valid user registration\\n");
    if (register_user(db, "alice", "SecurePass123") == 0) {
        printf("Success\\n");
    }
    
    // Test case 2: Another valid user
    printf("Test 2: Another valid user\\n");
    if (register_user(db, "bob_user", "AnotherSecure456") == 0) {
        printf("Success\\n");
    }
    
    // Test case 3: Invalid username (too short)
    printf("Test 3: Invalid username\\n");
    if (register_user(db, "", "password123") != 0) {
        printf("Correctly rejected\\n");
    }
    
    // Test case 4: Invalid password (too short)
    printf("Test 4: Invalid password\\n");
    if (register_user(db, "charlie", "short") != 0) {
        printf("Correctly rejected\\n");
    }
    
    // Test case 5: Invalid username (special chars)
    printf("Test 5: Invalid username with special chars\\n");
    if (register_user(db, "user@name", "ValidPass123") != 0) {
        printf("Correctly rejected\\n");
    }
    
    // Clean up
    sqlite3_close(db);
    
    // Clear sensitive data from memory - Rules#9
    secure_zero(password, sizeof(password));
    
    return 0;
}
