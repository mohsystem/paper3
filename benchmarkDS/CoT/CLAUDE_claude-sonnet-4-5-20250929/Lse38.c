
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

#define MAX_STRING 256

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
    
    char* result = (char*)malloc(bufferPtr->length + 1);
    memcpy(result, bufferPtr->data, bufferPtr->length);
    result[bufferPtr->length] = '\\0';
    
    BIO_free_all(bio);
    return result;
}

char* hash_password(const char* password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password, strlen(password), hash);
    return base64_encode(hash, SHA256_DIGEST_LENGTH);
}

char* authenticate_user(const char* username, const char* password) {
    if (username == NULL || password == NULL || strlen(username) == 0 || strlen(password) == 0) {
        return "login";
    }
    
    char* hashed_password = hash_password(password);
    sqlite3* db;
    sqlite3_stmt* stmt;
    
    if (sqlite3_open("users.db", &db) != SQLITE_OK) {
        fprintf(stderr, "Database error: %s\\n", sqlite3_errmsg(db));
        free(hashed_password);
        return "login";
    }
    
    const char* query = "SELECT password_hash FROM users WHERE username = ?";
    
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        free(hashed_password);
        return "login";
    }
    
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    
    char* result = "login";
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* stored_hash = (const char*)sqlite3_column_text(stmt, 0);
        if (stored_hash && strcmp(hashed_password, stored_hash) == 0) {
            result = "dashboard";
        }
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    free(hashed_password);
    
    return result;
}

int main() {
    printf("Test Case 1 - Valid user:\\n");
    printf("Redirect to: %s\\n", authenticate_user("admin", "admin123"));
    
    printf("\\nTest Case 2 - Invalid password:\\n");
    printf("Redirect to: %s\\n", authenticate_user("admin", "wrongpass"));
    
    printf("\\nTest Case 3 - Non-existent user:\\n");
    printf("Redirect to: %s\\n", authenticate_user("nonexistent", "password"));
    
    printf("\\nTest Case 4 - Empty credentials:\\n");
    printf("Redirect to: %s\\n", authenticate_user("", ""));
    
    printf("\\nTest Case 5 - NULL credentials:\\n");
    printf("Redirect to: %s\\n", authenticate_user(NULL, NULL));
    
    return 0;
}
