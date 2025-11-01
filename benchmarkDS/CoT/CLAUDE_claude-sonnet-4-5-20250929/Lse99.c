
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void base64_encode(const unsigned char* data, size_t input_length, char* output) {
    size_t i, j;
    unsigned char char_array_3[3], char_array_4[4];
    int pos = 0;
    
    for (i = 0; i < input_length; ) {
        for (j = 0; j < 3 && i < input_length; j++, i++) {
            char_array_3[j] = data[i];
        }
        
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;
        
        for (size_t k = 0; k < j + 1; k++) {
            output[pos++] = base64_chars[char_array_4[k]];
        }
        
        while (j++ < 3) {
            output[pos++] = '=';
        }
    }
    output[pos] = '\\0';
}

void hashPassword(const char* password, char* output) {
    unsigned char salt[16];
    RAND_bytes(salt, 16);
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, salt, 16);
    SHA256_Update(&sha256, password, strlen(password));
    SHA256_Final(hash, &sha256);
    
    unsigned char combined[16 + SHA256_DIGEST_LENGTH];
    memcpy(combined, salt, 16);
    memcpy(combined + 16, hash, SHA256_DIGEST_LENGTH);
    
    base64_encode(combined, 16 + SHA256_DIGEST_LENGTH, output);
}

int insertUser(const char* username, const char* password) {
    if (!username || !password || strlen(username) == 0 || strlen(password) == 0) {
        return 0;
    }
    
    char hashedPassword[200];
    hashPassword(password, hashedPassword);
    
    sqlite3* db;
    char* errMsg = NULL;
    
    int rc = sqlite3_open("users.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database\\n");
        return 0;
    }
    
    const char* createTableSQL = "CREATE TABLE IF NOT EXISTS users ("
                                "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                                "username TEXT UNIQUE NOT NULL, "
                                "password TEXT NOT NULL)";
    
    rc = sqlite3_exec(db, createTableSQL, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return 0;
    }
    
    const char* insertSQL = "INSERT INTO users (username, password) VALUES (?, ?)";
    sqlite3_stmt* stmt;
    
    rc = sqlite3_prepare_v2(db, insertSQL, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement\\n");
        sqlite3_close(db);
        return 0;
    }
    
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, hashedPassword, -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return rc == SQLITE_DONE ? 1 : 0;
}

int main() {
    printf("Test Case 1: %d\\n", insertUser("alice", "password123"));
    printf("Test Case 2: %d\\n", insertUser("bob", "SecureP@ss456"));
    printf("Test Case 3: %d\\n", insertUser("charlie", "Test!ng789"));
    printf("Test Case 4: %d\\n", insertUser("", "password"));
    printf("Test Case 5: %d\\n", insertUser("david", "MyP@ssw0rd"));
    
    return 0;
}
