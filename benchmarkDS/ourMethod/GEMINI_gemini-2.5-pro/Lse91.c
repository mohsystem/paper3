// To compile: gcc your_file_name.c -o your_program -lssl -lcrypto
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

// Constants for password hashing
#define SALT_LEN 16
#define HASH_LEN 32
// Iteration count should be high. OWASP recommends at least 310,000 for PBKDF2-SHA256.
#define ITERATIONS 310000

// In-memory database simulation
#define MAX_USERS 10
#define MAX_USERNAME_LEN 21 // 20 chars + null terminator
#define MAX_PASSWORD_LEN 101 // 100 chars + null terminator
#define STORED_HASH_LEN 256

typedef struct {
    char username[MAX_USERNAME_LEN];
    char stored_password[STORED_HASH_LEN];
} User;

User* user_database[MAX_USERS];
int user_count = 0;

// Portable function to securely zero memory
void secure_zero_memory(void *v, size_t n) {
    volatile unsigned char *p = v;
    while (n--) {
        *p++ = 0;
    }
}

// Helper to convert binary to hex string. Caller must free the returned string.
char* to_hex(const unsigned char* data, size_t len) {
    char* hex_str = (char*)malloc(len * 2 + 1);
    if (!hex_str) return NULL;
    for (size_t i = 0; i < len; i++) {
        sprintf(hex_str + i * 2, "%02x", data[i]);
    }
    hex_str[len * 2] = '\0';
    return hex_str;
}

bool registerUser(const char* username, char* password) {
    // Rule #1: Input Validation
    if (username == NULL || strlen(username) < 3 || strlen(username) >= MAX_USERNAME_LEN) {
        fprintf(stderr, "Registration failed: Invalid username length.\n");
        return false;
    }
    if (password == NULL || strlen(password) < 8 || strlen(password) >= MAX_PASSWORD_LEN) {
        fprintf(stderr, "Registration failed: Invalid password length.\n");
        return false;
    }

    if (user_count >= MAX_USERS) {
        fprintf(stderr, "Registration failed: Database is full.\n");
        return false;
    }

    for (int i = 0; i < user_count; i++) {
        if (strcmp(user_database[i]->username, username) == 0) {
            fprintf(stderr, "Registration failed: Username '%s' already exists.\n", username);
            return false;
        }
    }

    unsigned char salt[SALT_LEN];
    unsigned char hash[HASH_LEN];

    // Rule #9 & #10: Generate a unique, unpredictable salt
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        fprintf(stderr, "Critical error: Failed to generate random salt.\n");
        return false;
    }

    // Rule #8: Use a strong key derivation function
    if (PKCS5_PBKDF2_HMAC(password, strlen(password), salt, sizeof(salt), ITERATIONS, EVP_sha256(), sizeof(hash), hash) != 1) {
        fprintf(stderr, "Critical error: Failed to hash password with PBKDF2.\n");
        return false;
    }

    User* new_user = (User*)malloc(sizeof(User));
    if (!new_user) {
        fprintf(stderr, "Critical error: Memory allocation failed.\n");
        return false;
    }

    char* salt_hex = to_hex(salt, sizeof(salt));
    char* hash_hex = to_hex(hash, sizeof(hash));
    if (!salt_hex || !hash_hex) {
        fprintf(stderr, "Critical error: Memory allocation for hex conversion failed.\n");
        free(salt_hex);
        free(hash_hex);
        free(new_user);
        return false;
    }
    
    // Store parameters with the hash for future verification
    int written = snprintf(new_user->stored_password, STORED_HASH_LEN, "pbkdf2_sha256:%d:%s:%s", ITERATIONS, salt_hex, hash_hex);
    if (written < 0 || written >= STORED_HASH_LEN) {
        fprintf(stderr, "Critical error: Stored hash buffer is too small.\n");
        free(salt_hex);
        free(hash_hex);
        free(new_user);
        return false;
    }
    
    strncpy(new_user->username, username, MAX_USERNAME_LEN -1);
    new_user->username[MAX_USERNAME_LEN-1] = '\0';
    
    user_database[user_count++] = new_user;
    
    printf("User '%s' registered successfully.\n", username);
    
    free(salt_hex);
    free(hash_hex);
    // Securely clear sensitive data
    secure_zero_memory(password, strlen(password));
    secure_zero_memory(hash, sizeof(hash));
    
    return true;
}

void cleanup() {
    for (int i = 0; i < user_count; i++) {
        free(user_database[i]);
    }
}

int main() {
    printf("--- User Registration Simulation ---\n");
    
    char pass1[] = "ValidPass1!LongEnough";
    char pass2[] = "Another_Good_P@ssw0rd";
    char pass3[] = "SomeOtherPass1?ForAlice";
    char pass4[] = "ValidPass1!ForInvalid";
    char pass5[] = "weakpasswordbutlong";

    // Test Case 1: Successful registration
    registerUser("alice_123", pass1);

    // Test Case 2: Successful registration with another user
    registerUser("bob_smith", pass2);

    // Test Case 3: Failed registration - duplicate username
    registerUser("alice_123", pass3);
    
    // Test Case 4: Failed registration - username too long
    registerUser("a_very_long_username_that_is_invalid", pass4);
    
    // Test Case 5: Successful registration (password policy is just length in C example for simplicity)
    registerUser("charlie", pass5);

    printf("\n--- Simulated User Database ---\n");
    for (int i = 0; i < user_count; i++) {
        printf("Username: %s\n", user_database[i]->username);
        printf("Stored Hash: %s\n\n", user_database[i]->stored_password);
    }

    cleanup();
    return 0;
}