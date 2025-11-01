
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

#define MAX_USERS 100
#define SALT_SIZE 16
#define HASH_SIZE 32
#define MAX_USERNAME 50
#define MAX_PASSWORD 100

typedef struct {
    char username[MAX_USERNAME];
    unsigned char salt[SALT_SIZE];
    unsigned char hash[HASH_SIZE];
} User;

static User userDatabase[MAX_USERS];
static int userCount = 0;

void generateSalt(unsigned char* salt) {
    RAND_bytes(salt, SALT_SIZE);
}

void hashPassword(const char* password, const unsigned char* salt, unsigned char* hash) {
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, salt, SALT_SIZE);
    SHA256_Update(&ctx, password, strlen(password));
    SHA256_Final(hash, &ctx);
}

int constantTimeEquals(const unsigned char* a, const unsigned char* b, size_t len) {
    unsigned char result = 0;
    for (size_t i = 0; i < len; i++) {
        result |= a[i] ^ b[i];
    }
    return result == 0;
}

void addUser(const char* username, const char* password) {
    if (userCount >= MAX_USERS) {
        return;
    }
    
    strncpy(userDatabase[userCount].username, username, MAX_USERNAME - 1);
    userDatabase[userCount].username[MAX_USERNAME - 1] = '\\0';
    
    generateSalt(userDatabase[userCount].salt);
    hashPassword(password, userDatabase[userCount].salt, userDatabase[userCount].hash);
    
    userCount++;
}

const char* check_mod(const char* username, const char* password) {
    if (username == NULL || password == NULL || strlen(username) == 0 || strlen(password) == 0) {
        return "false";
    }
    
    for (int i = 0; i < userCount; i++) {
        if (strcmp(userDatabase[i].username, username) == 0) {
            unsigned char computedHash[HASH_SIZE];
            hashPassword(password, userDatabase[i].salt, computedHash);
            
            if (constantTimeEquals(computedHash, userDatabase[i].hash, HASH_SIZE)) {
                return "true";
            }
            return "false";
        }
    }
    
    return "false";
}

void initializeUsers() {
    addUser("admin", "Admin@123");
    addUser("user1", "Pass@word1");
    addUser("testuser", "Test@123");
}

int main() {
    initializeUsers();
    
    printf("Test Case 1 (Valid admin): %s\\n", check_mod("admin", "Admin@123"));
    printf("Test Case 2 (Valid user1): %s\\n", check_mod("user1", "Pass@word1"));
    printf("Test Case 3 (Invalid password): %s\\n", check_mod("admin", "wrongpass"));
    printf("Test Case 4 (Non-existent user): %s\\n", check_mod("hacker", "password"));
    printf("Test Case 5 (Empty credentials): %s\\n", check_mod("", ""));
    
    return 0;
}
