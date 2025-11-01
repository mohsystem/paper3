
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <time.h>

#define MAX_USERS 100
#define MAX_USERNAME 50
#define MAX_PASSWORD 100
#define SALT_LENGTH 16

typedef struct {
    char username[MAX_USERNAME];
    char hashed_password[65];
    char salt[33];
} User;

static User users[MAX_USERS];
static int user_count = 0;

void generate_salt(char* salt) {
    const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    srand(time(NULL) + rand());
    
    for (int i = 0; i < SALT_LENGTH; i++) {
        int key = rand() % (sizeof(charset) - 1);
        salt[i] = charset[key];
    }
    salt[SALT_LENGTH] = '\\0';
}

void hash_password(const char* password, const char* salt, char* output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    char combined[200];
    
    snprintf(combined, sizeof(combined), "%s%s", salt, password);
    
    SHA256((unsigned char*)combined, strlen(combined), hash);
    
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[64] = '\\0';
}

int constant_time_compare(const char* a, const char* b) {
    int result = 0;
    size_t len = strlen(a);
    
    if (len != strlen(b)) return 0;
    
    for (size_t i = 0; i < len; i++) {
        result |= a[i] ^ b[i];
    }
    return result == 0;
}

void register_user(const char* username, const char* password) {
    if (user_count >= MAX_USERS || !username || !password || 
        strlen(username) == 0 || strlen(password) < 8) {
        return;
    }
    
    char salt[SALT_LENGTH + 1];
    generate_salt(salt);
    
    strncpy(users[user_count].username, username, MAX_USERNAME - 1);
    users[user_count].username[MAX_USERNAME - 1] = '\\0';
    
    strncpy(users[user_count].salt, salt, 32);
    users[user_count].salt[32] = '\\0';
    
    hash_password(password, salt, users[user_count].hashed_password);
    
    user_count++;
}

const char* login(const char* username, const char* password) {
    if (!username || !password || strlen(username) == 0 || strlen(password) == 0) {
        return "Username or password is incorrect";
    }
    
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            char provided_hash[65];
            hash_password(password, users[i].salt, provided_hash);
            
            if (constant_time_compare(users[i].hashed_password, provided_hash)) {
                return "Login successful! Redirecting to session page...";
            } else {
                return "Username or password is incorrect";
            }
        }
    }
    
    return "Username or password is incorrect";
}

int main() {
    printf("=== Login System Test Cases ===\\n\\n");
    
    register_user("user1", "Password123!");
    register_user("admin", "Admin@2024");
    register_user("test", "Test#Pass1");
    register_user("john", "John$ecure9");
    register_user("alice", "Alice&Pass7");
    
    printf("Test 1 - Valid credentials (user1):\\n");
    printf("%s\\n\\n", login("user1", "Password123!"));
    
    printf("Test 2 - Valid credentials (admin):\\n");
    printf("%s\\n\\n", login("admin", "Admin@2024"));
    
    printf("Test 3 - Invalid password:\\n");
    printf("%s\\n\\n", login("user1", "WrongPassword"));
    
    printf("Test 4 - Non-existent user:\\n");
    printf("%s\\n\\n", login("nonexistent", "Password123!"));
    
    printf("Test 5 - Empty credentials:\\n");
    printf("%s\\n\\n", login("", ""));
    
    return 0;
}
