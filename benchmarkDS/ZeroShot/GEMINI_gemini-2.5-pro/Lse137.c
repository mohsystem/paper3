#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define MAX_USERS 10
#define MAX_SESSIONS 10
#define MAX_STRING_LEN 64
#define SESSION_TOKEN_LEN 32

// --- Simulated Database ---

typedef struct {
    char username[MAX_STRING_LEN];
    char hashedPassword[MAX_STRING_LEN];
    char role[MAX_STRING_LEN];
} User;

typedef struct {
    char token[MAX_STRING_LEN];
    char username[MAX_STRING_LEN];
} Session;

User userDatabase[MAX_USERS];
int userCount = 0;

Session sessionDatabase[MAX_SESSIONS];
int sessionCount = 0;

// --- Security Utilities ---

// SECURITY NOTE: This is a *demonstration* hash function. In a real application,
// use a strong, salted, and standardized library like OpenSSL for hashing,
// and algorithms like BCrypt or Argon2. This is NOT secure.
void hashPassword(const char* password, char* output) {
    // Simple non-secure hash for demonstration
    unsigned long hash = 5381;
    int c;
    while ((c = *password++)) {
        hash = ((hash << 5) + hash) + c;
    }
    snprintf(output, MAX_STRING_LEN, "%lu", hash);
}

// SECURITY NOTE: rand() is not cryptographically secure. For a real application,
// use a proper source of randomness like /dev/urandom on Linux or CryptGenRandom on Windows.
void generateSessionToken(char* token) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for (int i = 0; i < SESSION_TOKEN_LEN; ++i) {
        int key = rand() % (sizeof(charset) - 1);
        token[i] = charset[key];
    }
    token[SESSION_TOKEN_LEN] = '\0';
}

// Constant-time string comparison to help prevent timing attacks.
bool secure_compare(const char* a, const char* b) {
    if (a == NULL || b == NULL) return false;
    size_t len_a = strlen(a);
    size_t len_b = strlen(b);
    if (len_a != len_b) {
        return false;
    }
    int result = 0;
    for (size_t i = 0; i < len_a; ++i) {
        result |= a[i] ^ b[i];
    }
    return result == 0;
}

// --- Simulated Authentication and Authorization Logic ---

const char* login(const char* username, const char* password) {
    char providedPasswordHash[MAX_STRING_LEN];
    hashPassword(password, providedPasswordHash);

    for (int i = 0; i < userCount; ++i) {
        if (strcmp(userDatabase[i].username, username) == 0) {
            if (secure_compare(userDatabase[i].hashedPassword, providedPasswordHash)) {
                if (sessionCount < MAX_SESSIONS) {
                    generateSessionToken(sessionDatabase[sessionCount].token);
                    strncpy(sessionDatabase[sessionCount].username, username, MAX_STRING_LEN - 1);
                    sessionDatabase[sessionCount].username[MAX_STRING_LEN-1] = '\0';
                    return sessionDatabase[sessionCount++].token;
                }
                return NULL; // No more session slots
            }
        }
    }
    return NULL; // Auth failed
}

bool auth_mod(const char* session) {
    if (session == NULL || *session == '\0') {
        return false;
    }
    for (int i = 0; i < sessionCount; ++i) {
        if (strcmp(sessionDatabase[i].token, session) == 0) {
            // Session found, now find the user
            for (int j = 0; j < userCount; ++j) {
                if (strcmp(sessionDatabase[i].username, userDatabase[j].username) == 0) {
                    return strcmp(userDatabase[j].role, "moderator") == 0;
                }
            }
        }
    }
    return false;
}

bool check_mod(const char* username, const char* password, const char* session) {
    // SECURITY NOTE: After login, authorization should be based on a session token,
    // not by re-submitting credentials. The username and password parameters are
    // ignored here to follow secure design principles.
    return auth_mod(session);
}

// --- Main function with Test Cases ---

void addUser(const char* username, const char* password, const char* role) {
    if (userCount < MAX_USERS) {
        strncpy(userDatabase[userCount].username, username, MAX_STRING_LEN - 1);
        userDatabase[userCount].username[MAX_STRING_LEN-1] = '\0';
        hashPassword(password, userDatabase[userCount].hashedPassword);
        strncpy(userDatabase[userCount].role, role, MAX_STRING_LEN - 1);
        userDatabase[userCount].role[MAX_STRING_LEN-1] = '\0';
        userCount++;
    }
}

int main() {
    srand((unsigned int)time(NULL)); // Seed random number generator

    // 1. Setup: Populate user database
    addUser("admin", "adminpass123", "moderator");
    addUser("bob", "bobspassword", "user");
    addUser("charlie", "charP@ss!", "moderator");

    // 2. Simulate logins to get session tokens
    const char* adminSession = login("admin", "adminpass123");
    const char* bobSession = login("bob", "bobspassword");
    const char* failedLoginSession = login("admin", "wrongpassword"); // Will be NULL

    printf("--- Test Cases for check_mod() ---\n");

    // Test Case 1: Valid moderator user
    bool test1 = check_mod("admin", "adminpass123", adminSession);
    printf("Test Case 1 (Admin is moderator): %s\n", test1 ? "Passed" : "Failed");

    // Test Case 2: Valid non-moderator user
    bool test2 = check_mod("bob", "bobspassword", bobSession);
    printf("Test Case 2 (Bob is not moderator): %s\n", !test2 ? "Passed" : "Failed");

    // Test Case 3: Invalid session token
    bool test3 = check_mod("guest", "pass", "invalid-session-token-12345");
    printf("Test Case 3 (Invalid session): %s\n", !test3 ? "Passed" : "Failed");

    // Test Case 4: Null session token (from failed login)
    bool test4 = check_mod("admin", "wrongpassword", failedLoginSession);
    printf("Test Case 4 (Null session from failed login): %s\n", !test4 ? "Passed" : "Failed");

    // Test Case 5: Empty session token
    bool test5 = check_mod("user", "pass", "");
    printf("Test Case 5 (Empty session): %s\n", !test5 ? "Passed" : "Failed");

    return 0;
}