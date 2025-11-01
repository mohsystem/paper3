
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/sha.h>

#define MAX_USERNAME 50
#define MAX_PASSWORD 100
#define HASH_SIZE 65

typedef struct {
    char username[MAX_USERNAME];
    char passwordHash[HASH_SIZE];
} User;

User userDatabase[5];
int dbInitialized = 0;

void hashPassword(const char* password, char* outputHash) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password, strlen(password), hash);
    
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(outputHash + (i * 2), "%02x", hash[i]);
    }
    outputHash[64] = '\\0';
}

void initializeDatabase() {
    if (dbInitialized) return;
    
    strcpy(userDatabase[0].username, "admin");
    hashPassword("admin123", userDatabase[0].passwordHash);
    
    strcpy(userDatabase[1].username, "user1");
    hashPassword("password1", userDatabase[1].passwordHash);
    
    strcpy(userDatabase[2].username, "john");
    hashPassword("securePass", userDatabase[2].passwordHash);
    
    strcpy(userDatabase[3].username, "alice");
    hashPassword("alice2023", userDatabase[3].passwordHash);
    
    strcpy(userDatabase[4].username, "bob");
    hashPassword("bobSecret", userDatabase[4].passwordHash);
    
    dbInitialized = 1;
}

const char* login(const char* username, const char* password) {
    initializeDatabase();
    
    // Input validation
    if (username == NULL || password == NULL) {
        return "cant_login";
    }
    
    if (strlen(username) == 0 || strlen(password) == 0) {
        return "cant_login";
    }
    
    if (strlen(username) >= MAX_USERNAME || strlen(password) >= MAX_PASSWORD) {
        return "cant_login";
    }
    
    // Hash the provided password
    char hashedPassword[HASH_SIZE];
    hashPassword(password, hashedPassword);
    
    // Check credentials
    for (int i = 0; i < 5; i++) {
        if (strcmp(userDatabase[i].username, username) == 0) {
            if (strcmp(userDatabase[i].passwordHash, hashedPassword) == 0) {
                return "success";
            } else {
                return "cant_login";
            }
        }
    }
    
    return "cant_login";
}

int main() {
    printf("=== Secure Login System Test Cases ===\\n\\n");
    
    // Test Case 1: Valid credentials
    const char* result1 = login("admin", "admin123");
    printf("Test 1 - Valid login (admin): %s\\n", result1);
    
    // Test Case 2: Invalid password
    const char* result2 = login("admin", "wrongpass");
    printf("Test 2 - Invalid password: %s\\n", result2);
    
    // Test Case 3: Non-existent user
    const char* result3 = login("hacker", "password");
    printf("Test 3 - Non-existent user: %s\\n", result3);
    
    // Test Case 4: Empty credentials
    const char* result4 = login("", "");
    printf("Test 4 - Empty credentials: %s\\n", result4);
    
    // Test Case 5: Valid user different password
    const char* result5 = login("john", "securePass");
    printf("Test 5 - Valid login (john): %s\\n", result5);
    
    return 0;
}
