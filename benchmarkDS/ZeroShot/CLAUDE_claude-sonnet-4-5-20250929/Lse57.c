
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>

#define MAX_EMAIL 256
#define MAX_PASSWORD 128
#define MAX_USERS 100
#define HASH_SIZE 65

typedef struct {
    char email[MAX_EMAIL];
    char passwordHash[HASH_SIZE];
} User;

typedef struct {
    User users[MAX_USERS];
    int userCount;
    int loggedInIndex;
    int isLoggedIn;
} Lse57;

void hashPassword(const char* password, char* output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password, strlen(password), hash);
    
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[64] = '\\0';
}

int verifyPassword(User* user, const char* password) {
    char hash[HASH_SIZE];
    hashPassword(password, hash);
    return strcmp(user->passwordHash, hash) == 0;
}

int findUserByEmail(Lse57* system, const char* email) {
    for(int i = 0; i < system->userCount; i++) {
        if(strcmp(system->users[i].email, email) == 0) {
            return i;
        }
    }
    return -1;
}

int isValidEmail(const char* email) {
    if(email == NULL || strlen(email) == 0) return 0;
    
    int atCount = 0;
    int dotAfterAt = 0;
    int atPos = -1;
    
    for(int i = 0; email[i] != '\\0'; i++) {
        if(email[i] == '@') {
            atCount++;
            atPos = i;
        }
        if(atPos != -1 && email[i] == '.' && i > atPos) {
            dotAfterAt = 1;
        }
    }
    
    return atCount == 1 && dotAfterAt && atPos > 0;
}

void initSystem(Lse57* system) {
    system->userCount = 0;
    system->loggedInIndex = -1;
    system->isLoggedIn = 0;
}

int registerUser(Lse57* system, const char* email, const char* password) {
    if(system->userCount >= MAX_USERS) return 0;
    if(findUserByEmail(system, email) != -1) return 0;
    
    User* newUser = &system->users[system->userCount];
    strncpy(newUser->email, email, MAX_EMAIL - 1);
    newUser->email[MAX_EMAIL - 1] = '\\0';
    hashPassword(password, newUser->passwordHash);
    system->userCount++;
    
    return 1;
}

int login(Lse57* system, const char* email, const char* password) {
    int index = findUserByEmail(system, email);
    if(index == -1) return 0;
    
    if(verifyPassword(&system->users[index], password)) {
        system->loggedInIndex = index;
        system->isLoggedIn = 1;
        return 1;
    }
    return 0;
}

void logout(Lse57* system) {
    system->loggedInIndex = -1;
    system->isLoggedIn = 0;
}

const char* changeEmail(Lse57* system, const char* oldEmail, const char* newEmail, 
                       const char* confirmPassword) {
    // Check if user is logged in
    if(!system->isLoggedIn || system->loggedInIndex == -1) {
        return "Error: User not logged in";
    }
    
    User* currentUser = &system->users[system->loggedInIndex];
    
    // Verify old email matches logged in user's email\n    if(strcmp(currentUser->email, oldEmail) != 0) {\n        return "Error: Old email does not match current user email";\n    }\n    \n    // Verify old email exists in database\n    if(findUserByEmail(system, oldEmail) == -1) {\n        return "Error: Old email does not exist";\n    }\n    \n    // Verify confirm password is correct\n    if(!verifyPassword(currentUser, confirmPassword)) {\n        return "Error: Incorrect password";\n    }\n    \n    // Check if new email is already in use\n    int newEmailIndex = findUserByEmail(system, newEmail);\n    if(newEmailIndex != -1 && strcmp(newEmail, oldEmail) != 0) {\n        return "Error: New email already exists";\n    }\n    \n    // Validate email format\n    if(!isValidEmail(newEmail)) {\n        return "Error: Invalid email format";\n    }\n    \n    // Change email\n    strncpy(currentUser->email, newEmail, MAX_EMAIL - 1);\n    currentUser->email[MAX_EMAIL - 1] = '\\0';
    
    return "Success: Email changed successfully";
}

int main() {
    Lse57 system;
    initSystem(&system);
    
    // Test Case 1: Successful email change
    printf("Test Case 1: Successful email change\\n");
    registerUser(&system, "user1@example.com", "password123");
    login(&system, "user1@example.com", "password123");
    const char* result1 = changeEmail(&system, "user1@example.com", "newuser1@example.com", "password123");
    printf("%s\\n\\n", result1);
    logout(&system);
    
    // Test Case 2: User not logged in
    printf("Test Case 2: User not logged in\\n");
    const char* result2 = changeEmail(&system, "user2@example.com", "newuser2@example.com", "password123");
    printf("%s\\n\\n", result2);
    
    // Test Case 3: Wrong password
    printf("Test Case 3: Wrong password\\n");
    registerUser(&system, "user3@example.com", "password456");
    login(&system, "user3@example.com", "password456");
    const char* result3 = changeEmail(&system, "user3@example.com", "newuser3@example.com", "wrongpassword");
    printf("%s\\n\\n", result3);
    logout(&system);
    
    // Test Case 4: Old email mismatch
    printf("Test Case 4: Old email mismatch\\n");
    registerUser(&system, "user4@example.com", "password789");
    login(&system, "user4@example.com", "password789");
    const char* result4 = changeEmail(&system, "different@example.com", "newuser4@example.com", "password789");
    printf("%s\\n\\n", result4);
    logout(&system);
    
    // Test Case 5: New email already exists
    printf("Test Case 5: New email already exists\\n");
    registerUser(&system, "user5@example.com", "password101");
    registerUser(&system, "existing@example.com", "password102");
    login(&system, "user5@example.com", "password101");
    const char* result5 = changeEmail(&system, "user5@example.com", "existing@example.com", "password101");
    printf("%s\\n", result5);
    logout(&system);
    
    return 0;
}
