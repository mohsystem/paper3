
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/evp.h>

#define MAX_STRING_LEN 256

typedef struct {
    int success;
    char redirectUrl[MAX_STRING_LEN];
} LoginResult;

char* base64_encode(const unsigned char* buffer, size_t length) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;
    
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, buffer, length);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    BIO_set_close(bio, BIO_NOCLOSE);
    
    char* result = (char*)malloc(bufferPtr->length + 1);
    memcpy(result, bufferPtr->data, bufferPtr->length);
    result[bufferPtr->length] = '\\0';
    
    BIO_free_all(bio);
    BUF_MEM_free(bufferPtr);
    
    return result;
}

void hash_password(const char* password, const char* salt_b64, char* output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    
    // Simplified hashing for C implementation
    unsigned char combined[512];
    snprintf((char*)combined, sizeof(combined), "%s%s", salt_b64, password);
    
    SHA256(combined, strlen((char*)combined), hash);
    
    char* encoded = base64_encode(hash, SHA256_DIGEST_LENGTH);
    strncpy(output, encoded, MAX_STRING_LEN - 1);
    output[MAX_STRING_LEN - 1] = '\\0';
    free(encoded);
}

LoginResult login(const char* username, const char* password) {
    LoginResult result;
    result.success = 0;
    strcpy(result.redirectUrl, "login.html");
    
    if (username == NULL || password == NULL || strlen(username) == 0 || strlen(password) == 0) {
        return result;
    }
    
    sqlite3* db;
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_open("users.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\\n", sqlite3_errmsg(db));
        return result;
    }
    
    const char* query = "SELECT password_hash, salt FROM users WHERE username = ?";
    rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        return result;
    }
    
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* stored_hash = (const char*)sqlite3_column_text(stmt, 0);
        const char* salt = (const char*)sqlite3_column_text(stmt, 1);
        
        char input_hash[MAX_STRING_LEN];
        hash_password(password, salt, input_hash);
        
        if (strcmp(stored_hash, input_hash) == 0) {
            result.success = 1;
            strcpy(result.redirectUrl, "home.html");
        }
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return result;
}

int main() {
    printf("Test 1 - Valid user:\\n");
    LoginResult result1 = login("john_doe", "SecurePass123");
    printf("Success: %d, Redirect: %s\\n", result1.success, result1.redirectUrl);
    
    printf("\\nTest 2 - Invalid password:\\n");
    LoginResult result2 = login("john_doe", "WrongPass");
    printf("Success: %d, Redirect: %s\\n", result2.success, result2.redirectUrl);
    
    printf("\\nTest 3 - Non-existent user:\\n");
    LoginResult result3 = login("fake_user", "password");
    printf("Success: %d, Redirect: %s\\n", result3.success, result3.redirectUrl);
    
    printf("\\nTest 4 - Empty username:\\n");
    LoginResult result4 = login("", "password");
    printf("Success: %d, Redirect: %s\\n", result4.success, result4.redirectUrl);
    
    printf("\\nTest 5 - NULL inputs:\\n");
    LoginResult result5 = login(NULL, NULL);
    printf("Success: %d, Redirect: %s\\n", result5.success, result5.redirectUrl);
    
    return 0;
}
