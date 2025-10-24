
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/sha.h>

#define MAX_USERS 100
#define MAX_USERNAME_LEN 50
#define MAX_PASSWORD_LEN 65

typedef struct {
    char username[MAX_USERNAME_LEN];
    char passwordHash[MAX_PASSWORD_LEN];
} User;

typedef struct {
    User users[MAX_USERS];
    int count;
} Task136;

void hashPassword(const char* password, char* output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password, strlen(password), hash);
    
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[64] = '\\0';
}

int findUser(Task136* auth, const char* username) {
    for (int i = 0; i < auth->count; i++) {
        if (strcmp(auth->users[i].username, username) == 0) {
            return i;
        }
    }
    return -1;
}

int registerUser(Task136* auth, const char* username, const char* password) {
    if (username == NULL || password == NULL || strlen(username) == 0 || strlen(password) == 0) {
        return 0;
    }
    if (findUser(auth, username) != -1) {
        return 0;
    }
    if (auth->count >= MAX_USERS) {
        return 0;
    }
    
    strcpy(auth->users[auth->count].username, username);
    hashPassword(password, auth->users[auth->count].passwordHash);
    auth->count++;
    return 1;
}

int authenticateUser(Task136* auth, const char* username, const char* password) {
    if (username == NULL || password == NULL) {
        return 0;
    }
    
    int index = findUser(auth, username);
    if (index == -1) {
        return 0;
    }
    
    char hashedPassword[MAX_PASSWORD_LEN];
    hashPassword(password, hashedPassword);
    
    return strcmp(auth->users[index].passwordHash, hashedPassword) == 0;
}

int deleteUser(Task136* auth, const char* username) {
    if (username == NULL) {
        return 0;
    }
    
    int index = findUser(auth, username);
    if (index == -1) {
        return 0;
    }
    
    for (int i = index; i < auth->count - 1; i++) {
        auth->users[i] = auth->users[i + 1];
    }
    auth->count--;
    return 1;
}

int changePassword(Task136* auth, const char* username, const char* oldPassword, const char* newPassword) {
    if (!authenticateUser(auth, username, oldPassword)) {
        return 0;
    }
    
    int index = findUser(auth, username);
    hashPassword(newPassword, auth->users[index].passwordHash);
    return 1;
}

int main() {
    Task136 auth = {.count = 0};
    
    // Test Case 1: Register a new user
    printf("Test 1 - Register user 'john': %s\\n", registerUser(&auth, "john", "password123") ? "true" : "false");
    
    // Test Case 2: Authenticate with correct credentials
    printf("Test 2 - Authenticate 'john' with correct password: %s\\n", authenticateUser(&auth, "john", "password123") ? "true" : "false");
    
    // Test Case 3: Authenticate with incorrect credentials
    printf("Test 3 - Authenticate 'john' with wrong password: %s\\n", authenticateUser(&auth, "john", "wrongpassword") ? "true" : "false");
    
    // Test Case 4: Register duplicate user
    printf("Test 4 - Register duplicate user 'john': %s\\n", registerUser(&auth, "john", "newpassword") ? "true" : "false");
    
    // Test Case 5: Change password
    printf("Test 5 - Change password for 'john': %s\\n", changePassword(&auth, "john", "password123", "newpassword123") ? "true" : "false");
    printf("Test 5 - Authenticate with new password: %s\\n", authenticateUser(&auth, "john", "newpassword123") ? "true" : "false");
    
    return 0;
}
