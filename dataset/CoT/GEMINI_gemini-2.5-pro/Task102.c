/*
 * NOTE: This code requires the OpenSSL library.
 * To compile and run:
 * 1. Ensure you have OpenSSL installed (e.g., `sudo apt-get install libssl-dev` on Debian/Ubuntu).
 * 2. Compile with: gcc -o task102_c your_file_name.c -lssl -lcrypto
 * 3. Run with: ./task102_c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/rand.h>
#include <openssl/evp.h>

// --- Hashing Configuration ---
#define SALT_SIZE_BYTES 16
#define HASH_SIZE_BYTES 32
#define ITERATION_COUNT 260000

// --- Simulated Database ---
#define MAX_USERS 10

typedef struct {
    char* username;
    unsigned char* salt;
    unsigned char* hash;
} User;

// In-memory array to simulate a user database
static User user_database[MAX_USERS];
static int user_count = 0;

// Helper to print bytes as a hex string for display
void print_hex(const char* label, const unsigned char* data, int len) {
    printf("%s", label);
    for (int i = 0; i < len; ++i) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

/**
 * Hashes a password using PBKDF2 with a given salt.
 *
 * @param password The plain-text password.
 * @param salt The salt to use.
 * @param output_hash Buffer to store the resulting hash. Must be HASH_SIZE_BYTES long.
 * @return 1 on success, 0 on failure.
 */
int hash_password(const char* password, const unsigned char* salt, unsigned char* output_hash) {
    return PKCS5_PBKDF2_HMAC(
        password,
        strlen(password),
        salt,
        SALT_SIZE_BYTES,
        ITERATION_COUNT,
        EVP_sha256(),
        HASH_SIZE_BYTES,
        output_hash
    );
}

/**
 * Updates a user's password in the simulated database.
 *
 * @param username The username of the user to update.
 * @param new_password The new plain-text password.
 * @return 1 on success, 0 on failure.
 */
int update_user_password(const char* username, const char* new_password) {
    if (username == NULL || new_password == NULL || strlen(username) == 0 || strlen(new_password) == 0) {
        fprintf(stderr, "Error: Username and password cannot be null or empty.\n");
        return 0;
    }

    int user_index = -1;
    for (int i = 0; i < user_count; ++i) {
        if (strcmp(user_database[i].username, username) == 0) {
            user_index = i;
            break;
        }
    }

    if (user_index == -1) {
        fprintf(stderr, "Error: User '%s' not found.\n", username);
        return 0;
    }
    
    // Generate a new random salt
    unsigned char* new_salt = malloc(SALT_SIZE_BYTES);
    if (!new_salt) {
        fprintf(stderr, "Critical error: Failed to allocate memory for salt.\n");
        return 0;
    }
    if (RAND_bytes(new_salt, SALT_SIZE_BYTES) != 1) {
        fprintf(stderr, "Critical error: Failed to generate random salt.\n");
        free(new_salt);
        return 0;
    }
    
    // Hash the new password with the new salt
    unsigned char* new_hash = malloc(HASH_SIZE_BYTES);
    if (!new_hash) {
        fprintf(stderr, "Critical error: Failed to allocate memory for hash.\n");
        free(new_salt);
        return 0;
    }
    if (hash_password(new_password, new_salt, new_hash) != 1) {
        fprintf(stderr, "Critical error: Password hashing failed.\n");
        free(new_salt);
        free(new_hash);
        return 0;
    }

    // Free old salt and hash, then update with new ones
    free(user_database[user_index].salt);
    free(user_database[user_index].hash);
    user_database[user_index].salt = new_salt;
    user_database[user_index].hash = new_hash;

    printf("Successfully updated password for user '%s'.\n", username);
    return 1;
}

// Helper to set up an initial user for testing
void setup_initial_user(const char* username, const char* password) {
    if (user_count >= MAX_USERS) {
        printf("Database full. Cannot add user.\n");
        return;
    }
    
    unsigned char* salt = malloc(SALT_SIZE_BYTES);
    unsigned char* hash = malloc(HASH_SIZE_BYTES);
    char* uname = strdup(username);
    
    if (!salt || !hash || !uname) {
        fprintf(stderr, "Memory allocation failed for new user.\n");
        free(salt); free(hash); free(uname);
        return;
    }

    RAND_bytes(salt, SALT_SIZE_BYTES);
    if (hash_password(password, salt, hash)) {
        user_database[user_count].username = uname;
        user_database[user_count].salt = salt;
        user_database[user_count].hash = hash;
        user_count++;
    } else {
        fprintf(stderr, "Failed to setup initial user %s\n", username);
        free(salt); free(hash); free(uname);
    }
}

// Helper to free all allocated memory
void cleanup_database() {
    for (int i = 0; i < user_count; ++i) {
        free(user_database[i].username);
        free(user_database[i].salt);
        free(user_database[i].hash);
    }
}

void print_db_state() {
    printf("--- Database State ---\n");
    for (int i = 0; i < user_count; ++i) {
        printf("  '%s':\n", user_database[i].username);
        print_hex("    Salt: ", user_database[i].salt, SALT_SIZE_BYTES);
        print_hex("    Hash: ", user_database[i].hash, HASH_SIZE_BYTES);
    }
    printf("----------------------\n");
}


int main() {
    // --- Setup: Pre-populate the database ---
    setup_initial_user("user1", "oldPassword123");
    setup_initial_user("user2", "anotherOldPwd");
    print_db_state();
    
    printf("\n--- Running Test Cases ---\n");
    
    // Test Case 1: Update password for an existing user
    printf("\n[Test Case 1]: Update password for existing user 'user1'\n");
    update_user_password("user1", "newStrongPassword!@#");
    print_db_state();
    
    // Test Case 2: Attempt to update password for a non-existent user
    printf("\n[Test Case 2]: Attempt to update password for non-existent user 'unknownUser'\n");
    update_user_password("unknownUser", "somePassword");
    
    // Test Case 3: Attempt to update with an empty password
    printf("\n[Test Case 3]: Attempt to update 'user2' with an empty password\n");
    update_user_password("user2", "");
    print_db_state();

    // Test Case 4: Update password for the second user
    printf("\n[Test Case 4]: Update password for existing user 'user2'\n");
    update_user_password("user2", "updatedP@ssword456");
    print_db_state();

    // Test Case 5: Update password for the first user again
    printf("\n[Test Case 5]: Update password for 'user1' again\n");
    update_user_password("user1", "evenNewerPassword$");
    print_db_state();

    // Clean up all dynamically allocated memory before exiting
    cleanup_database();

    return 0;
}