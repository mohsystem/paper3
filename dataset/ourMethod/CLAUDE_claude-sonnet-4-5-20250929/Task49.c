
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <ctype.h>

/* Security: Constants for validation and crypto */
#define MAX_NAME_LENGTH 100
#define MAX_EMAIL_LENGTH 255
#define MIN_PASSWORD_LENGTH 8
#define MAX_PASSWORD_LENGTH 128
#define SALT_SIZE 16
#define HASH_SIZE 32
#define PBKDF2_ITERATIONS 210000

/* Security: Database structure with proper initialization */
typedef struct {
    sqlite3* db;
} Database;

/* Security: Initialize database pointer to NULL */
void initDatabase(Database* database) {
    if (database != NULL) {
        database->db = NULL;
    }
}

/* Security: Close database and cleanup */
void closeDatabase(Database* database) {
    if (database != NULL && database->db != NULL) {
        sqlite3_close(database->db);
        database->db = NULL;
    }
}

/* Security: Generate cryptographically secure random salt */
int generateSalt(unsigned char* salt, size_t size) {
    if (salt == NULL || size == 0) {
        return 0;
    }
    
    /* Security: Use OpenSSL's CSPRNG */\n    if (RAND_bytes(salt, (int)size) != 1) {\n        fprintf(stderr, "Failed to generate random salt\\n");\n        return 0;\n    }\n    return 1;\n}\n\n/* Security: Hash password using PBKDF2-HMAC-SHA256 */\nint hashPassword(const char* password, size_t passLen, \n                 const unsigned char* salt, unsigned char* hash) {\n    if (password == NULL || salt == NULL || hash == NULL || passLen == 0) {\n        return 0;\n    }\n    \n    /* Security: Use PBKDF2 with SHA256 and high iteration count */\n    if (PKCS5_PBKDF2_HMAC(password, (int)passLen,\n                          salt, SALT_SIZE,\n                          PBKDF2_ITERATIONS,\n                          EVP_sha256(),\n                          HASH_SIZE, hash) != 1) {\n        fprintf(stderr, "Password hashing failed\\n");\n        return 0;\n    }\n    return 1;\n}\n\n/* Security: Validate name input */\nint validateName(const char* name) {\n    size_t len;\n    size_t i;\n    \n    if (name == NULL) {\n        return 0;\n    }\n    \n    len = strlen(name);\n    \n    /* Security: Check length bounds */\n    if (len == 0 || len > MAX_NAME_LENGTH) {\n        return 0;\n    }\n    \n    /* Security: Allow only alphanumeric, spaces, hyphens, apostrophes */\n    for (i = 0; i < len; i++) {\n        char c = name[i];\n        if (!isalpha((unsigned char)c) && c != ' ' && c != '-' && c != '\\'') {
            return 0;
        }
    }
    
    return 1;
}

/* Security: Validate email input */
int validateEmail(const char* email) {
    size_t len;
    size_t i;
    int atFound = 0;
    int dotAfterAt = 0;
    
    if (email == NULL) {
        return 0;
    }
    
    len = strlen(email);
    
    /* Security: Check length bounds */
    if (len == 0 || len > MAX_EMAIL_LENGTH) {
        return 0;
    }
    
    /* Security: Basic email format validation */
    for (i = 0; i < len; i++) {
        char c = email[i];
        if (c == '@') {
            if (atFound || i == 0) {
                return 0;
            }
            atFound = 1;
        } else if (c == '.' && atFound) {
            dotAfterAt = 1;
        } else if (!isalnum((unsigned char)c) && c != '.' && 
                   c != '_' && c != '%' && c != '+' && c != '-') {
            return 0;
        }
    }
    
    return atFound && dotAfterAt;
}

/* Security: Validate password input */
int validatePassword(const char* password) {
    size_t len;
    
    if (password == NULL) {
        return 0;
    }
    
    len = strlen(password);
    
    /* Security: Check length bounds */
    if (len < MIN_PASSWORD_LENGTH || len > MAX_PASSWORD_LENGTH) {
        return 0;
    }
    
    return 1;
}

/* Security: Initialize database with proper schema */
int initializeDatabaseFile(Database* database, const char* dbPath) {
    int rc;
    char* errMsg = NULL;
    const char* sql;
    size_t pathLen;
    size_t i;
    
    if (database == NULL || dbPath == NULL) {
        return 0;
    }
    
    pathLen = strlen(dbPath);
    
    /* Security: Validate database path to prevent path traversal */
    if (pathLen == 0 || pathLen > 255) {
        fprintf(stderr, "Invalid database path\\n");
        return 0;
    }
    
    /* Security: Check for path traversal patterns */
    for (i = 0; i < pathLen - 1; i++) {
        if (dbPath[i] == '.' && dbPath[i+1] == '.') {
            fprintf(stderr, "Path traversal detected\\n");
            return 0;
        }
        if (dbPath[i] == '/' && dbPath[i+1] == '/') {
            fprintf(stderr, "Invalid path format\\n");
            return 0;
        }
    }
    
    rc = sqlite3_open(dbPath, &database->db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database\\n");
        return 0;
    }
    
    /* Security: Create table with appropriate constraints */
    sql = "CREATE TABLE IF NOT EXISTS users ("
          "id INTEGER PRIMARY KEY AUTOINCREMENT,"
          "name TEXT NOT NULL CHECK(length(name) <= 100),"
          "email TEXT NOT NULL UNIQUE CHECK(length(email) <= 255),"
          "password_hash BLOB NOT NULL,"
          "salt BLOB NOT NULL,"
          "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
          ");";
    
    rc = sqlite3_exec(database->db, sql, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\\n", errMsg ? errMsg : "unknown");
        if (errMsg != NULL) {
            sqlite3_free(errMsg);
        }
        return 0;
    }
    
    return 1;
}

/* Security: API endpoint to store user data */
int storeUserData(Database* database, const char* name, 
                  const char* email, const char* password) {
    unsigned char salt[SALT_SIZE];
    unsigned char hash[HASH_SIZE];
    sqlite3_stmt* stmt = NULL;
    const char* sql;
    int rc;
    size_t passLen;
    
    if (database == NULL || database->db == NULL || 
        name == NULL || email == NULL || password == NULL) {
        fprintf(stderr, "Invalid parameters\\n");
        return 0;
    }
    
    /* Security: Validate all inputs before processing */
    if (!validateName(name)) {
        fprintf(stderr, "Invalid name format\\n");
        return 0;
    }
    
    if (!validateEmail(email)) {
        fprintf(stderr, "Invalid email format\\n");
        return 0;
    }
    
    if (!validatePassword(password)) {
        fprintf(stderr, "Invalid password (must be 8-128 characters)\\n");
        return 0;
    }
    
    /* Security: Initialize arrays to zero */
    memset(salt, 0, SALT_SIZE);
    memset(hash, 0, HASH_SIZE);
    
    /* Security: Generate unique salt for this user */
    if (!generateSalt(salt, SALT_SIZE)) {
        return 0;
    }
    
    passLen = strlen(password);
    
    /* Security: Hash password with salt using PBKDF2 */
    if (!hashPassword(password, passLen, salt, hash)) {
        /* Security: Clear sensitive data */
        OPENSSL_cleanse(hash, HASH_SIZE);
        OPENSSL_cleanse(salt, SALT_SIZE);
        return 0;
    }
    
    /* Security: Use prepared statements to prevent SQL injection */
    sql = "INSERT INTO users (name, email, password_hash, salt) VALUES (?, ?, ?, ?)";
    
    rc = sqlite3_prepare_v2(database->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement\\n");
        OPENSSL_cleanse(hash, HASH_SIZE);
        OPENSSL_cleanse(salt, SALT_SIZE);
        return 0;
    }
    
    /* Security: Bind validated parameters */
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, email, -1, SQLITE_TRANSIENT);
    sqlite3_bind_blob(stmt, 3, hash, HASH_SIZE, SQLITE_TRANSIENT);
    sqlite3_bind_blob(stmt, 4, salt, SALT_SIZE, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    /* Security: Clear sensitive data from memory */
    OPENSSL_cleanse(hash, HASH_SIZE);
    OPENSSL_cleanse(salt, SALT_SIZE);
    
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to insert user data\\n");
        return 0;
    }
    
    return 1;
}

int main(void) {
    Database db;
    
    /* Security: Initialize database structure */
    initDatabase(&db);
    
    /* Security: Initialize database in current directory only */
    if (!initializeDatabaseFile(&db, "users.db")) {
        return 1;
    }
    
    printf("API Endpoint Test Cases:\\n");
    printf("========================\\n");
    
    /* Test case 1: Valid user data */
    printf("Test 1: Valid user - ");
    if (storeUserData(&db, "John Doe", "john.doe@example.com", "SecurePass123")) {
        printf("SUCCESS\\n");
    } else {
        printf("FAILED\\n");
    }
    
    /* Test case 2: Another valid user */
    printf("Test 2: Valid user - ");
    if (storeUserData(&db, "Jane Smith", "jane.smith@example.com", "AnotherSecure456")) {
        printf("SUCCESS\\n");
    } else {
        printf("FAILED\\n");
    }
    
    /* Test case 3: Invalid email format */
    printf("Test 3: Invalid email - ");
    if (!storeUserData(&db, "Bob Brown", "invalid-email", "Password789")) {
        printf("CORRECTLY REJECTED\\n");
    } else {
        printf("FAILED (should reject)\\n");
    }
    
    /* Test case 4: Password too short */
    printf("Test 4: Short password - ");
    if (!storeUserData(&db, "Alice Johnson", "alice@example.com", "short")) {
        printf("CORRECTLY REJECTED\\n");
    } else {
        printf("FAILED (should reject)\\n");
    }
    
    /* Test case 5: Invalid name with special characters */
    printf("Test 5: Invalid name - ");
    if (!storeUserData(&db, "User<script>", "user@example.com", "ValidPass123")) {
        printf("CORRECTLY REJECTED\\n");
    } else {
        printf("FAILED (should reject)\\n");
    }
    
    /* Security: Cleanup database connection */
    closeDatabase(&db);
    
    return 0;
}
