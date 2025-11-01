
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#define SALT_LENGTH 32
#define HASH_LENGTH 32
#define MAX_USERNAME 100
#define MAX_PASSWORD 100

// Base64 encoding table
static const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char* base64_encode(const unsigned char* input, int length) {
    int output_length = 4 * ((length + 2) / 3);
    char* encoded = malloc(output_length + 1);
    
    if (!encoded) return NULL;
    
    int i, j;
    for (i = 0, j = 0; i < length;) {
        uint32_t octet_a = i < length ? input[i++] : 0;
        uint32_t octet_b = i < length ? input[i++] : 0;
        uint32_t octet_c = i < length ? input[i++] : 0;
        uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;
        
        encoded[j++] = base64_table[(triple >> 18) & 0x3F];
        encoded[j++] = base64_table[(triple >> 12) & 0x3F];
        encoded[j++] = base64_table[(triple >> 6) & 0x3F];
        encoded[j++] = base64_table[triple & 0x3F];
    }
    
    for (i = 0; i < (3 - length % 3) % 3; i++)
        encoded[output_length - 1 - i] = '=';
    
    encoded[output_length] = '\\0';
    return encoded;
}

unsigned char* base64_decode(const char* input, int* output_length) {
    int length = strlen(input);
    *output_length = length / 4 * 3;
    
    if (input[length - 1] == '=') (*output_length)--;
    if (input[length - 2] == '=') (*output_length)--;
    
    unsigned char* decoded = malloc(*output_length);
    if (!decoded) return NULL;
    
    int i, j;
    for (i = 0, j = 0; i < length;) {
        uint32_t sextet_a = input[i] == '=' ? 0 & i++ : strchr(base64_table, input[i++]) - base64_table;
        uint32_t sextet_b = input[i] == '=' ? 0 & i++ : strchr(base64_table, input[i++]) - base64_table;
        uint32_t sextet_c = input[i] == '=' ? 0 & i++ : strchr(base64_table, input[i++]) - base64_table;
        uint32_t sextet_d = input[i] == '=' ? 0 & i++ : strchr(base64_table, input[i++]) - base64_table;
        
        uint32_t triple = (sextet_a << 18) + (sextet_b << 12) + (sextet_c << 6) + sextet_d;
        
        if (j < *output_length) decoded[j++] = (triple >> 16) & 0xFF;
        if (j < *output_length) decoded[j++] = (triple >> 8) & 0xFF;
        if (j < *output_length) decoded[j++] = triple & 0xFF;
    }
    
    return decoded;
}

char* hash_password(const char* password, const unsigned char* salt) {
    unsigned char hash[HASH_LENGTH];
    
    PKCS5_PBKDF2_HMAC(password, strlen(password),
                      salt, SALT_LENGTH,
                      100000, EVP_sha256(),
                      HASH_LENGTH, hash);
    
    return base64_encode(hash, HASH_LENGTH);
}

void generate_salt(unsigned char* salt) {
    RAND_bytes(salt, SALT_LENGTH);
}

char* authenticate_user(const char* username, const char* password) {
    if (!username || !password || strlen(username) == 0 || strlen(password) == 0) {
        return strdup("redirect:/login?error=invalid");
    }
    
    sqlite3* db;
    sqlite3_stmt* stmt;
    char* result = strdup("redirect:/login?error=invalid");
    
    if (sqlite3_open("users.db", &db) != SQLITE_OK) {
        return result;
    }
    
    const char* query = "SELECT password_hash, salt FROM users WHERE username = ?";
    
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_close(db);
        return result;
    }
    
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* stored_hash = (const char*)sqlite3_column_text(stmt, 0);
        const char* salt_str = (const char*)sqlite3_column_text(stmt, 1);
        
        int salt_len;
        unsigned char* salt = base64_decode(salt_str, &salt_len);
        
        if (salt) {
            char* input_hash = hash_password(password, salt);
            
            if (input_hash && strcmp(stored_hash, input_hash) == 0) {
                free(result);
                result = strdup("redirect:/dashboard");
            }
            
            free(input_hash);
            free(salt);
        }
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return result;
}

void create_user(const char* username, const char* password) {
    sqlite3* db;
    sqlite3_stmt* stmt;
    
    if (sqlite3_open("users.db", &db) != SQLITE_OK) {
        return;
    }
    
    unsigned char salt[SALT_LENGTH];
    generate_salt(salt);
    
    char* password_hash = hash_password(password, salt);
    char* salt_str = base64_encode(salt, SALT_LENGTH);
    
    const char* query = "INSERT INTO users (username, password_hash, salt) VALUES (?, ?, ?)";
    
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, password_hash, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, salt_str, -1, SQLITE_TRANSIENT);
        
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
    
    free(password_hash);
    free(salt_str);
    sqlite3_close(db);
}

void init_database() {
    sqlite3* db;
    
    if (sqlite3_open("users.db", &db) != SQLITE_OK) {
        return;
    }
    
    const char* sql = "CREATE TABLE IF NOT EXISTS users ("
                     "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                     "username TEXT UNIQUE NOT NULL, "
                     "password_hash TEXT NOT NULL, "
                     "salt TEXT NOT NULL)";
    
    sqlite3_exec(db, sql, NULL, NULL, NULL);
    sqlite3_close(db);
}

int main() {
    init_database();
    
    printf("=== Test Cases ===\\n");
    
    // Setup test users
    create_user("admin", "SecurePass123!");
    create_user("user1", "MyPassword456");
    create_user("testuser", "Test@789");
    
    // Test 1: Valid credentials
    char* result1 = authenticate_user("admin", "SecurePass123!");
    printf("Test 1 - Valid login: %s\\n", result1);
    free(result1);
    
    // Test 2: Invalid password
    char* result2 = authenticate_user("admin", "WrongPass");
    printf("Test 2 - Wrong password: %s\\n", result2);
    free(result2);
    
    // Test 3: Non-existent user
    char* result3 = authenticate_user("hacker", "password");
    printf("Test 3 - Invalid user: %s\\n", result3);
    free(result3);
    
    // Test 4: Empty credentials
    char* result4 = authenticate_user("", "");
    printf("Test 4 - Empty input: %s\\n", result4);
    free(result4);
    
    // Test 5: SQL injection attempt
    char* result5 = authenticate_user("admin' OR '1'='1", "anything");
    printf("Test 5 - SQL injection attempt: %s\\n", result5);
    free(result5);
    
    return 0;
}
