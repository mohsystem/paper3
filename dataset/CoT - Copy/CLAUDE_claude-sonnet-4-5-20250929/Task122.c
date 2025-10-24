
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <openssl/sha.h>

#define MAX_USERS 100
#define MAX_USERNAME 50
#define MAX_PASSWORD 100
#define SALT_LENGTH 16
#define ITERATIONS 10000
#define HASH_LENGTH 64

typedef struct {
    char username[MAX_USERNAME];
    char passwordHash[HASH_LENGTH];
    char salt[HASH_LENGTH];
} UserCredentials;

typedef struct {
    UserCredentials users[MAX_USERS];
    int userCount;
} UserDatabase;

UserDatabase database = {.userCount = 0};

void base64Encode(const unsigned char* data, size_t len, char* output) {
    static const char base64_chars[] = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int i = 0, j = 0;
    unsigned char array3[3], array4[4];
    
    while (len--) {
        array3[i++] = *(data++);
        if (i == 3) {
            array4[0] = (array3[0] & 0xfc) >> 2;
            array4[1] = ((array3[0] & 0x03) << 4) + ((array3[1] & 0xf0) >> 4);
            array4[2] = ((array3[1] & 0x0f) << 2) + ((array3[2] & 0xc0) >> 6);
            array4[3] = array3[2] & 0x3f;
            
            for(i = 0; i < 4; i++) output[j++] = base64_chars[array4[i]];
            i = 0;
        }
    }
    
    if (i) {
        for(int k = i; k < 3; k++) array3[k] = '\\0';
        array4[0] = (array3[0] & 0xfc) >> 2;
        array4[1] = ((array3[0] & 0x03) << 4) + ((array3[1] & 0xf0) >> 4);
        array4[2] = ((array3[1] & 0x0f) << 2) + ((array3[2] & 0xc0) >> 6);
        
        for (int k = 0; k < i + 1; k++) output[j++] = base64_chars[array4[k]];
        while(i++ < 3) output[j++] = '=';
    }
    output[j] = '\\0';
}

void generateSalt(char* salt) {
    unsigned char randomBytes[SALT_LENGTH];
    srand(time(NULL));
    for (int i = 0; i < SALT_LENGTH; i++) {
        randomBytes[i] = rand() % 256;
    }
    base64Encode(randomBytes, SALT_LENGTH, salt);
}

void hashPassword(const char* password, const char* salt, char* output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    char combined[256];
    snprintf(combined, sizeof(combined), "%s%s", salt, password);
    
    SHA256((unsigned char*)combined, strlen(combined), hash);
    
    for (int i = 0; i < ITERATIONS; i++) {
        SHA256(hash, SHA256_DIGEST_LENGTH, hash);
    }
    
    base64Encode(hash, SHA256_DIGEST_LENGTH, output);
}

int validatePassword(const char* password) {
    if (!password || strlen(password) < 8) return 0;
    
    int hasUpper = 0, hasLower = 0, hasDigit = 0, hasSpecial = 0;
    for (int i = 0; password[i]; i++) {
        if (isupper(password[i])) hasUpper = 1;
        else if (islower(password[i])) hasLower = 1;
        else if (isdigit(password[i])) hasDigit = 1;
        else hasSpecial = 1;
    }
    
    return hasUpper && hasLower && hasDigit && hasSpecial;
}

const char* signupUser(const char* username, const char* password) {
    if (!username || strlen(username) == 0) {
        return "Error: Username cannot be empty";
    }
    
    for (int i = 0; i < database.userCount; i++) {
        if (strcmp(database.users[i].username, username) == 0) {
            return "Error: Username already exists";
        }
    }
    
    if (!validatePassword(password)) {
        return "Error: Password must be at least 8 characters and contain uppercase, lowercase, digit, and special character";
    }
    
    if (database.userCount >= MAX_USERS) {
        return "Error: User database full";
    }
    
    UserCredentials* user = &database.users[database.userCount];
    strncpy(user->username, username, MAX_USERNAME - 1);
    user->username[MAX_USERNAME - 1] = '\\0';
    
    generateSalt(user->salt);
    hashPassword(password, user->salt, user->passwordHash);
    
    database.userCount++;
    
    return "Success: User registered successfully";
}

int verifyLogin(const char* username, const char* password) {
    for (int i = 0; i < database.userCount; i++) {
        if (strcmp(database.users[i].username, username) == 0) {
            char hashedAttempt[HASH_LENGTH];
            hashPassword(password, database.users[i].salt, hashedAttempt);
            return strcmp(hashedAttempt, database.users[i].passwordHash) == 0;
        }
    }
    return 0;
}

int main() {
    printf("Test Case 1: Valid signup\\n");
    printf("%s\\n\\n", signupUser("john_doe", "SecurePass123!"));
    
    printf("Test Case 2: Duplicate username\\n");
    printf("%s\\n\\n", signupUser("john_doe", "AnotherPass456@"));
    
    printf("Test Case 3: Weak password\\n");
    printf("%s\\n\\n", signupUser("jane_doe", "weak"));
    
    printf("Test Case 4: Valid signup\\n");
    printf("%s\\n\\n", signupUser("alice_smith", "MyP@ssw0rd!"));
    
    printf("Test Case 5: Empty username\\n");
    printf("%s\\n\\n", signupUser("", "ValidPass123!"));
    
    printf("Login Verification Test:\\n");
    printf("Correct password: %s\\n", verifyLogin("john_doe", "SecurePass123!") ? "true" : "false");
    printf("Wrong password: %s\\n", verifyLogin("john_doe", "WrongPass123!") ? "true" : "false");
    
    return 0;
}
