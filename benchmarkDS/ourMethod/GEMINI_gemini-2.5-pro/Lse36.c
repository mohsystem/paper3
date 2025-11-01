#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/crypto.h>

// Strong cryptographic parameters
#define SALT_LENGTH 16
#define KEY_LENGTH 32 // 32 bytes = 256 bits
#define ITERATION_COUNT 310000
#define MAX_USERS 10

// Represents a user record in the database
typedef struct {
    char* username;
    unsigned char* salt;
    unsigned char* hashedPassword;
} User;

// In a real application, this would be a database.
static User* userDatabase[MAX_USERS] = {NULL};
static int user_count = 0;

// Securely clears a memory buffer.
#if defined(_WIN32)
#include <windows.h>
#elif defined(__GNUC__) || defined(__clang__)
// This implementation uses a volatile pointer to prevent the compiler from optimizing it away.
static void secure_zero_memory(void* ptr, size_t size) {
    volatile unsigned char* p = (volatile unsigned char*)ptr;
    while (size--) {
        *p++ = 0;
    }
}
#else
// A portable fallback, though less guaranteed against optimization.
static void secure_zero_memory(void* ptr, size_t size) {
    memset(ptr, 0, size);
}
#endif

void cleanup_user(User* user) {
    if (user) {
        free(user->username);
        if (user->salt) {
            secure_zero_memory(user->salt, SALT_LENGTH);
            free(user->salt);
        }
        if (user->hashedPassword) {
            secure_zero_memory(user->hashedPassword, KEY_LENGTH);
            free(user->hashedPassword);
        }
        free(user);
    }
}

void cleanup_database() {
    for (int i = 0; i < user_count; ++i) {
        cleanup_user(userDatabase[i]);
        userDatabase[i] = NULL;
    }
}

int hash_password(const char* password, const unsigned char* salt, unsigned char* hash_out) {
    return PKCS5_PBKDF2_HMAC(
        password, 
        strlen(password), 
        salt, 
        SALT_LENGTH, 
        ITERATION_COUNT, 
        EVP_sha256(), 
        KEY_LENGTH, 
        hash_out
    );
}

void registerUser(const char* username, char* password) {
    if (username == NULL || strlen(username) == 0 || password == NULL || strlen(password) == 0) {
        printf("Registration failed: Username and password cannot be empty.\n");
        return;
    }
    if (user_count >= MAX_USERS) {
        printf("Registration failed: Database is full.\n");
        return;
    }

    User* newUser = malloc(sizeof(User));
    if (!newUser) {
        perror("Failed to allocate memory for user");
        return;
    }
    newUser->username = NULL;
    newUser->salt = NULL;
    newUser->hashedPassword = NULL;

    newUser->username = strdup(username);
    if (!newUser->username) {
        perror("Failed to allocate memory for username");
        goto cleanup;
    }

    newUser->salt = malloc(SALT_LENGTH);
    if (!newUser->salt) {
        perror("Failed to allocate memory for salt");
        goto cleanup;
    }

    if (RAND_bytes(newUser->salt, SALT_LENGTH) != 1) {
        fprintf(stderr, "Error generating random salt.\n");
        goto cleanup;
    }

    newUser->hashedPassword = malloc(KEY_LENGTH);
    if (!newUser->hashedPassword) {
        perror("Failed to allocate memory for hash");
        goto cleanup;
    }

    if (hash_password(password, newUser->salt, newUser->hashedPassword) != 1) {
        fprintf(stderr, "Error hashing password with PBKDF2.\n");
        goto cleanup;
    }

    userDatabase[user_count++] = newUser;
    printf("User '%s' registered successfully.\n", username);

    // Securely clear password from memory
    secure_zero_memory(password, strlen(password));
    return;

cleanup:
    cleanup_user(newUser);
    secure_zero_memory(password, strlen(password));
}

bool login(const char* username, char* password) {
    if (username == NULL || strlen(username) == 0 || password == NULL || strlen(password) == 0) {
        printf("Login failed: Username and password cannot be empty.\n");
        return false;
    }

    User* foundUser = NULL;
    for (int i = 0; i < user_count; ++i) {
        if (strcmp(userDatabase[i]->username, username) == 0) {
            foundUser = userDatabase[i];
            break;
        }
    }

    if (foundUser == NULL) {
        printf("Login failed: Invalid username or password.\n");
        secure_zero_memory(password, strlen(password));
        return false;
    }
    
    unsigned char providedPasswordHash[KEY_LENGTH];
    bool success = false;
    
    if (hash_password(password, foundUser->salt, providedPasswordHash) != 1) {
        fprintf(stderr, "Login error during hashing.\n");
        goto end;
    }

    if (CRYPTO_memcmp(providedPasswordHash, foundUser->hashedPassword, KEY_LENGTH) == 0) {
        printf("Login successful for user '%s'. Redirecting to home page...\n", username);
        success = true;
    } else {
        printf("Login failed: Invalid username or password.\n");
    }

end:
    secure_zero_memory(providedPasswordHash, KEY_LENGTH);
    secure_zero_memory(password, strlen(password));
    return success;
}

int main() {
    // We need mutable strings for password clearing
    char alice_pass[] = "Password123!";
    char bob_pass[] = "SecurePass@2024";

    registerUser("alice", alice_pass);
    registerUser("bob", bob_pass);
    
    printf("\n--- Running Test Cases ---\n");

    // Test Case 1: Successful login
    printf("\n[Test 1] Correct credentials for 'alice'\n");
    char test1_pass[] = "Password123!";
    login("alice", test1_pass);

    // Test Case 2: Incorrect password
    printf("\n[Test 2] Incorrect password for 'alice'\n");
    char test2_pass[] = "wrongpassword";
    login("alice", test2_pass);

    // Test Case 3: Non-existent user
    printf("\n[Test 3] Non-existent user 'charlie'\n");
    char test3_pass[] = "anypassword";
    login("charlie", test3_pass);

    // Test Case 4: Successful login for another user
    printf("\n[Test 4] Correct credentials for 'bob'\n");
    char test4_pass[] = "SecurePass@2024";
    login("bob", test4_pass);
    
    // Test Case 5: Empty password
    printf("\n[Test 5] Empty password for 'alice'\n");
    char test5_pass[] = "";
    login("alice", test5_pass);

    cleanup_database();

    return 0;
}