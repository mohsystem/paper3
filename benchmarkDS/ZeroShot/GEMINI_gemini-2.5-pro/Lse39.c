#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_USERNAME_LEN 32
#define MAX_PASSWORD_LEN 64
#define SALT_LEN 16
// Placeholder hash length is salt + password
#define HASH_LEN (SALT_LEN + MAX_PASSWORD_LEN)
#define MAX_USERS 10

// ========================== SECURITY WARNING ============================
// The hashing functions used here are PLACEHOLDERS and are NOT SECURE.
// They are for demonstration purposes only to fit the single-file requirement.
// In a real-world application, you MUST use a well-vetted, standard
// password hashing library like Argon2, scrypt, or bcrypt, linked
// from a library like libsodium or OpenSSL.
// NEVER store passwords in plaintext or with simple hash functions.
// ========================================================================

typedef struct {
    char username[MAX_USERNAME_LEN];
    char salt[SALT_LEN];
    char hashedPassword[HASH_LEN];
} UserData;

// In a real application, this would be a database.
static UserData userDatabase[MAX_USERS];
static int userCount = 0;

/**
 * @brief Placeholder salt generator. In a real app, use a cryptographic PRNG.
 */
void generate_salt(char* salt_buffer) {
    // For demonstration, we use a fixed but seemingly random string.
    const char* s = "abcdefghijklmnopqrstuvwxyz0123456789";
    for(int i = 0; i < SALT_LEN; ++i) {
        salt_buffer[i] = s[i % strlen(s)];
    }
}

/**
 * @brief INSECURE placeholder hash function.
 */
void insecure_placeholder_hash(const char* password, const char* salt, char* hash_buffer) {
    // THIS IS NOT A REAL HASH FUNCTION!
    // It simply concatenates the salt and password.
    // Replace this with a call to a real password hashing function.
    strncpy(hash_buffer, salt, SALT_LEN);
    strncat(hash_buffer, password, MAX_PASSWORD_LEN);
}

/**
 * @brief Performs a constant-time comparison of two memory buffers.
 * It compares all bytes to prevent timing side-channel attacks.
 * @return true if buffers are equal, false otherwise.
 */
bool constant_time_compare(const char* a, const char* b, size_t len) {
    int result = 0;
    for (size_t i = 0; i < len; ++i) {
        result |= a[i] ^ b[i];
    }
    return result == 0;
}

void registerUser(const char* username, const char* password) {
    if (userCount >= MAX_USERS) {
        printf("Registration failed: Database is full.\n");
        return;
    }
    if (username == NULL || password == NULL || strlen(username) == 0 || strlen(password) == 0 ||
        strlen(username) >= MAX_USERNAME_LEN || strlen(password) >= MAX_PASSWORD_LEN) {
        printf("Registration failed: Invalid username or password.\n");
        return;
    }
    
    UserData* newUser = &userDatabase[userCount];
    strncpy(newUser->username, username, MAX_USERNAME_LEN - 1);
    newUser->username[MAX_USERNAME_LEN - 1] = '\0'; // Ensure null termination
    
    generate_salt(newUser->salt);
    insecure_placeholder_hash(password, newUser->salt, newUser->hashedPassword);
    
    userCount++;
}

bool login(const char* username, const char* password) {
    if (username == NULL || password == NULL || strlen(username) == 0 || strlen(password) == 0) {
        return false;
    }
    
    UserData* foundUser = NULL;
    for (int i = 0; i < userCount; ++i) {
        if (strncmp(userDatabase[i].username, username, MAX_USERNAME_LEN) == 0) {
            foundUser = &userDatabase[i];
            break;
        }
    }

    if (foundUser == NULL) {
        // To prevent timing attacks, perform a dummy hash calculation.
        char dummy_salt[SALT_LEN];
        char dummy_hash[HASH_LEN];
        generate_salt(dummy_salt);
        insecure_placeholder_hash(password, dummy_salt, dummy_hash);
        return false;
    }
    
    char providedPasswordHash[HASH_LEN] = {0};
    insecure_placeholder_hash(password, foundUser->salt, providedPasswordHash);

    // Use strlen of stored hash to avoid comparing padding bytes
    size_t hash_len = strlen(foundUser->hashedPassword);
    if (strlen(providedPasswordHash) != hash_len) {
        return false; // Hashes of different lengths can't match
    }
    
    return constant_time_compare(foundUser->hashedPassword, providedPasswordHash, hash_len);
}

int main() {
    // "Register" a user for testing
    registerUser("admin", "S3cureP@ssword!123");
    
    printf("--- Running 5 Test Cases ---\n");

    // Test Case 1: Correct username and correct password
    printf("Test Case 1 (Correct credentials): ");
    bool success1 = login("admin", "S3cureP@ssword!123");
    printf("%s\n", success1 ? "Login successful. Redirecting to session page." : "Invalid username or password.");

    // Test Case 2: Correct username and incorrect password
    printf("Test Case 2 (Incorrect password): ");
    bool success2 = login("admin", "wrongpassword");
    printf("%s\n", success2 ? "Login successful. Redirecting to session page." : "Invalid username or password.");

    // Test Case 3: Incorrect username
    printf("Test Case 3 (Incorrect username): ");
    bool success3 = login("guest", "S3cureP@ssword!123");
    printf("%s\n", success3 ? "Login successful. Redirecting to session page." : "Invalid username or password.");

    // Test Case 4: Correct username, empty password
    printf("Test Case 4 (Empty password): ");
    bool success4 = login("admin", "");
    printf("%s\n", success4 ? "Login successful. Redirecting to session page." : "Invalid username or password.");

    // Test Case 5: Null input
    printf("Test Case 5 (Null username): ");
    bool success5 = login(NULL, "S3cureP@ssword!123");
    printf("%s\n", success5 ? "Login successful. Redirecting to session page." : "Invalid username or password.");

    return 0;
}