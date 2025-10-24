
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <openssl/sha.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#endif

#define MAX_USERNAME_LEN 32
#define MIN_USERNAME_LEN 3
#define MAX_PASSWORD_LEN 128
#define MIN_PASSWORD_LEN 8

int validate_username(const char* username) {
    if (username == NULL || strlen(username) == 0) {
        return 0;
    }
    
    size_t len = strlen(username);
    if (len < MIN_USERNAME_LEN || len > MAX_USERNAME_LEN) {
        return 0;
    }
    
    // Check for valid characters
    for (size_t i = 0; i < len; i++) {
        char c = username[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || 
              (c >= '0' && c <= '9') || c == '_' || c == '-')) {
            return 0;
        }
    }
    
    return 1;
}

int validate_password(const char* password) {
    if (password == NULL || strlen(password) == 0) {
        return 0;
    }
    
    size_t len = strlen(password);
    if (len < MIN_PASSWORD_LEN || len > MAX_PASSWORD_LEN) {
        return 0;
    }
    
    return 1;
}

char* hash_password(const char* password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    char* output = (char*)malloc(SHA256_DIGEST_LENGTH * 2 + 1);
    
    if (output == NULL) {
        return NULL;
    }
    
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, password, strlen(password));
    SHA256_Final(hash, &sha256);
    
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[SHA256_DIGEST_LENGTH * 2] = '\\0';
    
    return output;
}

int change_password_secure(const char* username, const char* new_password) {
    // Validate inputs
    if (!validate_username(username)) {
        fprintf(stderr, "Invalid username format\\n");
        return 0;
    }
    
    if (!validate_password(new_password)) {
        fprintf(stderr, "Invalid password format\\n");
        return 0;
    }
    
    // Simulated privileged operation
    printf("Simulating privileged password change for user: %s\\n", username);
    
    // Hash password
    char* hashed_password = hash_password(new_password);
    if (hashed_password == NULL) {
        fprintf(stderr, "Password hashing failed\\n");
        return 0;
    }
    
    printf("Password changed successfully (simulated)\\n");
    
    // Clear sensitive data
    memset(hashed_password, 0, strlen(hashed_password));
    free(hashed_password);
    
    return 1;
}

void drop_privileges(void) {
    printf("Dropping privileges...\\n");
    
#ifdef _WIN32
    // Windows implementation
    printf("Would adjust token privileges on Windows\\n");
    printf("Privileges dropped to limited user\\n");
#else
    // Unix-like systems
    printf("Would call setuid/setgid on Unix-like system\\n");
    printf("Privileges dropped to unprivileged user\\n");
#endif
}

void perform_privileged_operation(const char* username, const char* new_password) {
    int success = change_password_secure(username, new_password);
    
    if (success) {
        printf("Operation completed successfully\\n");
    } else {
        printf("Operation failed\\n");
    }
    
    // Always drop privileges
    drop_privileges();
}

int main(void) {
    printf("=== Test Case 1: Valid username and password ===\\n");
    perform_privileged_operation("testuser1", "SecurePass123!");
    
    printf("\\n=== Test Case 2: Invalid username (special chars) ===\\n");
    perform_privileged_operation("test@user", "SecurePass123!");
    
    printf("\\n=== Test Case 3: Invalid password (too short) ===\\n");
    perform_privileged_operation("testuser2", "short");
    
    printf("\\n=== Test Case 4: Valid username and long password ===\\n");
    perform_privileged_operation("validuser", "ThisIsAVeryLongAndSecurePassword12345!");
    
    printf("\\n=== Test Case 5: Empty username ===\\n");
    perform_privileged_operation("", "SecurePass123!");
    
    return 0;
}
