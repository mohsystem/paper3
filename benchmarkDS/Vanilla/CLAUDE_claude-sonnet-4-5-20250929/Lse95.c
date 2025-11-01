
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

#define SALT_SIZE 16
#define HASH_SIZE 32
#define BASE64_SIZE(n) (((n) + 2) / 3 * 4)

static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void base64_encode(const unsigned char* input, size_t length, char* output) {
    int i = 0, j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    size_t out_len = 0;
    
    while (length--) {
        char_array_3[i++] = *(input++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            
            for(i = 0; i < 4; i++)
                output[out_len++] = base64_chars[char_array_4[i]];
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
            output[out_len++] = base64_chars[char_array_4[j]];
            
        while(i++ < 3)
            output[out_len++] = '=';
    }
    
    output[out_len] = '\\0';
}

void generate_salt(char* salt_output) {
    unsigned char salt[SALT_SIZE];
    RAND_bytes(salt, SALT_SIZE);
    base64_encode(salt, SALT_SIZE, salt_output);
}

void hash_password(const char* password, const char* salt, char* hash_output) {
    size_t salt_len = strlen(salt);
    size_t pass_len = strlen(password);
    char* combined = (char*)malloc(salt_len + pass_len + 1);
    
    strcpy(combined, salt);
    strcat(combined, password);
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)combined, strlen(combined), hash);
    
    base64_encode(hash, SHA256_DIGEST_LENGTH, hash_output);
    
    free(combined);
}

int setup_database(const char* db_name) {
    sqlite3* db;
    char* err_msg = NULL;
    
    int rc = sqlite3_open(db_name, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\\n", sqlite3_errmsg(db));
        return 0;
    }
    
    const char* sql = "CREATE TABLE IF NOT EXISTS users ("
                     "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                     "username TEXT UNIQUE NOT NULL,"
                     "password TEXT NOT NULL);";
    
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 0;
    }
    
    sqlite3_close(db);
    return 1;
}

int register_user(const char* db_name, const char* username, const char* password) {
    sqlite3* db;
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_open(db_name, &db);
    if (rc != SQLITE_OK) {
        return 0;
    }
    
    char salt[BASE64_SIZE(SALT_SIZE) + 1];
    char hashed_password[BASE64_SIZE(HASH_SIZE) + 1];
    char stored_password[200];
    
    generate_salt(salt);
    hash_password(password, salt, hashed_password);
    sprintf(stored_password, "%s:%s", hashed_password, salt);
    
    const char* sql = "INSERT INTO users (username, password) VALUES (?, ?)";
    
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        return 0;
    }
    
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, stored_password, -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return rc == SQLITE_DONE;
}

int verify_user(const char* db_name, const char* username, const char* password) {
    sqlite3* db;
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_open(db_name, &db);
    if (rc != SQLITE_OK) {
        return 0;
    }
    
    const char* sql = "SELECT password FROM users WHERE username = ?";
    
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        return 0;
    }
    
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        const char* stored_password = (const char*)sqlite3_column_text(stmt, 0);
        char stored_hash[100];
        char salt[100];
        
        sscanf(stored_password, "%[^:]:%s", stored_hash, salt);
        
        char hashed_password[BASE64_SIZE(HASH_SIZE) + 1];
        hash_password(password, salt, hashed_password);
        
        int result = strcmp(hashed_password, stored_hash) == 0;
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        return result;
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
}

int main() {
    const char* db_name = "users.db";
    setup_database(db_name);
    
    printf("Test Case 1: Register user1\\n");
    printf("%d\\n", register_user(db_name, "user1", "password123"));
    
    printf("\\nTest Case 2: Register user2\\n");
    printf("%d\\n", register_user(db_name, "user2", "securePass456"));
    
    printf("\\nTest Case 3: Verify user1 with correct password\\n");
    printf("%d\\n", verify_user(db_name, "user1", "password123"));
    
    printf("\\nTest Case 4: Verify user1 with incorrect password\\n");
    printf("%d\\n", verify_user(db_name, "user1", "wrongpassword"));
    
    printf("\\nTest Case 5: Verify non-existent user\\n");
    printf("%d\\n", verify_user(db_name, "user3", "password789"));
    
    return 0;
}
