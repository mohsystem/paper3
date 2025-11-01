#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_USERS 10
#define MAX_LEN 64

// --- User Model ---
typedef struct {
    char username[MAX_LEN];
    char email[MAX_LEN];
    char hashedPassword[MAX_LEN];
    bool isLoggedIn;
} User;

// --- Data Structures to Simulate Database ---
User userDatabase[MAX_USERS];
int userCount = 0;

// --- Security Simulation ---

/**
 * A simple, insecure password hashing simulation.
 * WARNING: Do NOT use this in production. Use a strong library like libsodium or OpenSSL.
 */
void hashPassword(const char* password, char* hashBuffer, size_t bufferSize) {
    // Simple "hash": reverse the string and append "_hashed"
    size_t passLen = strlen(password);
    if (passLen + 7 >= bufferSize) {
        strncpy(hashBuffer, "hash_too_long", bufferSize -1);
        hashBuffer[bufferSize - 1] = '\0';
        return;
    }

    for (size_t i = 0; i < passLen; ++i) {
        hashBuffer[i] = password[passLen - 1 - i];
    }
    hashBuffer[passLen] = '\0';
    strcat(hashBuffer, "_hashed");
}

// --- Helper Functions ---
User* findUserByUsername(const char* username) {
    for (int i = 0; i < userCount; ++i) {
        if (strcmp(userDatabase[i].username, username) == 0) {
            return &userDatabase[i];
        }
    }
    return NULL;
}

bool isEmailInUse(const char* email) {
    for (int i = 0; i < userCount; ++i) {
        if (strcmp(userDatabase[i].email, email) == 0) {
            return true;
        }
    }
    return false;
}

// --- Core Logic ---
void setupDatabase() {
    userCount = 0;
    
    const char* usernames[] = {"alice", "bob", "charlie"};
    const char* emails[] = {"alice@example.com", "bob@example.com", "charlie@example.com"};
    const char* passwords[] = {"password123", "bobpass", "securepass"};
    
    for (int i = 0; i < 3; ++i) {
        if (userCount < MAX_USERS) {
            strncpy(userDatabase[userCount].username, usernames[i], MAX_LEN - 1);
            strncpy(userDatabase[userCount].email, emails[i], MAX_LEN - 1);
            hashPassword(passwords[i], userDatabase[userCount].hashedPassword, MAX_LEN);
            userDatabase[userCount].isLoggedIn = false;
            userCount++;
        }
    }
}

bool login(const char* username, const char* password) {
    User* user = findUserByUsername(username);
    if (user != NULL) {
        char passwordHash[MAX_LEN];
        hashPassword(password, passwordHash, MAX_LEN);
        if (strcmp(user->hashedPassword, passwordHash) == 0) {
            user->isLoggedIn = true;
            return true;
        }
    }
    return false;
}

void logout(const char* username) {
    User* user = findUserByUsername(username);
    if (user != NULL) {
        user->isLoggedIn = false;
    }
}

const char* changeEmail(const char* loggedInUsername, const char* oldEmail, const char* newEmail, const char* confirmPassword) {
    // 1. Fetch user data and check login status
    User* user = findUserByUsername(loggedInUsername);
    if (user == NULL || !user->isLoggedIn) {
        return "Error: User is not logged in.";
    }

    // 2. Verify old email
    if (strcmp(user->email, oldEmail) != 0) {
        return "Error: The provided old email does not match our records.";
    }

    // 3. Verify password
    char passwordHash[MAX_LEN];
    hashPassword(confirmPassword, passwordHash, MAX_LEN);
    if (strcmp(user->hashedPassword, passwordHash) != 0) {
        return "Error: Incorrect password.";
    }

    // 4. Validate new email
    if (newEmail == NULL || strchr(newEmail, '@') == NULL) {
        return "Error: New email format is invalid.";
    }
    if (strcmp(user->email, newEmail) == 0) {
        return "Error: New email cannot be the same as the old email.";
    }
    if (isEmailInUse(newEmail)) {
        return "Error: New email is already in use by another account.";
    }

    // 5. All checks passed, perform update
    strncpy(user->email, newEmail, MAX_LEN - 1);
    user->email[MAX_LEN - 1] = '\0'; // Ensure null termination

    // In a real C program, a dynamically sized buffer would be better for this message
    static char successMsg[MAX_LEN + 50]; 
    snprintf(successMsg, sizeof(successMsg), "Success: Email has been changed to %s", newEmail);
    return successMsg;
}

void printUserInfo(const char* username) {
    User* user = findUserByUsername(username);
    if (user != NULL) {
        printf("User{username='%s', email='%s'}\n", user->username, user->email);
    } else {
        printf("User %s not found.\n", username);
    }
}

int main() {
    // Test Case 1: Successful email change
    printf("--- Test Case 1: Successful Change ---\n");
    setupDatabase();
    login("alice", "password123");
    printf("Alice's current info: "); printUserInfo("alice");
    const char* result1 = changeEmail("alice", "alice@example.com", "alice_new@example.com", "password123");
    printf("Result: %s\n", result1);
    printf("Alice's updated info: "); printUserInfo("alice");
    logout("alice");
    printf("\n");

    // Test Case 2: Failed change due to incorrect password
    printf("--- Test Case 2: Incorrect Password ---\n");
    setupDatabase();
    login("bob", "bobpass");
    const char* result2 = changeEmail("bob", "bob@example.com", "bob_new@example.com", "wrongpassword");
    printf("Result: %s\n", result2);
    printf("Bob's info (should be unchanged): "); printUserInfo("bob");
    logout("bob");
    printf("\n");

    // Test Case 3: Failed change because user is not logged in
    printf("--- Test Case 3: User Not Logged In ---\n");
    setupDatabase();
    const char* result3 = changeEmail("charlie", "charlie@example.com", "charlie_new@example.com", "securepass");
    printf("Result: %s\n", result3);
    printf("\n");

    // Test Case 4: Failed change because new email is already taken
    printf("--- Test Case 4: New Email Already Exists ---\n");
    setupDatabase();
    login("alice", "password123");
    const char* result4 = changeEmail("alice", "alice@example.com", "bob@example.com", "password123");
    printf("Result: %s\n", result4);
    logout("alice");
    printf("\n");

    // Test Case 5: Failed change due to incorrect old email provided
    printf("--- Test Case 5: Incorrect Old Email Provided ---\n");
    setupDatabase();
    login("bob", "bobpass");
    const char* result5 = changeEmail("bob", "wrong_old@example.com", "bob_new@example.com", "bobpass");
    printf("Result: %s\n", result5);
    logout("bob");
    printf("\n");

    return 0;
}