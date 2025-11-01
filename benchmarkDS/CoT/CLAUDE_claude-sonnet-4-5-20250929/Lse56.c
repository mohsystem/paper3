
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <regex.h>

#define MAX_EMAIL_LEN 256
#define MAX_PASS_LEN 256
#define MAX_USERS 100
#define SALT_LEN 32
#define HASH_LEN 65

typedef struct {
    char email[MAX_EMAIL_LEN];
    char passwordHash[HASH_LEN];
    char salt[SALT_LEN + 1];
} User;

typedef struct {
    User users[MAX_USERS];
    int userCount;
    User* currentLoggedInUser;
} Lse56;

void generateSalt(char* salt) {
    unsigned char randomBytes[16];
    RAND_bytes(randomBytes, 16);
    
    for (int i = 0; i < 16; i++) {
        sprintf(salt + (i * 2), "%02x", randomBytes[i]);
    }
    salt[32] = '\\0';
}

void hashPassword(const char* password, const char* salt, char* output) {
    char combined[MAX_PASS_LEN + SALT_LEN];
    sprintf(combined, "%s%s", salt, password);
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)combined, strlen(combined), hash);
    
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[64] = '\\0';
}

int verifyPassword(const char* password, const char* hash, const char* salt) {
    char newHash[HASH_LEN];
    hashPassword(password, salt, newHash);
    return CRYPTO_memcmp(newHash, hash, strlen(newHash)) == 0;
}

int isValidEmail(const char* email) {
    regex_t regex;
    int ret = regcomp(&regex, "^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$", REG_EXTENDED);
    if (ret != 0) return 0;
    
    ret = regexec(&regex, email, 0, NULL, 0);
    regfree(&regex);
    return ret == 0;
}

Lse56* createLse56() {
    Lse56* system = (Lse56*)malloc(sizeof(Lse56));
    system->userCount = 0;
    system->currentLoggedInUser = NULL;
    return system;
}

int registerUser(Lse56* system, const char* email, const char* password) {
    if (!isValidEmail(email) || strlen(password) == 0 || system->userCount >= MAX_USERS) {
        return 0;
    }
    
    for (int i = 0; i < system->userCount; i++) {
        if (strcmp(system->users[i].email, email) == 0) {
            return 0;
        }
    }
    
    User* newUser = &system->users[system->userCount];
    strncpy(newUser->email, email, MAX_EMAIL_LEN - 1);
    generateSalt(newUser->salt);
    hashPassword(password, newUser->salt, newUser->passwordHash);
    
    system->userCount++;
    return 1;
}

int login(Lse56* system, const char* email, const char* password) {
    if (!isValidEmail(email) || strlen(password) == 0) {
        return 0;
    }
    
    for (int i = 0; i < system->userCount; i++) {
        if (strcmp(system->users[i].email, email) == 0) {
            if (verifyPassword(password, system->users[i].passwordHash, system->users[i].salt)) {
                system->currentLoggedInUser = &system->users[i];
                return 1;
            }
        }
    }
    return 0;
}

void logout(Lse56* system) {
    system->currentLoggedInUser = NULL;
}

int isLoggedIn(Lse56* system) {
    return system->currentLoggedInUser != NULL;
}

const char* changeEmail(Lse56* system, const char* oldEmail, const char* newEmail, const char* confirmPassword) {
    if (!isLoggedIn(system)) {
        return "Error: User must be logged in to change email";
    }
    
    if (!isValidEmail(oldEmail) || !isValidEmail(newEmail)) {
        return "Error: Invalid email format";
    }
    
    if (strlen(confirmPassword) == 0) {
        return "Error: Password is required";
    }
    
    if (strcmp(system->currentLoggedInUser->email, oldEmail) != 0) {
        return "Error: Old email does not match current user email";
    }
    
    if (strcmp(oldEmail, newEmail) == 0) {
        return "Error: New email must be different from old email";
    }
    
    if (!verifyPassword(confirmPassword, system->currentLoggedInUser->passwordHash, system->currentLoggedInUser->salt)) {
        return "Error: Incorrect password";
    }
    
    for (int i = 0; i < system->userCount; i++) {
        if (strcmp(system->users[i].email, newEmail) == 0) {
            return "Error: New email already exists";
        }
    }
    
    strncpy(system->currentLoggedInUser->email, newEmail, MAX_EMAIL_LEN - 1);
    return "Success: Email changed successfully";
}

int main() {
    Lse56* system = createLse56();
    
    // Test Case 1: Register and login successfully
    registerUser(system, "user@example.com", "SecurePass123");
    printf("Test 1 - Login: %s\\n", login(system, "user@example.com", "SecurePass123") ? "true" : "false");
    
    // Test Case 2: Change email successfully
    printf("Test 2 - Change Email: %s\\n", changeEmail(system, "user@example.com", "newemail@example.com", "SecurePass123"));
    
    // Test Case 3: Try to change email with wrong password
    logout(system);
    login(system, "newemail@example.com", "SecurePass123");
    printf("Test 3 - Wrong Password: %s\\n", changeEmail(system, "newemail@example.com", "another@example.com", "WrongPass"));
    
    // Test Case 4: Try to change email to same email
    printf("Test 4 - Same Email: %s\\n", changeEmail(system, "newemail@example.com", "newemail@example.com", "SecurePass123"));
    
    // Test Case 5: Try to change email without being logged in
    logout(system);
    printf("Test 5 - Not Logged In: %s\\n", changeEmail(system, "newemail@example.com", "test@example.com", "SecurePass123"));
    
    free(system);
    return 0;
}
