
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <regex.h>

#define SALT_SIZE 16
#define MAX_USERNAME 21
#define MIN_PASSWORD 8

static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void base64_encode(const unsigned char* data, size_t len, char* output) {
    size_t i, j = 0;
    unsigned char a, b, c;
    
    for (i = 0; i < len; i += 3) {
        a = data[i];
        b = (i + 1 < len) ? data[i + 1] : 0;
        c = (i + 2 < len) ? data[i + 2] : 0;
        
        output[j++] = base64_chars[a >> 2];
        output[j++] = base64_chars[((a & 0x3) << 4) | (b >> 4)];
        output[j++] = (i + 1 < len) ? base64_chars[((b & 0xF) << 2) | (c >> 6)] : '=';
        output[j++] = (i + 2 < len) ? base64_chars[c & 0x3F] : '=';
    }
    output[j] = '\\0';
}

void hash_password(const char* password, const unsigned char* salt, char* output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, salt, SALT_SIZE);
    SHA256_Update(&sha256, password, strlen(password));
    SHA256_Final(hash, &sha256);
    
    base64_encode(hash, SHA256_DIGEST_LENGTH, output);
}

void generate_salt(unsigned char* salt) {
    RAND_bytes(salt, SALT_SIZE);
}

int validate_username(const char* username) {
    regex_t regex;
    int result;
    
    if (regcomp(&regex, "^[a-zA-Z0-9_]{3,20}$", REG_EXTENDED) != 0) {
        return 0;
    }
    
    result = regexec(&regex, username, 0, NULL, 0);
    regfree(&regex);
    
    return result == 0;
}

int register_user(const char* username, const char* password, sqlite3* db) {
    unsigned char salt[SALT_SIZE];
    char hashed_password[256];
    char salt_string[256];
    sqlite3_stmt* stmt;
    int result;
    
    // Input validation
    if (username == NULL || strlen(username) == 0 || 
        password == NULL || strlen(password) < MIN_PASSWORD) {
        printf("Invalid input: username cannot be empty and password must be at least 8 characters\\n");
        return 0;
    }
    
    if (!validate_username(username)) {
        printf("Invalid username format\\n");
        return 0;
    }
    
    // Generate salt and hash password
    generate_salt(salt);
    hash_password(password, salt, hashed_password);
    base64_encode(salt, SALT_SIZE, salt_string);
    
    // Use parameterized query to prevent SQL injection
    const char* sql = "INSERT INTO users (username, password_hash, salt) VALUES (?, ?, ?)";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Database error: %s\\n", sqlite3_errmsg(db));
        return 0;
    }
    
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hashed_password, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, salt_string, -1, SQLITE_TRANSIENT);
    
    result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return result == SQLITE_DONE;
}

void initialize_database(sqlite3* db) {
    const char* sql = "CREATE TABLE IF NOT EXISTS users ("
                     "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                     "username TEXT UNIQUE NOT NULL, "
                     "password_hash TEXT NOT NULL, "
                     "salt TEXT NOT NULL)";
    
    char* errMsg = NULL;
    if (sqlite3_exec(db, sql, NULL, NULL, &errMsg) != SQLITE_OK) {
        printf("Error creating table: %s\\n", errMsg);
        sqlite3_free(errMsg);
    }
}

int main() {
    sqlite3* db;
    
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
        printf("Cannot open database\\n");
        return 1;
    }
    
    initialize_database(db);
    
    // Test cases
    printf("Test Case 1: Valid registration\\n");
    printf("Result: %d\\n", register_user("testuser1", "SecurePass123", db));
    
    printf("\\nTest Case 2: Another valid registration\\n");
    printf("Result: %d\\n", register_user("alice_99", "MyP@ssw0rd!", db));
    
    printf("\\nTest Case 3: Short password (should fail)\\n");
    printf("Result: %d\\n", register_user("testuser2", "short", db));
    
    printf("\\nTest Case 4: Empty username (should fail)\\n");
    printf("Result: %d\\n", register_user("", "ValidPass123", db));
    
    printf("\\nTest Case 5: Invalid username characters (should fail)\\n");
    printf("Result: %d\\n", register_user("test@user!", "ValidPass123", db));
    
    sqlite3_close(db);
    return 0;
}
