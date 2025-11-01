
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <sqlite3.h>

/* Constants for secure password hashing */
#define SALT_LENGTH 16
#define ITERATIONS 210000
#define KEY_LENGTH 32
#define MAX_USERNAME_LEN 100
#define MIN_PASSWORD_LEN 12
#define MAX_PASSWORD_LEN 128

/* Generate cryptographically secure random salt - CWE-759, CWE-330 */
int generate_salt(unsigned char *salt, size_t len) {
    if (salt == NULL || len == 0) {
        return 0;
    }
    return RAND_bytes(salt, len) == 1;
}

/* Hash password using PBKDF2-HMAC-SHA256 - secure KDF - CWE-327 */
int hash_password(const char *password, const unsigned char *salt, char *output, size_t output_len) {
    if (password == NULL || salt == NULL || output == NULL) {
        return 0;
    }
    
    unsigned char hash[KEY_LENGTH];
    
    /* Use PBKDF2 with SHA-256 */
    if (PKCS5_PBKDF2_HMAC(password, strlen(password),
                          salt, SALT_LENGTH,
                          ITERATIONS,
                          EVP_sha256(),
                          KEY_LENGTH, hash) != 1) {
        return 0;
    }
    
    /* Convert salt and hash to hex string */
    if (output_len < (SALT_LENGTH + KEY_LENGTH) * 2 + 1) {
        explicit_bzero(hash, KEY_LENGTH);
        return 0;
    }
    
    for (size_t i = 0; i < SALT_LENGTH; i++) {
        snprintf(output + i * 2, 3, "%02x", salt[i]);
    }
    for (size_t i = 0; i < KEY_LENGTH; i++) {
        snprintf(output + (SALT_LENGTH + i) * 2, 3, "%02x", hash[i]);
    }
    
    /* Securely clear sensitive data - CWE-14 */
    explicit_bzero(hash, KEY_LENGTH);
    
    return 1;
}

/* Validate password policy - CWE-521 */
int validate_password_policy(const char *password) {
    if (password == NULL) {
        return 0;
    }
    
    size_t len = strlen(password);
    if (len < MIN_PASSWORD_LEN || len > MAX_PASSWORD_LEN) {
        return 0;
    }
    
    int has_upper = 0, has_lower = 0, has_digit = 0, has_special = 0;
    
    for (size_t i = 0; i < len; i++) {
        if (isupper(password[i])) has_upper = 1;
        if (islower(password[i])) has_lower = 1;
        if (isdigit(password[i])) has_digit = 1;
        if (strchr("!@#$%^&*()_+-=[]{}; ':\\"\\\\|,.<>/?", password[i])) has_special = 1;
    }
    
    return has_upper && has_lower && has_digit && has_special;
}

/* Register user with secure password hashing */
int register_user(const char *username, const char *password, sqlite3 *db) {
    /* Input validation - CWE-20 */
    if (username == NULL || strlen(username) == 0 || strlen(username) > MAX_USERNAME_LEN) {
        fprintf(stderr, "Invalid username\\n");
        return 0;
    }
    
    /* Validate password policy - CWE-521 */
    if (!validate_password_policy(password)) {
        fprintf(stderr, "Password must be 12-128 characters with uppercase, lowercase, digit, and special character\\n");
        return 0;
    }
    
    /* Generate unique salt for this user - CWE-759, CWE-330 */
    unsigned char salt[SALT_LENGTH];
    if (!generate_salt(salt, SALT_LENGTH)) {
        fprintf(stderr, "Failed to generate salt\\n");
        return 0;
    }
    
    /* Hash password with salt using secure KDF - CWE-327 */
    char hashed_password[(SALT_LENGTH + KEY_LENGTH) * 2 + 1];
    memset(hashed_password, 0, sizeof(hashed_password));
    
    if (!hash_password(password, salt, hashed_password, sizeof(hashed_password))) {
        fprintf(stderr, "Failed to hash password\\n");
        explicit_bzero(salt, SALT_LENGTH);
        return 0;
    }
    
    /* Use prepared statement to prevent SQL injection - CWE-89 */
    const char *sql = "INSERT INTO users (username, password_hash) VALUES (?, ?)";
    sqlite3_stmt *stmt = NULL;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Registration failed due to system error\\n");
        explicit_bzero(salt, SALT_LENGTH);
        explicit_bzero(hashed_password, sizeof(hashed_password));
        return 0;
    }
    
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hashed_password, -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    /* Securely clear sensitive data */
    explicit_bzero(salt, SALT_LENGTH);
    explicit_bzero(hashed_password, sizeof(hashed_password));
    
    if (rc == SQLITE_CONSTRAINT) {
        fprintf(stderr, "Registration failed: username may already exist\\n");
        return 0;
    } else if (rc != SQLITE_DONE) {
        fprintf(stderr, "Registration failed due to system error\\n");
        return 0;
    }
    
    return 1;
}

int main(void) {
    /* Initialize OpenSSL */
    OpenSSL_add_all_algorithms();
    
    /* Create in-memory database for testing */
    sqlite3 *db = NULL;
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
        fprintf(stderr, "Cannot open database\\n");
        return 1;
    }
    
    /* Create users table */
    const char *create_table = "CREATE TABLE users (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT UNIQUE NOT NULL, password_hash TEXT NOT NULL)";
    char *err_msg = NULL;
    if (sqlite3_exec(db, create_table, NULL, NULL, &err_msg) != SQLITE_OK) {
        fprintf(stderr, "Failed to create table\\n");
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 1;
    }
    
    /* Test case 1: Valid registration */
    printf("Test 1: %d\\n", register_user("alice", "SecurePass123!@#", db));
    
    /* Test case 2: Valid registration with different user */
    printf("Test 2: %d\\n", register_user("bob", "MyP@ssw0rd2024!", db));
    
    /* Test case 3: Weak password (too short) */
    printf("Test 3: %d\\n", register_user("charlie", "Short1!", db));
    
    /* Test case 4: Password without special character */
    printf("Test 4: %d\\n", register_user("david", "NoSpecialChar123", db));
    
    /* Test case 5: Empty username */
    printf("Test 5: %d\\n", register_user("", "ValidPass123!@#", db));
    
    sqlite3_close(db);
    EVP_cleanup();
    
    return 0;
}
