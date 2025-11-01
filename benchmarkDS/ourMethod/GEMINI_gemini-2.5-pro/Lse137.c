#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

// WARNING: This example uses a simplified hashing and random number simulation
// for demonstration purposes. A single C file cannot easily include robust
// cryptographic libraries like OpenSSL or libsodium without external dependencies.
// IN A REAL-WORLD APPLICATION, NEVER IMPLEMENT YOUR OWN CRYPTOGRAPHY.
// Use a vetted library like OpenSSL (EVP_BytesToKey, PKCS5_PBKDF2_HMAC) or libsodium.
// For random data, use the OS's CSPRNG (e.g., /dev/urandom on Linux).

#define MAX_USERS 10
#define MAX_SESSIONS 10
#define USERNAME_LEN 32
#define PASSWORD_HASH_LEN 65 // SHA-256 hex string length + 1
#define SALT_LEN 17 // 16 bytes + null terminator
#define ROLE_LEN 16
#define SESSION_TOKEN_LEN 64

typedef struct {
    char username[USERNAME_LEN];
    char passwordHash[PASSWORD_HASH_LEN];
    char salt[SALT_LEN];
    char role[ROLE_LEN];
} User;

typedef struct {
    char token[SESSION_TOKEN_LEN];
    char username[USERNAME_LEN];
} Session;

// Mock database and session store. Use secure storage in a real app.
static User userDatabase[MAX_USERS];
static Session sessionStore[MAX_SESSIONS];
static int userCount = 0;
static int sessionCount = 0;

/**
 * SIMULATED random salt generation.
 * THIS IS NOT SECURE. Use a CSPRNG in a real application.
 */
void generate_salt(char* salt_buffer) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for (int i = 0; i < SALT_LEN - 1; ++i) {
        salt_buffer[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    salt_buffer[SALT_LEN - 1] = '\0';
}

/**
 * SIMULATED password hashing.
 * THIS IS NOT SECURE. Use PBKDF2, scrypt, or Argon2 from a crypto library.
 */
void hash_password(const char* password, const char* salt, char* hash_buffer) {
    char temp[256];
    snprintf(temp, sizeof(temp), "%s%s", salt, password);
    
    // This is a placeholder for a real hash function (e.g., SHA-256)
    // Here we just use a simple string transformation as a stand-in.
    size_t len = strlen(temp);
    for (size_t i = 0; i < len && i < PASSWORD_HASH_LEN - 1; ++i) {
        hash_buffer[i] = (temp[i] % 16) + 'a';
    }
    hash_buffer[len < PASSWORD_HASH_LEN - 1 ? len : PASSWORD_HASH_LEN - 1] = '\0';
}

/**
 * Constant-time string comparison to prevent timing attacks.
 */
bool timing_safe_compare(const char* a, const char* b) {
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

void setup_mock_data() {
    // Add moderator
    generate_salt(userDatabase[userCount].salt);
    hash_password("ModPassword123!", userDatabase[userCount].salt, userDatabase[userCount].passwordHash);
    strncpy(userDatabase[userCount].username, "moderator", USERNAME_LEN - 1);
    strncpy(userDatabase[userCount].role, "moderator", ROLE_LEN - 1);
    userCount++;

    strncpy(sessionStore[sessionCount].token, "SESSION_MOD_VALID", SESSION_TOKEN_LEN - 1);
    strncpy(sessionStore[sessionCount].username, "moderator", USERNAME_LEN - 1);
    sessionCount++;

    // Add regular user
    generate_salt(userDatabase[userCount].salt);
    hash_password("UserPassword456$", userDatabase[userCount].salt, userDatabase[userCount].passwordHash);
    strncpy(userDatabase[userCount].username, "regular_user", USERNAME_LEN - 1);
    strncpy(userDatabase[userCount].role, "user", ROLE_LEN - 1);
    userCount++;

    strncpy(sessionStore[sessionCount].token, "SESSION_USER_VALID", SESSION_TOKEN_LEN - 1);
    strncpy(sessionStore[sessionCount].username, "regular_user", USERNAME_LEN - 1);
    sessionCount++;
}

bool auth_mod(const char* username, const char* password, const char* session) {
    if (username == NULL || *username == '\0' || password == NULL || *password == '\0' || session == NULL || *session == '\0') {
        return false;
    }

    bool session_valid = false;
    for (int i = 0; i < sessionCount; ++i) {
        if (strcmp(sessionStore[i].token, session) == 0 && strcmp(sessionStore[i].username, username) == 0) {
            session_valid = true;
            break;
        }
    }
    if (!session_valid) return false;

    User* user = NULL;
    for (int i = 0; i < userCount; ++i) {
        if (strcmp(userDatabase[i].username, username) == 0) {
            user = &userDatabase[i];
            break;
        }
    }
    if (user == NULL) return false;

    char provided_hash[PASSWORD_HASH_LEN];
    hash_password(password, user->salt, provided_hash);
    
    if (!timing_safe_compare(provided_hash, user->passwordHash)) {
        return false;
    }

    return strcmp(user->role, "moderator") == 0;
}

void check_mod(const char* username, const char* password, const char* session) {
    bool is_moderator = auth_mod(username, password, session);
    printf("User '%s' is moderator: %s\n", username, is_moderator ? "true" : "false");
}

int main() {
    // Seed for simulated random salt generation. Use a better source in production.
    srand((unsigned int)time(NULL));
    setup_mock_data();

    printf("--- Running 5 Test Cases ---\n");

    // Test Case 1: Successful moderator login
    printf("Test 1 (Success - Moderator): ");
    check_mod("moderator", "ModPassword123!", "SESSION_MOD_VALID");

    // Test Case 2: Successful login, but user is not a moderator
    printf("Test 2 (Failure - Not a Moderator): ");
    check_mod("regular_user", "UserPassword456$", "SESSION_USER_VALID");

    // Test Case 3: Login with a wrong password
    printf("Test 3 (Failure - Wrong Password): ");
    check_mod("moderator", "WrongPassword", "SESSION_MOD_VALID");

    // Test Case 4: Login with a non-existent user
    printf("Test 4 (Failure - User Not Found): ");
    check_mod("unknown_user", "any_password", "SESSION_MOD_VALID");

    // Test Case 5: Login with an invalid session token
    printf("Test 5 (Failure - Invalid Session): ");
    check_mod("moderator", "ModPassword123!", "INVALID_SESSION");

    printf("--- Test Cases Finished ---\n");

    return 0;
}