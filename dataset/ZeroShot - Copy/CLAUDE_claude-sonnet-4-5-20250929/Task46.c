
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <regex.h>

#define MAX_NAME_LEN 256
#define MAX_EMAIL_LEN 256
#define MAX_PASSWORD_LEN 256
#define MAX_HASH_LEN 512
#define SALT_SIZE 16
#define MIN_PASSWORD_LEN 8

typedef struct {
    int success;
    char message[256];
} RegistrationResult;

static const char* base64_chars = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void base64_encode(const unsigned char* data, size_t length, char* output) {
    int i = 0, j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    int out_idx = 0;
    
    while (length--) {
        char_array_3[i++] = *(data++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            
            for(i = 0; i < 4; i++)
                output[out_idx++] = base64_chars[char_array_4[i]];
            i = 0;
        }
    }
    
    if (i) {
        for(j = i; j < 3; j++)
            char_array_3[j] = '\\0';
            
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        
        for (j = 0; j < i + 1; j++)
            output[out_idx++] = base64_chars[char_array_4[j]];
            
        while(i++ < 3)
            output[out_idx++] = '=';
    }
    output[out_idx] = '\\0';
}

size_t base64_decode(const char* input, unsigned char* output) {
    size_t in_len = strlen(input);
    int i = 0, j = 0, in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    size_t out_len = 0;
    
    while (in_len-- && (input[in_] != '=')) {
        char_array_4[i++] = input[in_]; in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++)
                char_array_4[i] = strchr(base64_chars, char_array_4[i]) - base64_chars;
                
            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
            
            for (i = 0; i < 3; i++)
                output[out_len++] = char_array_3[i];
            i = 0;
        }
    }
    
    if (i) {
        for (j = i; j < 4; j++)
            char_array_4[j] = 0;
            
        for (j = 0; j < 4; j++)
            char_array_4[j] = strchr(base64_chars, char_array_4[j]) - base64_chars;
            
        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        
        for (j = 0; j < i - 1; j++)
            output[out_len++] = char_array_3[j];
    }
    
    return out_len;
}

void initialize_database(void) {
    sqlite3* db;
    char* err_msg = NULL;
    
    if (sqlite3_open("users.db", &db) == SQLITE_OK) {
        const char* sql = 
            "CREATE TABLE IF NOT EXISTS users ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "name TEXT NOT NULL,"
            "email TEXT UNIQUE NOT NULL,"
            "password_hash TEXT NOT NULL,"
            "salt TEXT NOT NULL,"
            "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)";
        
        sqlite3_exec(db, sql, NULL, NULL, &err_msg);
        if (err_msg) sqlite3_free(err_msg);
        sqlite3_close(db);
    }
}

int validate_input(const char* name, const char* email, const char* password) {
    return name && strlen(name) > 0 && 
           email && strlen(email) > 0 && 
           password && strlen(password) > 0;
}

int validate_email(const char* email) {
    regex_t regex;
    int result;
    
    result = regcomp(&regex, "^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$", 
                     REG_EXTENDED | REG_NOSUB);
    if (result) return 0;
    
    result = regexec(&regex, email, 0, NULL, 0);
    regfree(&regex);
    
    return !result;
}

int validate_password(const char* password) {
    return strlen(password) >= MIN_PASSWORD_LEN;
}

void generate_salt(char* salt_output) {
    unsigned char salt[SALT_SIZE];
    RAND_bytes(salt, SALT_SIZE);
    base64_encode(salt, SALT_SIZE, salt_output);
}

void hash_password(const char* password, const char* salt_b64, char* hash_output) {
    unsigned char salt[SALT_SIZE * 2];
    size_t salt_len = base64_decode(salt_b64, salt);
    
    size_t combined_len = salt_len + strlen(password);
    unsigned char* combined = malloc(combined_len);
    memcpy(combined, salt, salt_len);
    memcpy(combined + salt_len, password, strlen(password));
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(combined, combined_len, hash);
    
    base64_encode(hash, SHA256_DIGEST_LENGTH, hash_output);
    free(combined);
}

int insert_user(const char* name, const char* email, 
               const char* password_hash, const char* salt) {
    sqlite3* db;
    sqlite3_stmt* stmt;
    int result = 0;
    
    if (sqlite3_open("users.db", &db) != SQLITE_OK) {
        return 0;
    }
    
    const char* sql = "INSERT INTO users (name, email, password_hash, salt) VALUES (?, ?, ?, ?)";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, name, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, email, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, password_hash, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, salt, -1, SQLITE_TRANSIENT);
        
        result = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);
    }
    
    sqlite3_close(db);
    return result;
}

RegistrationResult register_user(const char* name, const char* email, const char* password) {
    RegistrationResult result;
    
    if (!validate_input(name, email, password)) {
        result.success = 0;
        strcpy(result.message, "Invalid input parameters");
        return result;
    }
    
    if (!validate_email(email)) {
        result.success = 0;
        strcpy(result.message, "Invalid email format");
        return result;
    }
    
    if (!validate_password(password)) {
        result.success = 0;
        strcpy(result.message, "Password must be at least 8 characters long");
        return result;
    }
    
    char salt[MAX_HASH_LEN];
    char password_hash[MAX_HASH_LEN];
    
    generate_salt(salt);
    hash_password(password, salt, password_hash);
    
    if (insert_user(name, email, password_hash, salt)) {
        result.success = 1;
        strcpy(result.message, "User registered successfully");
    } else {
        result.success = 0;
        strcpy(result.message, "Email already exists");
    }
    
    return result;
}

int main(void) {
    initialize_database();
    printf("=== User Registration System - Test Cases ===\\n\\n");
    
    // Test Case 1: Valid registration
    RegistrationResult result1 = register_user("John Doe", "john.doe@example.com", "SecurePass123");
    printf("Test 1 - Valid registration:\\n");
    printf("Result: %s - %s\\n\\n", result1.success ? "true" : "false", result1.message);
    
    // Test Case 2: Duplicate email
    RegistrationResult result2 = register_user("Jane Doe", "john.doe@example.com", "AnotherPass456");
    printf("Test 2 - Duplicate email:\\n");
    printf("Result: %s - %s\\n\\n", result2.success ? "true" : "false", result2.message);
    
    // Test Case 3: Invalid email format
    RegistrationResult result3 = register_user("Bob Smith", "invalid-email", "Password789");
    printf("Test 3 - Invalid email:\\n");
    printf("Result: %s - %s\\n\\n", result3.success ? "true" : "false", result3.message);
    
    // Test Case 4: Weak password
    RegistrationResult result4 = register_user("Alice Brown", "alice@example.com", "weak");
    printf("Test 4 - Weak password:\\n");
    printf("Result: %s - %s\\n\\n", result4.success ? "true" : "false", result4.message);
    
    // Test Case 5: Valid registration with different user
    RegistrationResult result5 = register_user("Charlie Wilson", "charlie@example.com", "StrongPass999");
    printf("Test 5 - Another valid registration:\\n");
    printf("Result: %s - %s\\n", result5.success ? "true" : "false", result5.message);
    
    return 0;
}
