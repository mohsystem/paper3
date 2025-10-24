
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/sha.h>

#define MAX_USERS 100
#define MAX_USERNAME 50
#define MAX_PASSWORD 100
#define SALT_LENGTH 16

typedef struct {
    char username[MAX_USERNAME];
    char salt[50];
    char hashedPassword[100];
    int active;
} UserCredential;

typedef struct {
    UserCredential users[MAX_USERS];
    int userCount;
} Task42;

const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void base64_encode(const unsigned char* data, size_t len, char* output) {
    int i = 0, j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    
    while (len--) {
        char_array_3[i++] = *(data++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            
            for (i = 0; i < 4; i++)
                output[j++] = base64_chars[char_array_4[i]];
            i = 0;
        }
    }
    
    if (i) {
        for (int k = i; k < 3; k++)
            char_array_3[k] = '\\0';
        
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        
        for (int k = 0; k < i + 1; k++)
            output[j++] = base64_chars[char_array_4[k]];
        
        while (i++ < 3)
            output[j++] = '=';
    }
    output[j] = '\\0';
}

void generateSalt(char* salt) {
    unsigned char bytes[SALT_LENGTH];
    for (int i = 0; i < SALT_LENGTH; i++) {
        bytes[i] = rand() % 256;
    }
    base64_encode(bytes, SALT_LENGTH, salt);
}

void hashPassword(const char* password, const char* salt, char* output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    char combined[200];
    snprintf(combined, sizeof(combined), "%s%s", salt, password);
    
    SHA256((unsigned char*)combined, strlen(combined), hash);
    base64_encode(hash, SHA256_DIGEST_LENGTH, output);
}

void initTask42(Task42* auth) {
    auth->userCount = 0;
    for (int i = 0; i < MAX_USERS; i++) {
        auth->users[i].active = 0;
    }
}

int registerUser(Task42* auth, const char* username, const char* password) {
    for (int i = 0; i < auth->userCount; i++) {
        if (auth->users[i].active && strcmp(auth->users[i].username, username) == 0) {
            return 0;
        }
    }
    
    if (auth->userCount >= MAX_USERS) {
        return 0;
    }
    
    UserCredential* user = &auth->users[auth->userCount];
    strcpy(user->username, username);
    generateSalt(user->salt);
    hashPassword(password, user->salt, user->hashedPassword);
    user->active = 1;
    auth->userCount++;
    
    return 1;
}

int authenticateUser(Task42* auth, const char* username, const char* password) {
    for (int i = 0; i < auth->userCount; i++) {
        if (auth->users[i].active && strcmp(auth->users[i].username, username) == 0) {
            char hashedPassword[100];
            hashPassword(password, auth->users[i].salt, hashedPassword);
            return strcmp(hashedPassword, auth->users[i].hashedPassword) == 0;
        }
    }
    return 0;
}

int deleteUser(Task42* auth, const char* username) {
    for (int i = 0; i < auth->userCount; i++) {
        if (auth->users[i].active && strcmp(auth->users[i].username, username) == 0) {
            auth->users[i].active = 0;
            return 1;
        }
    }
    return 0;
}

int changePassword(Task42* auth, const char* username, const char* oldPassword, const char* newPassword) {
    if (!authenticateUser(auth, username, oldPassword)) {
        return 0;
    }
    
    for (int i = 0; i < auth->userCount; i++) {
        if (auth->users[i].active && strcmp(auth->users[i].username, username) == 0) {
            generateSalt(auth->users[i].salt);
            hashPassword(newPassword, auth->users[i].salt, auth->users[i].hashedPassword);
            return 1;
        }
    }
    return 0;
}

int main() {
    srand(time(NULL));
    Task42 authSystem;
    initTask42(&authSystem);
    
    // Test Case 1: Register new user
    printf("Test 1 - Register user 'alice':\\n");
    int result1 = registerUser(&authSystem, "alice", "password123");
    printf("Registration successful: %s\\n", result1 ? "true" : "false");
    
    // Test Case 2: Authenticate with correct password
    printf("\\nTest 2 - Authenticate alice with correct password:\\n");
    int result2 = authenticateUser(&authSystem, "alice", "password123");
    printf("Authentication successful: %s\\n", result2 ? "true" : "false");
    
    // Test Case 3: Authenticate with incorrect password
    printf("\\nTest 3 - Authenticate alice with incorrect password:\\n");
    int result3 = authenticateUser(&authSystem, "alice", "wrongpassword");
    printf("Authentication successful: %s\\n", result3 ? "true" : "false");
    
    // Test Case 4: Change password
    printf("\\nTest 4 - Change password for alice:\\n");
    int result4 = changePassword(&authSystem, "alice", "password123", "newpassword456");
    printf("Password change successful: %s\\n", result4 ? "true" : "false");
    int result4b = authenticateUser(&authSystem, "alice", "newpassword456");
    printf("Authentication with new password: %s\\n", result4b ? "true" : "false");
    
    // Test Case 5: Register duplicate user and delete user
    printf("\\nTest 5 - Register duplicate user and delete:\\n");
    int result5a = registerUser(&authSystem, "alice", "anotherpassword");
    printf("Duplicate registration blocked: %s\\n", !result5a ? "true" : "false");
    int result5b = deleteUser(&authSystem, "alice");
    printf("User deletion successful: %s\\n", result5b ? "true" : "false");
    int result5c = authenticateUser(&authSystem, "alice", "newpassword456");
    printf("Authentication after deletion: %s\\n", result5c ? "true" : "false");
    
    return 0;
}
