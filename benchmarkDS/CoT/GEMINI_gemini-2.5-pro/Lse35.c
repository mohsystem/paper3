#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// ========================== SECURITY WARNING ==========================
// The following code is a SIMULATION for educational purposes only.
// Standard C does NOT have a built-in, standard library for
// cryptography. The hashing and memory management used here are simplified
// and are NOT cryptographically secure or robust.
//
// In a REAL-WORLD application, you MUST use a well-vetted, reputable
// third-party cryptography library like OpenSSL or libsodium to handle
// password hashing and random salt generation.
//
// DO NOT USE THIS CODE IN PRODUCTION.
// ====================================================================

#define MAX_USERS 10
#define MAX_USERNAME_LEN 50
#define MAX_STORED_LEN 100

typedef struct {
    char username[MAX_USERNAME_LEN];
    char stored_salt_and_hash[MAX_STORED_LEN];
} User;

User user_database[MAX_USERS];
int user_count = 0;

// FAKE salt for simulation. NOT SECURE.
const char* FAKE_SALT = "a1b2c3d4";

// FAKE password hasher for simulation. NOT SECURE.
// Hashes password+salt and stores the result in output_hash.
void fake_hash_password(const char* password, const char* salt, char* output_hash, size_t output_size) {
    char salted_password[256];
    snprintf(salted_password, sizeof(salted_password), "%s%s", salt, password);
    
    unsigned long hash = 5381;
    int i = 0;
    while (salted_password[i] != '\0') {
        hash = ((hash << 5) + hash) + salted_password[i];
        i++;
    }
    snprintf(output_hash, output_size, "%lu", hash);
}

// Simulates user registration.
void register_user(const char* username, const char* password) {
    if (user_count >= MAX_USERS) {
        printf("Database full.\n");
        return;
    }
    char hashed_password[MAX_STORED_LEN];
    fake_hash_password(password, FAKE_SALT, hashed_password, sizeof(hashed_password));
    
    strncpy(user_database[user_count].username, username, MAX_USERNAME_LEN - 1);
    user_database[user_count].username[MAX_USERNAME_LEN - 1] = '\0';

    snprintf(user_database[user_count].stored_salt_and_hash, MAX_STORED_LEN, "%s:%s", FAKE_SALT, hashed_password);
    user_count++;
}

// Constant-time string comparison to mitigate timing attacks.
int secure_compare(const char* a, const char* b) {
    size_t len_a = strlen(a);
    if (len_a != strlen(b)) {
        return 0; // Not a match
    }
    int result = 0;
    for (size_t i = 0; i < len_a; ++i) {
        result |= (a[i] ^ b[i]);
    }
    return result == 0; // Returns 1 if match, 0 if not
}

/**
 * Authenticates a user by checking their username and password.
 * @param username The username provided by the user.
 * @param password The password provided by the user.
 * @return 1 if authentication is successful, 0 otherwise.
 */
int login(const char* username, const char* password) {
    User* found_user = NULL;
    // 1. Find user in the database.
    for (int i = 0; i < user_count; ++i) {
        if (strcmp(user_database[i].username, username) == 0) {
            found_user = &user_database[i];
            break;
        }
    }

    if (found_user == NULL) {
        // To mitigate timing attacks, perform a dummy hash.
        char dummy_hash[MAX_STORED_LEN];
        fake_hash_password(password, FAKE_SALT, dummy_hash, sizeof(dummy_hash));
        return 0;
    }

    char stored_salt_and_hash[MAX_STORED_LEN];
    strncpy(stored_salt_and_hash, found_user->stored_salt_and_hash, MAX_STORED_LEN - 1);
    stored_salt_and_hash[MAX_STORED_LEN - 1] = '\0';
    
    // 2. Extract salt and stored hash.
    char* salt = strtok(stored_salt_and_hash, ":");
    char* stored_hash = strtok(NULL, ":");

    if (salt == NULL || stored_hash == NULL) {
        return 0; // Data integrity issue
    }

    // 3. Hash the provided password with the retrieved salt.
    char provided_password_hash[MAX_STORED_LEN];
    fake_hash_password(password, salt, provided_password_hash, sizeof(provided_password_hash));

    // 4. Compare the hashes securely.
    return secure_compare(stored_hash, provided_password_hash);
}

int main() {
    // Pre-populate the database
    register_user("admin", "AdminPass123!");
    register_user("user1", "UserPass@456");
    register_user("testuser", "password");

    // --- 5 Test Cases ---
    printf("--- Test Cases ---\n");

    // Test Case 1: Successful login for admin
    printf("\n1. Testing with correct admin credentials...\n");
    int isAdminLoggedIn = login("admin", "AdminPass123!");
    printf("Login success: %s\n", isAdminLoggedIn ? "true" : "false");
    printf("%s\n", isAdminLoggedIn ? "Redirecting to home page." : "Redirecting to login page.");

    // Test Case 2: Successful login for user1
    printf("\n2. Testing with correct user1 credentials...\n");
    int isUser1LoggedIn = login("user1", "UserPass@456");
    printf("Login success: %s\n", isUser1LoggedIn ? "true" : "false");
    printf("%s\n", isUser1LoggedIn ? "Redirecting to home page." : "Redirecting to login page.");

    // Test Case 3: Failed login with wrong password
    printf("\n3. Testing with incorrect password for admin...\n");
    int isWrongPassLoggedIn = login("admin", "wrongpassword");
    printf("Login success: %s\n", isWrongPassLoggedIn ? "true" : "false");
    printf("%s\n", isWrongPassLoggedIn ? "Redirecting to home page." : "Redirecting to login page.");

    // Test Case 4: Failed login with non-existent user
    printf("\n4. Testing with non-existent username...\n");
    int isNonExistentUserLoggedIn = login("unknownuser", "somepassword");
    printf("Login success: %s\n", isNonExistentUserLoggedIn ? "true" : "false");
    printf("%s\n", isNonExistentUserLoggedIn ? "Redirecting to home page." : "Redirecting to login page.");

    // Test Case 5: Failed login with empty password
    printf("\n5. Testing with empty password for user1...\n");
    int isEmptyPassLoggedIn = login("user1", "");
    printf("Login success: %s\n", isEmptyPassLoggedIn ? "true" : "false");
    printf("%s\n", isEmptyPassLoggedIn ? "Redirecting to home page." : "Redirecting to login page.");

    return 0;
}