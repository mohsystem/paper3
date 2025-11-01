
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <regex.h>

#define SALT_SIZE 16
#define HASH_SIZE 32

// Base64 encoding function
static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void base64_encode(const unsigned char* buffer, size_t length, char* output) {
    int i = 0, j = 0, idx = 0;
    unsigned char char_array_3[3], char_array_4[4];

    while (length--) {
        char_array_3[i++] = *(buffer++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for(i = 0; i < 4; i++)
                output[idx++] = base64_chars[char_array_4[i]];
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
            output[idx++] = base64_chars[char_array_4[j]];

        while(i++ < 3)
            output[idx++] = '=';
    }
    output[idx] = '\\0';
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

int is_valid_username(const char* username) {
    if (username == NULL) return 0;
    
    regex_t regex;
    int reti = regcomp(&regex, "^[a-zA-Z0-9_]{3,20}$", REG_EXTENDED);
    if (reti) return 0;
    
    reti = regexec(&regex, username, 0, NULL, 0);
    regfree(&regex);
    
    return !reti;
}

int is_valid_password(const char* password) {
    return password != NULL && strlen(password) >= 8;
}

int register_user(const char* username, const char* password) {
    if (!is_valid_username(username)) {
        printf("Invalid username format\\n");
        return 0;
    }

    if (!is_valid_password(password)) {
        printf("Invalid password format\\n");
        return 0;
    }

    sqlite3* db;
    sqlite3_stmt* stmt;
    int rc = sqlite3_open("users.db", &db);

    if (rc) {
        printf("Database error occurred\\n");
        return 0;
    }

    const char* create_table = "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT UNIQUE NOT NULL, password_hash TEXT NOT NULL, salt TEXT NOT NULL)";
    sqlite3_exec(db, create_table, 0, 0, 0);

    unsigned char salt[SALT_SIZE];
    generate_salt(salt);
    
    char hashed_password[100];
    hash_password(password, salt, hashed_password);
    
    char salt_string[50];
    base64_encode(salt, SALT_SIZE, salt_string);

    const char* sql = "INSERT INTO users (username, password_hash, salt) VALUES (?, ?, ?)";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

    if (rc != SQLITE_OK) {
        printf("Database error occurred\\n");
        sqlite3_close(db);
        return 0;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, hashed_password, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, salt_string, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_DONE) {
        printf("User registered successfully: %s\\n", username);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
    } else if (rc == SQLITE_CONSTRAINT) {
        printf("Username already exists\\n");
    } else {
        printf("Database error occurred\\n");
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
}

int main() {
    printf("=== User Registration System ===\\n\\n");

    printf("Test 1: Valid registration\\n");
    register_user("john_doe", "SecurePass123");

    printf("\\nTest 2: Duplicate username\\n");
    register_user("john_doe", "AnotherPass456");

    printf("\\nTest 3: Invalid username\\n");
    register_user("ab", "ValidPass789");

    printf("\\nTest 4: Invalid password\\n");
    register_user("jane_doe", "short");

    printf("\\nTest 5: Another valid registration\\n");
    register_user("alice_smith", "MySecurePassword2024");

    return 0;
}
