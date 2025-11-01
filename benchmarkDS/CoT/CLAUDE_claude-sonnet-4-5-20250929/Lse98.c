
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <regex.h>

#define DB_NAME "users.db"
#define MIN_PASSWORD_LENGTH 8
#define MAX_USERNAME_LENGTH 20
#define SALT_LENGTH 16

typedef struct {
    int success;
    char message[100];
} RegistrationResult;

void bytes_to_hex(const unsigned char* data, size_t length, char* output) {
    for (size_t i = 0; i < length; i++) {
        sprintf(output + (i * 2), "%02x", data[i]);
    }
    output[length * 2] = '\\0';
}

void hex_to_bytes(const char* hex, unsigned char* bytes, size_t length) {
    for (size_t i = 0; i < length; i++) {
        sscanf(hex + 2 * i, "%2hhx", &bytes[i]);
    }
}

void initialize_database() {
    sqlite3* db;
    if (sqlite3_open(DB_NAME, &db) == SQLITE_OK) {
        const char* sql = "CREATE TABLE IF NOT EXISTS users ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                         "username TEXT UNIQUE NOT NULL,"
                         "password_hash TEXT NOT NULL,"
                         "salt TEXT NOT NULL,"
                         "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)";
        
        char* errMsg = NULL;
        sqlite3_exec(db, sql, NULL, NULL, &errMsg);
        if (errMsg) sqlite3_free(errMsg);
        sqlite3_close(db);
    }
}

void hash_password(const char* password, const char* salt_hex, char* output) {
    unsigned char salt[SALT_LENGTH];
    hex_to_bytes(salt_hex, salt, SALT_LENGTH);
    
    size_t combined_len = SALT_LENGTH + strlen(password);
    unsigned char* combined = malloc(combined_len);
    memcpy(combined, salt, SALT_LENGTH);
    memcpy(combined + SALT_LENGTH, password, strlen(password));
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(combined, combined_len, hash);
    
    bytes_to_hex(hash, SHA256_DIGEST_LENGTH, output);
    free(combined);
}

void generate_salt(char* salt_hex) {
    unsigned char salt[SALT_LENGTH];
    RAND_bytes(salt, SALT_LENGTH);
    bytes_to_hex(salt, SALT_LENGTH, salt_hex);
}

int validate_username(const char* username) {
    if (!username || strlen(username) < 3 || strlen(username) > MAX_USERNAME_LENGTH) {
        return 0;
    }
    
    regex_t regex;
    int ret = regcomp(&regex, "^[a-zA-Z0-9_]+$", REG_EXTENDED);
    if (ret) return 0;
    
    ret = regexec(&regex, username, 0, NULL, 0);
    regfree(&regex);
    
    return ret == 0;
}

int validate_password(const char* password) {
    return password && strlen(password) >= MIN_PASSWORD_LENGTH;
}

RegistrationResult register_user(const char* username, const char* password) {
    RegistrationResult result;
    
    if (!validate_username(username)) {
        result.success = 0;
        strcpy(result.message, "Invalid username format");
        return result;
    }
    
    if (!validate_password(password)) {
        result.success = 0;
        strcpy(result.message, "Password must be at least 8 characters");
        return result;
    }
    
    char salt_hex[SALT_LENGTH * 2 + 1];
    generate_salt(salt_hex);
    
    char hashed_password[SHA256_DIGEST_LENGTH * 2 + 1];
    hash_password(password, salt_hex, hashed_password);
    
    sqlite3* db;
    if (sqlite3_open(DB_NAME, &db) != SQLITE_OK) {
        result.success = 0;
        strcpy(result.message, "Database connection failed");
        return result;
    }
    
    const char* sql = "INSERT INTO users (username, password_hash, salt) VALUES (?, ?, ?)";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, hashed_password, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, salt_hex, -1, SQLITE_TRANSIENT);
        
        int step_result = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        if (step_result == SQLITE_DONE) {
            result.success = 1;
            strcpy(result.message, "Registration successful");
            return result;
        } else if (step_result == SQLITE_CONSTRAINT) {
            result.success = 0;
            strcpy(result.message, "Username already exists");
            return result;
        }
    }
    
    sqlite3_close(db);
    result.success = 0;
    strcpy(result.message, "Registration failed");
    return result;
}

int main() {
    initialize_database();
    
    printf("=== User Registration Test Cases ===\\n\\n");
    
    RegistrationResult result1 = register_user("john_doe", "SecurePass123");
    printf("Test 1 - Valid registration: %s\\n", result1.message);
    
    RegistrationResult result2 = register_user("john_doe", "AnotherPass456");
    printf("Test 2 - Duplicate username: %s\\n", result2.message);
    
    RegistrationResult result3 = register_user("ab", "ValidPassword1");
    printf("Test 3 - Invalid username: %s\\n", result3.message);
    
    RegistrationResult result4 = register_user("jane_doe", "short");
    printf("Test 4 - Weak password: %s\\n", result4.message);
    
    RegistrationResult result5 = register_user("alice_smith", "Str0ng!P@ssw0rd");
    printf("Test 5 - Valid with special chars: %s\\n", result5.message);
    
    return 0;
}
