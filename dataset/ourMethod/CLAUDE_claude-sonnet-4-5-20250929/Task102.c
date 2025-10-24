
// C - Secure password update program with PBKDF2 hashing
// Compiles with: gcc -std=c11 -o task task.c -lsqlite3 -lcrypto -lssl

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

#define SALT_SIZE 16
#define HASH_SIZE 32
#define PBKDF2_ITERATIONS 210000
#define MAX_USERNAME_LEN 255
#define MIN_PASSWORD_LEN 8
#define MAX_PASSWORD_LEN 128

typedef struct {
    sqlite3* db;
} PasswordUpdater;

// Securely clear memory to prevent data leakage
static void secure_zero(void* ptr, size_t len) {
    if (ptr == NULL) return;
    volatile unsigned char* p = (volatile unsigned char*)ptr;
    while (len--) {
        *p++ = 0;
    }
}

// Generate cryptographically secure random salt
// Returns 1 on success, 0 on failure
static int generate_salt(unsigned char* salt, size_t salt_len) {
    if (salt == NULL || salt_len != SALT_SIZE) {
        fprintf(stderr, "Invalid salt buffer\\n");
        return 0;
    }
    
    // Use OpenSSL CSPRNG for secure random generation
    if (RAND_bytes(salt, (int)salt_len) != 1) {
        fprintf(stderr, "Failed to generate secure random salt\\n");
        return 0;
    }
    
    return 1;
}

// Hash password using PBKDF2-HMAC-SHA256
// Returns 1 on success, 0 on failure
static int hash_password(const char* password, size_t password_len,
                        const unsigned char* salt, size_t salt_len,
                        unsigned char* hash, size_t hash_len) {
    // Validate all inputs before processing
    if (password == NULL || salt == NULL || hash == NULL) {
        fprintf(stderr, "Null pointer in hash_password\\n");
        return 0;
    }
    
    if (password_len == 0 || salt_len != SALT_SIZE || hash_len != HASH_SIZE) {
        fprintf(stderr, "Invalid buffer sizes for hashing\\n");
        return 0;
    }
    
    // Use PBKDF2-HMAC-SHA256 with high iteration count
    if (PKCS5_PBKDF2_HMAC(password, (int)password_len,
                          salt, (int)salt_len,
                          PBKDF2_ITERATIONS,
                          EVP_sha256(),
                          (int)hash_len,
                          hash) != 1) {
        secure_zero(hash, hash_len);
        fprintf(stderr, "Password hashing failed\\n");
        return 0;
    }
    
    return 1;
}

// Convert binary data to hex string
// Returns allocated string that must be freed by caller, or NULL on error
static char* to_hex(const unsigned char* data, size_t data_len) {
    if (data == NULL || data_len == 0) {
        return NULL;
    }
    
    // Check for potential overflow: data_len * 2 + 1
    if (data_len > (SIZE_MAX - 1) / 2) {
        fprintf(stderr, "Data too large for hex conversion\\n");
        return NULL;
    }
    
    size_t hex_len = data_len * 2 + 1;
    char* hex = (char*)calloc(hex_len, 1);
    if (hex == NULL) {
        fprintf(stderr, "Memory allocation failed\\n");
        return NULL;
    }
    
    const char hex_chars[] = "0123456789abcdef";
    for (size_t i = 0; i < data_len; i++) {
        // Bounds check before writing
        if (i * 2 + 1 >= hex_len) {
            free(hex);
            return NULL;
        }
        hex[i * 2] = hex_chars[data[i] >> 4];
        hex[i * 2 + 1] = hex_chars[data[i] & 0x0F];
    }
    hex[data_len * 2] = '\\0';
    
    return hex;
}

// Initialize database and create users table
static int init_database(sqlite3* db) {
    if (db == NULL) {
        return 0;
    }
    
    const char* create_table = 
        "CREATE TABLE IF NOT EXISTS users ("
        "username TEXT PRIMARY KEY NOT NULL, "
        "password_hash TEXT NOT NULL, "
        "salt TEXT NOT NULL);";
    
    char* err_msg = NULL;
    int rc = sqlite3_exec(db, create_table, NULL, NULL, &err_msg);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Database initialization failed: %s\\n", err_msg);
        sqlite3_free(err_msg);
        return 0;
    }
    
    return 1;
}

// Initialize password updater with database
int password_updater_init(PasswordUpdater* updater, const char* db_path) {
    if (updater == NULL || db_path == NULL) {
        fprintf(stderr, "Invalid parameters to init\\n");
        return 0;
    }
    
    // Validate database path to prevent path traversal
    if (strlen(db_path) == 0 || strstr(db_path, "..") != NULL) {
        fprintf(stderr, "Invalid database path\\n");
        return 0;
    }
    
    updater->db = NULL;
    
    int rc = sqlite3_open(db_path, &updater->db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\\n", 
                updater->db ? sqlite3_errmsg(updater->db) : "unknown error");
        if (updater->db) {
            sqlite3_close(updater->db);
            updater->db = NULL;
        }
        return 0;
    }
    
    if (!init_database(updater->db)) {
        sqlite3_close(updater->db);
        updater->db = NULL;
        return 0;
    }
    
    return 1;
}

// Update user password with secure hashing
int password_updater_update(PasswordUpdater* updater, 
                           const char* username, 
                           const char* new_password) {
    if (updater == NULL || updater->db == NULL || 
        username == NULL || new_password == NULL) {
        fprintf(stderr, "Invalid parameters to update\\n");
        return 0;
    }
    
    // Validate username length
    size_t username_len = strlen(username);
    if (username_len == 0 || username_len > MAX_USERNAME_LEN) {
        fprintf(stderr, "Invalid username length\\n");
        return 0;
    }
    
    // Validate password length
    size_t password_len = strlen(new_password);
    if (password_len < MIN_PASSWORD_LEN || password_len > MAX_PASSWORD_LEN) {
        fprintf(stderr, "Password must be between %d and %d characters\\n",
                MIN_PASSWORD_LEN, MAX_PASSWORD_LEN);
        return 0;
    }
    
    // Generate unique salt
    unsigned char salt[SALT_SIZE];
    if (!generate_salt(salt, SALT_SIZE)) {
        return 0;
    }
    
    // Hash password with salt
    unsigned char hash[HASH_SIZE];
    if (!hash_password(new_password, password_len, salt, SALT_SIZE, 
                      hash, HASH_SIZE)) {
        secure_zero(salt, SALT_SIZE);
        return 0;
    }
    
    // Convert to hex for storage
    char* hash_hex = to_hex(hash, HASH_SIZE);
    char* salt_hex = to_hex(salt, SALT_SIZE);
    
    // Clear sensitive data from memory immediately
    secure_zero(hash, HASH_SIZE);
    secure_zero(salt, SALT_SIZE);
    
    if (hash_hex == NULL || salt_hex == NULL) {
        free(hash_hex);
        free(salt_hex);
        return 0;
    }
    
    // Use parameterized query to prevent SQL injection
    const char* sql = "INSERT OR REPLACE INTO users (username, password_hash, salt) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt = NULL;
    
    int rc = sqlite3_prepare_v2(updater->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\\n", 
                sqlite3_errmsg(updater->db));
        free(hash_hex);
        free(salt_hex);
        return 0;
    }
    
    // Bind parameters safely
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hash_hex, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, salt_hex, -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    // Clear hex strings from memory
    secure_zero(hash_hex, strlen(hash_hex));
    secure_zero(salt_hex, strlen(salt_hex));
    free(hash_hex);
    free(salt_hex);
    
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to update password: %s\\n", 
                sqlite3_errmsg(updater->db));
        return 0;
    }
    
    return 1;
}

// Clean up resources
void password_updater_cleanup(PasswordUpdater* updater) {
    if (updater != NULL && updater->db != NULL) {
        sqlite3_close(updater->db);
        updater->db = NULL;
    }
}

int main(void) {
    PasswordUpdater updater;
    memset(&updater, 0, sizeof(updater));
    
    // Initialize with in-memory database for testing
    if (!password_updater_init(&updater, ":memory:")) {
        return 1;
    }
    
    // Test cases
    typedef struct {
        const char* username;
        const char* password;
        int expected_success;
    } TestCase;
    
    TestCase tests[] = {
        {"alice", "SecurePass123!", 1},
        {"bob", "AnotherStr0ng#Pass", 1},
        {"charlie", "MyP@ssw0rd2024", 1},
        {"", "password", 0},
        {"dave", "short", 0}
    };
    
    size_t num_tests = sizeof(tests) / sizeof(tests[0]);
    for (size_t i = 0; i < num_tests; i++) {
        int result = password_updater_update(&updater, tests[i].username, 
                                            tests[i].password);
        printf("Test: username='%s' - %s\\n", 
               tests[i].username,
               (result == tests[i].expected_success) ? "PASS" : "FAIL");
    }
    
    password_updater_cleanup(&updater);
    return 0;
}
