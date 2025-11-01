
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

#define DB_NAME "users.db"
#define SALT_LENGTH 16
#define MAX_USERNAME_LENGTH 256
#define MAX_PASSWORD_LENGTH 256

typedef struct {
    int success;
    char message[256];
} RegistrationResult;

static const char base64_chars[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void base64_encode(const unsigned char* data, size_t input_length, char* output) {
    size_t i = 0, j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    size_t output_index = 0;
    
    while (input_length--) {
        char_array_3[i++] = *(data++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            
            for(i = 0; i < 4; i++)
                output[output_index++] = base64_chars[char_array_4[i]];
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
            output[output_index++] = base64_chars[char_array_4[j]];
            
        while(i++ < 3)
            output[output_index++] = '=';
    }
    
    output[output_index] = '\\0';
}

void initialize_database() {
    sqlite3* db;
    char* err_msg = NULL;
    
    int rc = sqlite3_open(DB_NAME, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
    
    const char* sql = "CREATE TABLE IF NOT EXISTS users ("
                     "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                     "username TEXT UNIQUE NOT NULL,"
                     "password_hash TEXT NOT NULL,"
                     "salt TEXT NOT NULL)";
    
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\\n", err_msg);
        sqlite3_free(err_msg);
    }
    
    sqlite3_close(db);
}

void generate_salt(char* salt_output) {
    unsigned char salt[SALT_LENGTH];
    RAND_bytes(salt, SALT_LENGTH);
    base64_encode(salt, SALT_LENGTH, salt_output);
}

void hash_password(const char* password, const char* salt, char* hash_output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, salt, strlen(salt));
    SHA256_Update(&sha256, password, strlen(password));
    SHA256_Final(hash, &sha256);
    
    base64_encode(hash, SHA256_DIGEST_LENGTH, hash_output);
}

RegistrationResult register_user(const char* username, const char* password) {
    RegistrationResult result;
    
    if (username == NULL || strlen(username) == 0) {
        result.success = 0;
        strcpy(result.message, "Username cannot be empty");
        return result;
    }
    
    if (password == NULL || strlen(password) < 8) {
        result.success = 0;
        strcpy(result.message, "Password must be at least 8 characters");
        return result;
    }
    
    char salt[128];
    char hashed_password[128];
    generate_salt(salt);
    hash_password(password, salt, hashed_password);
    
    sqlite3* db;
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_open(DB_NAME, &db);
    if (rc != SQLITE_OK) {
        result.success = 0;
        strcpy(result.message, "Database connection failed");
        return result;
    }
    
    const char* sql = "INSERT INTO users (username, password_hash, salt) VALUES (?, ?, ?)";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, hashed_password, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, salt, -1, SQLITE_TRANSIENT);
        
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        if (rc == SQLITE_DONE) {
            result.success = 1;
            strcpy(result.message, "User registered successfully");
            return result;
        } else if (rc == SQLITE_CONSTRAINT) {
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
    
    // Test case 1: Valid registration
    RegistrationResult result1 = register_user("john_doe", "SecurePass123");
    printf("Test 1: %s\\n", result1.message);
    
    // Test case 2: Duplicate username
    RegistrationResult result2 = register_user("john_doe", "AnotherPass456");
    printf("Test 2: %s\\n", result2.message);
    
    // Test case 3: Short password
    RegistrationResult result3 = register_user("jane_doe", "short");
    printf("Test 3: %s\\n", result3.message);
    
    // Test case 4: Empty username
    RegistrationResult result4 = register_user("", "ValidPassword123");
    printf("Test 4: %s\\n", result4.message);
    
    // Test case 5: Valid registration with special characters
    RegistrationResult result5 = register_user("alice_smith", "P@ssw0rd!2023");
    printf("Test 5: %s\\n", result5.message);
    
    return 0;
}
