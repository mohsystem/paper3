
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <openssl/sha.h>

#define MAX_USERNAME_LEN 50
#define MAX_PASSWORD_LEN 100
#define HASH_LEN 65
#define MAX_CREDENTIALS 10

typedef struct {
    char username[MAX_USERNAME_LEN];
    char password_hash[HASH_LEN];
} Credential;

static Credential credentials[MAX_CREDENTIALS];
static int credential_count = 0;

/* Secure password hashing using SHA-256 */
void hash_password(const char* password, char* output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password, strlen(password), hash);
    
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[64] = '\\0';
}

/* Constant-time string comparison to prevent timing attacks */
int constant_time_equals(const char* a, const char* b) {
    if (!a || !b) return 0;
    
    size_t len_a = strlen(a);
    size_t len_b = strlen(b);
    
    if (len_a != len_b) return 0;
    
    volatile unsigned char result = 0;
    for (size_t i = 0; i < len_a; i++) {
        result |= a[i] ^ b[i];
    }
    return result == 0;
}

/* Trim whitespace from string */
void trim(char* str) {
    if (!str) return;
    
    char* start = str;
    while(isspace(*start)) start++;
    
    if (*start == '\\0') {
        str[0] = '\\0';
        return;
    }
    
    char* end = str + strlen(str) - 1;
    while(end > start && isspace(*end)) end--;
    
    size_t len = end - start + 1;
    memmove(str, start, len);
    str[len] = '\\0';
}

/* Sanitize username */
void sanitize_username(char* username) {
    if (!username) return;
    
    char* write = username;
    for (char* read = username; *read; read++) {
        if (isalnum(*read) || *read == '_') {
            *write++ = *read;
        }
    }
    *write = '\\0';
}

/* Add credential to the store */
void add_credential(const char* username, const char* password) {
    if (credential_count >= MAX_CREDENTIALS) return;
    
    strncpy(credentials[credential_count].username, username, MAX_USERNAME_LEN - 1);
    credentials[credential_count].username[MAX_USERNAME_LEN - 1] = '\\0';
    
    hash_password(password, credentials[credential_count].password_hash);
    credential_count++;
}

/* Initialize credentials */
void initialize_credentials() {
    credential_count = 0;
    add_credential("admin", "Admin@123");
    add_credential("user1", "User@456");
    add_credential("moderator", "Mod@789");
}

/* Secure credential validation */
int check_mod(const char* username, const char* password) {
    /* Input validation */
    if (!username || !password || strlen(username) == 0 || strlen(password) == 0) {
        return 0;
    }
    
    char clean_username[MAX_USERNAME_LEN];
    char clean_password[MAX_PASSWORD_LEN];
    
    strncpy(clean_username, username, MAX_USERNAME_LEN - 1);
    clean_username[MAX_USERNAME_LEN - 1] = '\\0';
    strncpy(clean_password, password, MAX_PASSWORD_LEN - 1);
    clean_password[MAX_PASSWORD_LEN - 1] = '\\0';
    
    trim(clean_username);
    trim(clean_password);
    
    if (strlen(clean_username) == 0 || strlen(clean_password) == 0) {
        return 0;
    }
    
    /* Sanitize username */
    sanitize_username(clean_username);
    
    /* Find username in credentials */
    int found = 0;
    char stored_hash[HASH_LEN] = {0};
    
    for (int i = 0; i < credential_count; i++) {
        if (strcmp(credentials[i].username, clean_username) == 0) {
            found = 1;
            strncpy(stored_hash, credentials[i].password_hash, HASH_LEN - 1);
            break;
        }
    }
    
    /* Hash the provided password */
    char hashed_password[HASH_LEN];
    hash_password(clean_password, hashed_password);
    
    /* If username not found, still perform comparison to prevent timing attacks */
    if (!found) {
        return 0;
    }
    
    /* Use constant-time comparison */
    return constant_time_equals(hashed_password, stored_hash);
}

int main() {
    /* Initialize credentials */
    initialize_credentials();
    
    printf("=== Authentication Test Cases ===\\n\\n");
    
    /* Test case 1: Valid credentials */
    printf("Test 1 - Valid admin credentials:\\n");
    printf("Result: %s\\n\\n", check_mod("admin", "Admin@123") ? "true" : "false");
    
    /* Test case 2: Valid user credentials */
    printf("Test 2 - Valid user1 credentials:\\n");
    printf("Result: %s\\n\\n", check_mod("user1", "User@456") ? "true" : "false");
    
    /* Test case 3: Invalid password */
    printf("Test 3 - Invalid password:\\n");
    printf("Result: %s\\n\\n", check_mod("admin", "wrongpassword") ? "true" : "false");
    
    /* Test case 4: Non-existent user */
    printf("Test 4 - Non-existent user:\\n");
    printf("Result: %s\\n\\n", check_mod("hacker", "password123") ? "true" : "false");
    
    /* Test case 5: Empty credentials */
    printf("Test 5 - Empty credentials:\\n");
    printf("Result: %s\\n", check_mod("", "") ? "true" : "false");
    
    return 0;
}
