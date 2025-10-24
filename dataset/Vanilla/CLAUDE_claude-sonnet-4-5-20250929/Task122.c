
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_USERS 100
#define MAX_USERNAME_LEN 50
#define MAX_PASSWORD_LEN 100
#define SALT_LEN 16
#define HASH_LEN 64

typedef struct {
    char username[MAX_USERNAME_LEN];
    char hashedPassword[HASH_LEN + 1];
    char salt[SALT_LEN * 2 + 1];
    int active;
} UserCredentials;

UserCredentials userDatabase[MAX_USERS];
int userCount = 0;

void generateSalt(char* salt) {
    srand(time(NULL) + rand());
    for (int i = 0; i < SALT_LEN * 2; i++) {
        sprintf(&salt[i], "%x", rand() % 16);
    }
    salt[SALT_LEN * 2] = '\\0';
}

void simpleHash(const char* password, const char* salt, char* output) {
    char combined[MAX_PASSWORD_LEN + SALT_LEN * 2 + 1];
    sprintf(combined, "%s%s", salt, password);
    
    unsigned long hash = 5381;
    for (int i = 0; combined[i] != '\\0'; i++) {
        hash = ((hash << 5) + hash) + combined[i];
    }
    
    sprintf(output, "%016lx", hash);
    for (int i = strlen(output); i < HASH_LEN; i++) {
        output[i] = '0';
    }
    output[HASH_LEN] = '\\0';
}

int findUser(const char* username) {
    for (int i = 0; i < userCount; i++) {
        if (userDatabase[i].active && strcmp(userDatabase[i].username, username) == 0) {
            return i;
        }
    }
    return -1;
}

char* signup(const char* username, const char* password) {
    static char result[200];
    
    if (username == NULL || strlen(username) == 0) {
        strcpy(result, "Error: Username cannot be empty");
        return result;
    }
    if (password == NULL || strlen(password) < 6) {
        strcpy(result, "Error: Password must be at least 6 characters");
        return result;
    }
    if (findUser(username) != -1) {
        strcpy(result, "Error: Username already exists");
        return result;
    }
    if (userCount >= MAX_USERS) {
        strcpy(result, "Error: User database is full");
        return result;
    }
    
    strcpy(userDatabase[userCount].username, username);
    generateSalt(userDatabase[userCount].salt);
    simpleHash(password, userDatabase[userCount].salt, userDatabase[userCount].hashedPassword);
    userDatabase[userCount].active = 1;
    userCount++;
    
    sprintf(result, "Success: User %s registered successfully", username);
    return result;
}

int verifyPassword(const char* username, const char* password) {
    int idx = findUser(username);
    if (idx == -1) {
        return 0;
    }
    
    char hashedInput[HASH_LEN + 1];
    simpleHash(password, userDatabase[idx].salt, hashedInput);
    return strcmp(hashedInput, userDatabase[idx].hashedPassword) == 0;
}

int main() {
    printf("Test Case 1: Valid signup\\n");
    printf("%s\\n", signup("john_doe", "password123"));
    
    printf("\\nTest Case 2: Duplicate username\\n");
    printf("%s\\n", signup("john_doe", "newpassword456"));
    
    printf("\\nTest Case 3: Weak password\\n");
    printf("%s\\n", signup("jane_doe", "123"));
    
    printf("\\nTest Case 4: Empty username\\n");
    printf("%s\\n", signup("", "password123"));
    
    printf("\\nTest Case 5: Multiple valid signups and verification\\n");
    printf("%s\\n", signup("alice", "securePass789"));
    printf("%s\\n", signup("bob", "myPassword!"));
    printf("Verify alice password: %s\\n", verifyPassword("alice", "securePass789") ? "true" : "false");
    printf("Verify bob wrong password: %s\\n", verifyPassword("bob", "wrongPass") ? "true" : "false");
    
    return 0;
}
