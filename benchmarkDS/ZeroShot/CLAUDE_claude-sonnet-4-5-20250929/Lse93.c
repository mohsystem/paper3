
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

static const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void base64_encode(const unsigned char* input, int length, char* output) {
    int i = 0, j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    int out_idx = 0;

    while (length--) {
        char_array_3[i++] = *(input++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for(i = 0; i < 4; i++)
                output[out_idx++] = base64_table[char_array_4[i]];
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
            output[out_idx++] = base64_table[char_array_4[j]];

        while((i++ < 3))
            output[out_idx++] = '=';
    }
    output[out_idx] = '\\0';
}

void generate_salt(char* salt_output) {
    unsigned char salt[16];
    RAND_bytes(salt, 16);
    base64_encode(salt, 16, salt_output);
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

int insert_user(const char* username, const char* password, sqlite3* db) {
    char salt[32];
    char hashed_password[64];
    
    generate_salt(salt);
    hash_password(password, salt, hashed_password);

    const char* sql = "INSERT INTO users (username, password_hash, salt) VALUES (?, ?, ?)";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hashed_password, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, salt, -1, SQLITE_TRANSIENT);

    int success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

int main() {
    sqlite3* db;
    sqlite3_open(":memory:", &db);

    const char* create_table = "CREATE TABLE users (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT UNIQUE, password_hash TEXT, salt TEXT)";
    sqlite3_exec(db, create_table, NULL, NULL, NULL);

    printf("Test 1: %d\\n", insert_user("user1", "password123", db));
    printf("Test 2: %d\\n", insert_user("user2", "securePass456", db));
    printf("Test 3: %d\\n", insert_user("user3", "myP@ssw0rd", db));
    printf("Test 4: %d\\n", insert_user("user4", "testPassword789", db));
    printf("Test 5: %d\\n", insert_user("user5", "anotherSecure#Pass", db));

    sqlite3_close(db);
    return 0;
}
