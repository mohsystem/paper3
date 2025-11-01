
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

#define DB_NAME "users.db"
#define MAX_USERNAME_LEN 256
#define MAX_PASSWORD_LEN 256
#define HASH_LEN 512

/* Base64 encode function */
void base64_encode(const unsigned char* buffer, size_t length, char* output) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;
    
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, buffer, length);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    
    memcpy(output, bufferPtr->data, bufferPtr->length);
    output[bufferPtr->length] = '\\0';
    
    BIO_free_all(bio);
}

/* Hash password using SHA-256 */
void hash_password(const char* password, char* output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password, strlen(password), hash);
    base64_encode(hash, SHA256_DIGEST_LENGTH, output);
}

/* Login function with secure password verification */
int login(const char* username, const char* password) {
    /* Input validation */
    if (!username || !password || strlen(username) == 0 || strlen(password) == 0) {
        printf("Invalid credentials.\\n");
        return 0;
    }
    
    sqlite3* db;
    sqlite3_stmt* stmt;
    
    /* Open database */
    if (sqlite3_open(DB_NAME, &db) != SQLITE_OK) {
        fprintf(stderr, "Database error: %s\\n", sqlite3_errmsg(db));
        return 0;
    }
    
    /* Prepare parameterized query to prevent SQL injection */
    const char* query = "SELECT password_hash FROM users WHERE username = ?";
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Query preparation error: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }
    
    /* Bind parameters */
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    
    int login_success = 0;
    char input_hash[HASH_LEN];
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* stored_hash = (const char*)sqlite3_column_text(stmt, 0);
        hash_password(password, input_hash);
        
        /* Secure password comparison */
        if (stored_hash && strcmp(input_hash, stored_hash) == 0) {
            printf("Login successful. Redirecting to home page...\\n");
            login_success = 1;
        } else {
            printf("Incorrect password.\\n");
        }
    } else {
        printf("Username not found.\\n");
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return login_success;
}

/* Helper function to create test database and users */
void setup_test_database() {
    sqlite3* db;
    char* err_msg = NULL;
    
    if (sqlite3_open(DB_NAME, &db) != SQLITE_OK) {
        fprintf(stderr, "Cannot open database\\n");
        return;
    }
    
    sqlite3_exec(db, "DROP TABLE IF EXISTS users", NULL, NULL, &err_msg);
    sqlite3_exec(db, "CREATE TABLE users (id INTEGER PRIMARY KEY, username TEXT UNIQUE, password_hash TEXT)", 
                 NULL, NULL, &err_msg);
    
    /* Add test users */
    const char* test_users[][2] = {
        {"admin", "admin123"},
        {"user1", "password1"},
        {"testuser", "test@123"},
        {"john_doe", "secure_pass"},
        {"alice", "alice2024"}
    };
    
    sqlite3_stmt* stmt;
    const char* insert_query = "INSERT INTO users (username, password_hash) VALUES (?, ?)";
    sqlite3_prepare_v2(db, insert_query, -1, &stmt, NULL);
    
    for (int i = 0; i < 5; i++) {
        char hash[HASH_LEN];
        hash_password(test_users[i][1], hash);
        sqlite3_bind_text(stmt, 1, test_users[i][0], -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, hash, -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_reset(stmt);
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    printf("Test database setup complete.\\n\\n");
}

int main() {
    setup_test_database();
    
    printf("=== Test Cases ===\\n\\n");
    
    /* Test case 1: Valid login */
    printf("Test 1 - Valid login:\\n");
    login("admin", "admin123");
    printf("\\n");
    
    /* Test case 2: Wrong password */
    printf("Test 2 - Wrong password:\\n");
    login("admin", "wrongpassword");
    printf("\\n");
    
    /* Test case 3: Non-existent user */
    printf("Test 3 - Non-existent user:\\n");
    login("nonexistent", "password");
    printf("\\n");
    
    /* Test case 4: Empty credentials */
    printf("Test 4 - Empty credentials:\\n");
    login("", "");
    printf("\\n");
    
    /* Test case 5: Valid login for different user */
    printf("Test 5 - Valid login for different user:\\n");
    login("alice", "alice2024");
    
    return 0;
}
