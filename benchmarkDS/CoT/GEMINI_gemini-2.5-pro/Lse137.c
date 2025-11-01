#include <stdio.h>
#include <string.h>

// In a real application, user data would come from a secure database.
// Passwords should be securely hashed (e.g., using Argon2, bcrypt), not stored in plain text.
typedef struct {
    const char* username;
    const char* password; // NOTE: In a real app, this would be a hash.
    const char* session;
    const char* role;
} UserData;

// Mock database as a static array
static const UserData userDatabase[] = {
    {"admin", "pass123", "adminsession123", "moderator"},
    {"user", "password", "usersession456", "user"},
    {NULL, NULL, NULL, NULL} // Sentinel value to mark the end
};

/**
 * @brief Authenticates a user and checks if they have the 'moderator' role.
 * 
 * SECURITY NOTE: In a real-world scenario:
 * 1. Passwords must be compared using a secure, constant-time hash comparison function 
 *    (like `memcmp` on hashes) to prevent timing attacks. `strcmp` is not constant-time.
 * 2. This function should consult a database or a secure authentication service.
 * 3. Session tokens should be long, random, and managed securely.
 * 
 * @param username The user's name.
 * @param password The user's plain-text password.
 * @param session  The user's session token.
 * @return 1 if the user is an authenticated moderator, 0 otherwise.
 */
int auth_mod(const char* username, const char* password, const char* session) {
    if (username == NULL || password == NULL || session == NULL) {
        return 0;
    }

    for (int i = 0; userDatabase[i].username != NULL; ++i) {
        if (strcmp(userDatabase[i].username, username) == 0) {
            // NOTE: Plain text password comparison with strcmp is insecure.
            int isPasswordCorrect = (strcmp(userDatabase[i].password, password) == 0);
            int isSessionCorrect = (strcmp(userDatabase[i].session, session) == 0);
            int isModerator = (strcmp(userDatabase[i].role, "moderator") == 0);
            
            if (isPasswordCorrect && isSessionCorrect && isModerator) {
                return 1; // True
            }
            // Found user but credentials/role were wrong, so no need to check further.
            return 0; 
        }
    }
    
    return 0; // False (user not found)
}

/**
 * @brief Checks if a user is a moderator based on parameters.
 * 
 * SECURITY NOTE: Getting credentials from a URL is extremely insecure (CWE-598).
 * Sensitive data like passwords and session tokens should be sent in the body
 * of a POST request over HTTPS.
 * 
 * @param username The user's name.
 * @param password The user's plain-text password.
 * @param session  The user's session token.
 * @return 1 if the user is a moderator, 0 otherwise.
 */
int check_mod(const char* username, const char* password, const char* session) {
    return auth_mod(username, password, session);
}

int main() {
    // This class name is for consistency with the Java example, though not used in C.
    struct Lse137 {};

    printf("--- Running Test Cases ---\n");

    // Test Case 1: Valid moderator credentials
    int test1 = check_mod("admin", "pass123", "adminsession123");
    printf("Test Case 1 (Valid Moderator): %s\n", test1 ? "Passed" : "Failed");

    // Test Case 2: Valid user but not a moderator
    int test2 = check_mod("user", "password", "usersession456");
    printf("Test Case 2 (Not a Moderator): %s\n", !test2 ? "Passed" : "Failed");

    // Test Case 3: Correct username, wrong password
    int test3 = check_mod("admin", "wrongpassword", "adminsession123");
    printf("Test Case 3 (Wrong Password): %s\n", !test3 ? "Passed" : "Failed");

    // Test Case 4: Correct credentials, wrong session
    int test4 = check_mod("admin", "pass123", "wrongsession");
    printf("Test Case 4 (Wrong Session): %s\n", !test4 ? "Passed" : "Failed");

    // Test Case 5: Non-existent user
    int test5 = check_mod("unknown", "password", "somesession");
    printf("Test Case 5 (Non-existent User): %s\n", !test5 ? "Passed" : "Failed");

    return 0;
}