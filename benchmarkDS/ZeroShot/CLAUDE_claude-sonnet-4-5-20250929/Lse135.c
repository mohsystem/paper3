
#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>
#include <stdbool.h>

#define MAX_USERS 10
#define MAX_USERNAME_LEN 50
#define MAX_PASSWORD_LEN 65

typedef struct {
    char username[MAX_USERNAME_LEN];
    char password_hash[MAX_PASSWORD_LEN];
} User;

User user_database[MAX_USERS];
int user_count = 0;

// Hash password using SHA-256
void hash_password(const char* password, char* output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password, strlen(password), hash);
    
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[64] = '\\0';
}

// Initialize the user database
void initialize_database() {
    strcpy(user_database[0].username, "admin");
    hash_password("admin123", user_database[0].password_hash);
    
    strcpy(user_database[1].username, "user1");
    hash_password("user456", user_database[1].password_hash);
    
    strcpy(user_database[2].username, "testuser");
    hash_password("test789", user_database[2].password_hash);
    
    user_count = 3;
}

// Check credentials
bool check_mod(const char* username, const char* password) {
    if (username == NULL || password == NULL || strlen(username) == 0 || strlen(password) == 0) {
        return false;
    }
    
    char input_hash[MAX_PASSWORD_LEN];
    hash_password(password, input_hash);
    
    for (int i = 0; i < user_count; i++) {
        if (strcmp(user_database[i].username, username) == 0) {
            return strcmp(user_database[i].password_hash, input_hash) == 0;
        }
    }
    
    return false;
}

int main() {
    initialize_database();
    
    printf("Test Case 1 (Valid - admin): %s\\n", check_mod("admin", "admin123") ? "true" : "false");
    printf("Test Case 2 (Valid - user1): %s\\n", check_mod("user1", "user456") ? "true" : "false");
    printf("Test Case 3 (Invalid password): %s\\n", check_mod("admin", "wrongpass") ? "true" : "false");
    printf("Test Case 4 (Invalid username): %s\\n", check_mod("nonexistent", "admin123") ? "true" : "false");
    printf("Test Case 5 (Empty credentials): %s\\n", check_mod("", "") ? "true" : "false");
    
    return 0;
}
