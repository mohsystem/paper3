#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// --- IMPORTANT ---
// This code requires the OpenSSL library.
// To compile on Linux/macOS, use:
// gcc your_file_name.c -o your_program -lssl -lcrypto
#include <openssl/evp.h>
#include <openssl/rand.h>

// Constants for hashing
#define SALT_BYTES 16
#define HASH_BYTES 32 // 256 bits for SHA256
#define HASH_ITERATIONS 260000
#define MAX_USERS 10

// Struct to hold user data
typedef struct {
    char* username;
    unsigned char* salt;
    unsigned char* hashedPassword;
} UserData;

// A simple global array to simulate a database.
// In a real application, use a proper database.
static UserData userDatabase[MAX_USERS];
static int userCount = 0;

/**
 * Converts a byte buffer to a hexadecimal string for printing.
 * The caller must free the returned string.
 */
char* to_hex(const unsigned char* data, size_t len) {
    char* hex_str = (char*)malloc(len * 2 + 1);
    if (!hex_str) return NULL;
    for (size_t i = 0; i < len; ++i) {
        sprintf(hex_str + i * 2, "%02x", data[i]);
    }
    hex_str[len * 2] = '\0';
    return hex_str;
}

/**
 * Cleans up all dynamically allocated memory in the database.
 */
void cleanup_database() {
    for (int i = 0; i < userCount; ++i) {
        free(userDatabase[i].username);
        free(userDatabase[i].salt);
        free(userDatabase[i].hashedPassword);
    }
}

/**
 * "Inserts" a new user into the database after hashing their password.
 * @param username The username.
 * @param password The plaintext password.
 */
void register_user(const char* username, const char* password) {
    if (!username || !password || strlen(username) == 0 || strlen(password) == 0) {
        fprintf(stderr, "Error: Username and password cannot be empty.\n");
        return;
    }

    if (userCount >= MAX_USERS) {
        fprintf(stderr, "Error: Database is full.\n");
        return;
    }

    for (int i = 0; i < userCount; ++i) {
        if (strcmp(userDatabase[i].username, username) == 0) {
            fprintf(stderr, "Error: Username '%s' already exists.\n", username);
            return;
        }
    }

    // 1. Generate a cryptographically secure random salt
    unsigned char* salt = (unsigned char*)malloc(SALT_BYTES);
    if (!salt) {
        fprintf(stderr, "Error: Failed to allocate memory for salt.\n");
        return;
    }
    if (RAND_bytes(salt, SALT_BYTES) != 1) {
        fprintf(stderr, "Error: Failed to generate random salt.\n");
        free(salt);
        return;
    }

    // 2. Hash the password using PBKDF2
    unsigned char* hashedPassword = (unsigned char*)malloc(HASH_BYTES);
    if (!hashedPassword) {
        fprintf(stderr, "Error: Failed to allocate memory for hash.\n");
        free(salt);
        return;
    }
    int success = PKCS5_PBKDF2_HMAC(
        password,
        strlen(password),
        salt,
        SALT_BYTES,
        HASH_ITERATIONS,
        EVP_sha256(),
        HASH_BYTES,
        hashedPassword
    );

    if (success != 1) {
        fprintf(stderr, "Error: Failed to hash password.\n");
        free(salt);
        free(hashedPassword);
        return;
    }

    // 3. Simulate inserting into the database
    userDatabase[userCount].username = strdup(username);
    userDatabase[userCount].salt = salt;
    userDatabase[userCount].hashedPassword = hashedPassword;
    userCount++;

    printf("User '%s' registered successfully.\n", username);
}

void print_database_state() {
    printf("\n--- Simulated Database State ---\n");
    for (int i = 0; i < userCount; ++i) {
        char* salt_hex = to_hex(userDatabase[i].salt, SALT_BYTES);
        char* hash_hex = to_hex(userDatabase[i].hashedPassword, HASH_BYTES);
        
        printf("'%s':\n", userDatabase[i].username);
        printf("  Salt:            %s\n", salt_hex ? salt_hex : "N/A");
        printf("  Hashed Password: %s\n", hash_hex ? hash_hex : "N/A");
        
        free(salt_hex);
        free(hash_hex);
    }
}

int main() {
    // --- 5 Test Cases ---
    printf("--- Running 5 Test Cases ---\n");

    register_user("alice", "Password123!");
    register_user("bob", "My$trongP@ssw0rd");
    register_user("charlie", "Ch@rlieBrown");
    register_user("diana", "Secr3t-P@ss");
    register_user("eve", "password");

    print_database_state();

    // Clean up all allocated memory before exiting
    cleanup_database();
    
    return 0;
}