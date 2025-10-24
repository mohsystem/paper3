
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <time.h>
#include <openssl/sha.h>
#include <openssl/evp.h>

#define MAX_USERS 100
#define SALT_LENGTH 16
#define MAX_LOGIN_ATTEMPTS 3
#define MAX_USERNAME_LEN 20
#define MAX_PASSWORD_LEN 128
#define HASH_LENGTH 64

typedef struct {
    char username[MAX_USERNAME_LEN + 1];
    char hashedPassword[HASH_LENGTH + 1];
    char salt[HASH_LENGTH + 1];
    int loginAttempts;
    int isActive;
} UserCredential;

typedef struct {
    UserCredential users[MAX_USERS];
    int userCount;
} Task136;

void initTask136(Task136* auth) {
    auth->userCount = 0;
    memset(auth->users, 0, sizeof(auth->users));
}

void base64Encode(const unsigned char* input, int length, char* output) {
    static const char base64Chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int i = 0, j = 0;
    unsigned char array3[3], array4[4];
    
    while (length--) {
        array3[i++] = *(input++);
        if (i == 3) {
            array4[0] = (array3[0] & 0xfc) >> 2;
            array4[1] = ((array3[0] & 0x03) << 4) + ((array3[1] & 0xf0) >> 4);
            array4[2] = ((array3[1] & 0x0f) << 2) + ((array3[2] & 0xc0) >> 6);
            array4[3] = array3[2] & 0x3f;
            
            for (i = 0; i < 4; i++)
                output[j++] = base64Chars[array4[i]];
            i = 0;
        }
    }
    
    if (i) {
        for (int k = i; k < 3; k++)
            array3[k] = '\\0';
        
        array4[0] = (array3[0] & 0xfc) >> 2;
        array4[1] = ((array3[0] & 0x03) << 4) + ((array3[1] & 0xf0) >> 4);
        array4[2] = ((array3[1] & 0x0f) << 2) + ((array3[2] & 0xc0) >> 6);
        
        for (int k = 0; k < i + 1; k++)
            output[j++] = base64Chars[array4[k]];
        
        while (i++ < 3)
            output[j++] = '=';
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
    unsigned char combined[256];
    
    snprintf((char*)combined, sizeof(combined), "%s%s", salt, password);
    
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, combined, strlen((char*)combined));
    SHA256_Final(hash, &sha256);
    
    for (int i = 0; i < 10000; i++) {
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, hash, SHA256_DIGEST_LENGTH);
        SHA256_Final(hash, &sha256);
    }
    
    base64Encode(hash, SHA256_DIGEST_LENGTH, output);
}

int validateUsername(const char* username) {
    if (username == NULL || strlen(username) < 3 || strlen(username) > 20) {
        return 0;
    }
    
    regex_t regex;
    int ret = regcomp(&regex, "^[a-zA-Z0-9_]+$", REG_EXTENDED);
    if (ret != 0) return 0;
    
    ret = regexec(&regex, username, 0, NULL, 0);
    regfree(&regex);
    
    return ret == 0;
}

int validatePassword(const char* password) {
    return password != NULL && strlen(password) >= 8;
}

int constantTimeEquals(const char* a, const char* b) {
    if (a == NULL || b == NULL) return 0;
    
    size_t len_a = strlen(a);
    size_t len_b = strlen(b);
    
    if (len_a != len_b) return 0;
    
    int result = 0;
    for (size_t i = 0; i < len_a; i++) {
        result |= a[i] ^ b[i];
    }
    
    return result == 0;
}

const char* registerUser(Task136* auth, const char* username, const char* password) {
    if (!validateUsername(username)) {
        return "Invalid username. Must be 3-20 alphanumeric characters or underscore.";
    }
    
    if (!validatePassword(password)) {
        return "Invalid password. Must be at least 8 characters long.";
    }
    
    for (int i = 0; i < auth->userCount; i++) {
        if (strcmp(auth->users[i].username, username) == 0) {
            return "Username already exists.";
        }
    }
    
    if (auth->userCount >= MAX_USERS) {
        return "User database full.";
    }
    
    UserCredential* newUser = &auth->users[auth->userCount];
    strncpy(newUser->username, username, MAX_USERNAME_LEN);
    
    generateSalt(newUser->salt);
    hashPassword(password, newUser->salt, newUser->hashedPassword);
    
    newUser->loginAttempts = 0;
    newUser->isActive = 1;
    auth->userCount++;
    
    return "User registered successfully.";
}

const char* authenticateUser(Task136* auth, const char* username, const char* password) {
    if (!validateUsername(username)) {
        return "Authentication failed.";
    }
    
    UserCredential* user = NULL;
    for (int i = 0; i < auth->userCount; i++) {
        if (strcmp(auth->users[i].username, username) == 0) {
            user = &auth->users[i];
            break;
        }
    }
    
    if (user == NULL) {
        char dummySalt[HASH_LENGTH + 1];
        char dummyHash[HASH_LENGTH + 1];
        generateSalt(dummySalt);
        hashPassword(password, dummySalt, dummyHash);
        return "Authentication failed.";
    }
    
    if (user->loginAttempts >= MAX_LOGIN_ATTEMPTS) {
        return "Account locked due to too many failed attempts.";
    }
    
    char hashedInputPassword[HASH_LENGTH + 1];
    hashPassword(password, user->salt, hashedInputPassword);
    
    if (constantTimeEquals(hashedInputPassword, user->hashedPassword)) {
        user->loginAttempts = 0;
        return "Authentication successful.";
    } else {
        user->loginAttempts++;
        
        if (user->loginAttempts >= MAX_LOGIN_ATTEMPTS) {
            return "Account locked due to too many failed attempts.";
        }
        
        return "Authentication failed.";
    }
}

const char* resetLoginAttempts(Task136* auth, const char* username) {
    for (int i = 0; i < auth->userCount; i++) {
        if (strcmp(auth->users[i].username, username) == 0) {
            auth->users[i].loginAttempts = 0;
            return "Login attempts reset successfully.";
        }
    }
    return "User not found.";
}

int main() {
    Task136 auth;
    initTask136(&auth);
    
    printf("=== Test Case 1: Valid User Registration ===\\n");
    printf("%s\\n", registerUser(&auth, "john_doe", "SecurePass123"));
    
    printf("\\n=== Test Case 2: Invalid Username (too short) ===\\n");
    printf("%s\\n", registerUser(&auth, "ab", "SecurePass123"));
    
    printf("\\n=== Test Case 3: Invalid Password (too short) ===\\n");
    printf("%s\\n", registerUser(&auth, "jane_doe", "short"));
    
    printf("\\n=== Test Case 4: Successful Authentication ===\\n");
    printf("%s\\n", authenticateUser(&auth, "john_doe", "SecurePass123"));
    
    printf("\\n=== Test Case 5: Failed Authentication and Account Lockout ===\\n");
    printf("%s\\n", authenticateUser(&auth, "john_doe", "WrongPass1"));
    printf("%s\\n", authenticateUser(&auth, "john_doe", "WrongPass2"));
    printf("%s\\n", authenticateUser(&auth, "john_doe", "WrongPass3"));
    printf("%s\\n", authenticateUser(&auth, "john_doe", "SecurePass123"));
    printf("%s\\n", resetLoginAttempts(&auth, "john_doe"));
    printf("%s\\n", authenticateUser(&auth, "john_doe", "SecurePass123"));
    
    return 0;
}
