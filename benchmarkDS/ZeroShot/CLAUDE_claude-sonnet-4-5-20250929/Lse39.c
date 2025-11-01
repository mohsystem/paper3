
#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>

#define MAX_USERS 10
#define MAX_USERNAME_LEN 50
#define MAX_PASSWORD_LEN 100
#define HASH_LEN 65

typedef struct {
    char username[MAX_USERNAME_LEN];
    char passwordHash[HASH_LEN];
} User;

User userDatabase[MAX_USERS];
int userCount = 0;

// Hash password using SHA-256
void hashPassword(const char* password, char* output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password, strlen(password), hash);
    
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[64] = '\\0';
}

// Initialize user database
void initializeDatabase() {
    strcpy(userDatabase[0].username, "admin");
    hashPassword("admin123", userDatabase[0].passwordHash);
    
    strcpy(userDatabase[1].username, "user1");
    hashPassword("password1", userDatabase[1].passwordHash);
    
    strcpy(userDatabase[2].username, "user2");
    hashPassword("password2", userDatabase[2].passwordHash);
    
    strcpy(userDatabase[3].username, "testuser");
    hashPassword("test123", userDatabase[3].passwordHash);
    
    strcpy(userDatabase[4].username, "john");
    hashPassword("john456", userDatabase[4].passwordHash);
    
    userCount = 5;
}

// Secure login function
void login(const char* username, const char* password, char* result) {
    // Input validation
    if (username == NULL || password == NULL || strlen(username) == 0 || strlen(password) == 0) {
        strcpy(result, "Error: Username or password cannot be empty");
        return;
    }
    
    // Hash the input password
    char inputHash[HASH_LEN];
    hashPassword(password, inputHash);
    
    // Check if user exists and password matches
    for (int i = 0; i < userCount; i++) {
        if (strcmp(userDatabase[i].username, username) == 0) {
            if (strcmp(userDatabase[i].passwordHash, inputHash) == 0) {
                sprintf(result, "Success: Redirecting to session page for user: %s", username);
                return;
            }
        }
    }
    
    strcpy(result, "Error: Incorrect username or password");
}

int main() {
    initializeDatabase();
    char result[200];
    
    printf("=== Secure Login System Test Cases ===\\n\\n");
    
    // Test Case 1: Valid credentials
    printf("Test Case 1: Valid login (admin/admin123)\\n");
    login("admin", "admin123", result);
    printf("%s\\n\\n", result);
    
    // Test Case 2: Invalid password
    printf("Test Case 2: Invalid password (admin/wrongpass)\\n");
    login("admin", "wrongpass", result);
    printf("%s\\n\\n", result);
    
    // Test Case 3: Non-existent user
    printf("Test Case 3: Non-existent user (hacker/hack123)\\n");
    login("hacker", "hack123", result);
    printf("%s\\n\\n", result);
    
    // Test Case 4: Empty credentials
    printf("Test Case 4: Empty username\\n");
    login("", "password", result);
    printf("%s\\n\\n", result);
    
    // Test Case 5: Valid user2 login
    printf("Test Case 5: Valid login (user2/password2)\\n");
    login("user2", "password2", result);
    printf("%s\\n", result);
    
    return 0;
}
