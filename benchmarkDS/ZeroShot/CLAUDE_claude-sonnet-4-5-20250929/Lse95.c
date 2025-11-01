
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

#define MAX_USERNAME_LEN 100
#define MAX_PASSWORD_LEN 100
#define SALT_LEN 16
#define HASH_LEN SHA256_DIGEST_LENGTH

typedef struct {
    int success;
    char message[256];
} RegistrationResult;

static const char* base64_chars = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void base64_encode(const unsigned char* data, size_t len, char* output) {
    int i = 0, j = 0, k = 0;
    unsigned char char_array_3[3], char_array_4[4];
    
    while (len--) {
        char_array_3[i++] = *(data++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            
            for (i = 0; i < 4; i++)
                output[k++] = base64_chars[char_array_4[i]];
            i = 0;
        }
    }
    
    if (i) {
        for (j = i; j < 3; j++)
            char_array_3[j] = '\\0';
        
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        
        for (j = 0; j < i + 1; j++)
            output[k++] = base64_chars[char_array_4[j]];
        
        while (i++ < 3)
            output[k++] = '=';
    }
    
    output[k] = '\\0';
}

void initialize_database() {
    sqlite3* db;
    char* err_msg = NULL;
    
    if (sqlite3_open("users.db", &db) == SQLITE_OK) {
        const char* sql = 
            "CREATE TABLE IF NOT EXISTS users ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "username TEXT UNIQUE NOT NULL,"
            "password_hash TEXT NOT NULL,"
            "salt TEXT NOT NULL)";
        
        sqlite3_exec(db, sql, NULL, NULL, &err_msg);
        sqlite3_close(db);
    }
}

void generate_salt(char* salt_output) {
    unsigned char salt[SALT_LEN];
    RAND_bytes(salt, SALT_LEN);
    base64_encode(salt, SALT_LEN, salt_output);
}

void hash_password(const char* password, const char* salt, char* hash_output) {
    unsigned char hash[HASH_LEN];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, salt, strlen(salt));
    SHA256_Update(&sha256, password, strlen(password));
    SHA256_Final(hash, &sha256);
    
    base64_encode(hash, HASH_LEN, hash_output);
}

RegistrationResult register_user(const char* username, const char* password) {
    RegistrationResult result;
    
    if (!username || strlen(username) == 0) {
        result.success = 0;
        strcpy(result.message, "Username cannot be empty");
        return result;
    }
    
    if (!password || strlen(password) < 6) {
        result.success = 0;
        strcpy(result.message, "Password must be at least 6 characters");
        return result;
    }
    
    char salt[64];
    char hashed_password[64];
    generate_salt(salt);
    hash_password(password, salt, hashed_password);
    
    sqlite3* db;
    sqlite3_stmt* stmt;
    
    if (sqlite3_open("users.db", &db) != SQLITE_OK) {
        result.success = 0;
        strcpy(result.message, "Database connection failed");
        return result;
    }
    
    const char* sql = "INSERT INTO users (username, password_hash, salt) VALUES (?, ?, ?)";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, hashed_password, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, salt, -1, SQLITE_TRANSIENT);
        
        int step_result = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        if (step_result == SQLITE_DONE) {
            result.success = 1;
            strcpy(result.message, "User registered successfully");
        } else if (step_result == SQLITE_CONSTRAINT) {
            result.success = 0;
            strcpy(result.message, "Username already exists");
        } else {
            result.success = 0;
            strcpy(result.message, "Registration failed");
        }
    } else {
        sqlite3_close(db);
        result.success = 0;
        strcpy(result.message, "Registration failed");
    }
    
    return result;
}

int verify_login(const char* username, const char* password) {
    sqlite3* db;
    sqlite3_stmt* stmt;
    
    if (sqlite3_open("users.db", &db) != SQLITE_OK) {
        return 0;
    }
    
    const char* sql = "SELECT password_hash, salt FROM users WHERE username = ?";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const char* stored_hash = (const char*)sqlite3_column_text(stmt, 0);
            const char* salt = (const char*)sqlite3_column_text(stmt, 1);
            
            char hashed_password[64];
            hash_password(password, salt, hashed_password);
            
            int match = strcmp(stored_hash, hashed_password) == 0;
            
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            
            return match;
        }
        
        sqlite3_finalize(stmt);
    }
    
    sqlite3_close(db);
    return 0;
}

int main() {
    initialize_database();
    
    printf("=== Test Case 1: Valid Registration ===\\n");
    RegistrationResult result1 = register_user("john_doe", "password123");
    printf("%s\\n", result1.message);
    
    printf("\\n=== Test Case 2: Duplicate Username ===\\n");
    RegistrationResult result2 = register_user("john_doe", "anotherpass");
    printf("%s\\n", result2.message);
    
    printf("\\n=== Test Case 3: Empty Username ===\\n");
    RegistrationResult result3 = register_user("", "password123");
    printf("%s\\n", result3.message);
    
    printf("\\n=== Test Case 4: Short Password ===\\n");
    RegistrationResult result4 = register_user("jane_doe", "123");
    printf("%s\\n", result4.message);
    
    printf("\\n=== Test Case 5: Valid Login ===\\n");
    register_user("alice", "securepass");
    int login_success = verify_login("alice", "securepass");
    printf("Login successful: %s\\n", login_success ? "true" : "false");
    
    int login_fail = verify_login("alice", "wrongpass");
    printf("Login with wrong password: %s\\n", login_fail ? "true" : "false");
    
    return 0;
}
