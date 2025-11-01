#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/crypto.h> // For CRYPTO_memcmp and OPENSSL_cleanse

// Configuration for PBKDF2
#define ITERATIONS 310000
#define SALT_SIZE 16
#define KEY_LENGTH 32 // 256 bits

// This is a simple simulation. A real application would use a proper database.
#define MAX_USERS 10
#define MAX_USERNAME_LEN 50
#define MAX_STORED_PASS_LEN 256
struct User {
    char username[MAX_USERNAME_LEN];
    char storedPassword[MAX_STORED_PASS_LEN];
};
struct User userDatabase[MAX_USERS];
int userCount = 0;

void handle_openssl_error() {
    // In a real application, log this to a secure location, don't print to stderr for end users.
    // ERR_print_errors_fp(stderr);
    fprintf(stderr, "An internal security error occurred.\n");
}

// Returns a dynamically allocated string that must be freed by the caller.
char* base64_encode(const unsigned char* data, size_t data_len) {
    size_t encoded_len_alloc = 4 * ((data_len + 2) / 3) + 1;
    char* encoded_data = (char*)malloc(encoded_len_alloc);
    if (!encoded_data) {
        perror("malloc failed");
        return NULL;
    }
    
    int encoded_len = EVP_EncodeBlock((unsigned char*)encoded_data, data, data_len);
    if (encoded_len <= 0) {
        handle_openssl_error();
        free(encoded_data);
        return NULL;
    }
    encoded_data[encoded_len] = '\0';
    return encoded_data;
}

// Returns a dynamically allocated buffer and its size. The buffer must be freed by the caller.
unsigned char* base64_decode(const char* input, size_t* out_len) {
    size_t input_len = strlen(input);
    unsigned char* decoded_data = (unsigned char*)malloc(input_len);
    if (!decoded_data) {
        perror("malloc failed");
        return NULL;
    }
    
    int decoded_len = EVP_DecodeBlock(decoded_data, (const unsigned char*)input, input_len);
    if (decoded_len < 0) {
        handle_openssl_error();
        free(decoded_data);
        return NULL;
    }
    *out_len = decoded_len;
    return decoded_data;
}


// Returns a dynamically allocated string that must be freed by the caller.
char* registerUser(const char* username, const char* password) {
    unsigned char salt[SALT_SIZE];
    if (RAND_bytes(salt, SALT_SIZE) != 1) {
        handle_openssl_error();
        return NULL;
    }

    unsigned char hashedPassword[KEY_LENGTH];
    if (PKCS5_PBKDF2_HMAC(password, strlen(password), salt, SALT_SIZE, ITERATIONS, EVP_sha256(), KEY_LENGTH, hashedPassword) == 0) {
        handle_openssl_error();
        return NULL;
    }

    char* salt_b64 = base64_encode(salt, SALT_SIZE);
    char* hash_b64 = base64_encode(hashedPassword, KEY_LENGTH);

    if (!salt_b64 || !hash_b64) {
        free(salt_b64);
        free(hash_b64);
        return NULL;
    }
    
    size_t stored_len = strlen(salt_b64) + 1 + strlen(hash_b64) + 1;
    char* storedPassword = (char*)malloc(stored_len);
    if (!storedPassword) {
        perror("malloc failed");
        free(salt_b64);
        free(hash_b64);
        return NULL;
    }
    
    snprintf(storedPassword, stored_len, "%s:%s", salt_b64, hash_b64);
    
    if (userCount < MAX_USERS) {
        strncpy(userDatabase[userCount].username, username, MAX_USERNAME_LEN - 1);
        userDatabase[userCount].username[MAX_USERNAME_LEN - 1] = '\0';
        strncpy(userDatabase[userCount].storedPassword, storedPassword, MAX_STORED_PASS_LEN - 1);
        userDatabase[userCount].storedPassword[MAX_STORED_PASS_LEN - 1] = '\0';
        userCount++;
    }

    free(salt_b64);
    free(hash_b64);
    OPENSSL_cleanse(hashedPassword, KEY_LENGTH);

    return storedPassword;
}

bool verifyPassword(const char* username, const char* providedPassword) {
    const char* storedPassword = NULL;
    for (int i = 0; i < userCount; i++) {
        if (strcmp(userDatabase[i].username, username) == 0) {
            storedPassword = userDatabase[i].storedPassword;
            break;
        }
    }
    if (storedPassword == NULL) return false;

    char* storedPasswordCopy = strdup(storedPassword);
    if(!storedPasswordCopy) return false;

    char* salt_b64 = strtok(storedPasswordCopy, ":");
    char* original_hash_b64 = strtok(NULL, ":");
    if (!salt_b64 || !original_hash_b64) {
        free(storedPasswordCopy);
        return false;
    }

    size_t salt_len;
    unsigned char* salt = base64_decode(salt_b64, &salt_len);
    size_t original_hash_len;
    unsigned char* original_hash = base64_decode(original_hash_b64, &original_hash_len);
    
    free(storedPasswordCopy);

    if (!salt || !original_hash) {
        free(salt);
        free(original_hash);
        return false;
    }

    unsigned char new_hash[KEY_LENGTH];
    if (PKCS5_PBKDF2_HMAC(providedPassword, strlen(providedPassword), salt, salt_len, ITERATIONS, EVP_sha256(), KEY_LENGTH, new_hash) == 0) {
        handle_openssl_error();
        free(salt);
        free(original_hash);
        return false;
    }

    bool result = false;
    if (original_hash_len == KEY_LENGTH) {
        result = (CRYPTO_memcmp(original_hash, new_hash, KEY_LENGTH) == 0);
    }

    free(salt);
    free(original_hash);
    OPENSSL_cleanse(new_hash, KEY_LENGTH);
    return result;
}

void run_test_case(const char* username, const char* password) {
    printf("\n--- Testing for user: %s ---\n", username);

    char* storedPassword = registerUser(username, password);
    if (storedPassword) {
        printf("Registering user '%s'.\n", username);
        printf("Stored format (Base64(salt):Base64(hash)): %s\n", storedPassword);
        free(storedPassword);
    } else {
        printf("Registration failed for user '%s'.\n", username);
        return;
    }
    
    printf("Verifying with correct password...\n");
    bool isCorrect = verifyPassword(username, password);
    printf("Verification successful: %s\n", isCorrect ? "true" : "false");

    printf("Verifying with incorrect password ('wrongpassword')...\n");
    bool isIncorrect = verifyPassword(username, "wrongpassword");
    printf("Verification successful: %s\n", isIncorrect ? "true" : "false");
}

int main() {
    printf("--- User Registration and Verification Simulation ---\n");
    printf("Note: This program must be compiled and linked with OpenSSL.\n");
    printf("Example: gcc your_file.c -o your_program -lssl -lcrypto\n");

    run_test_case("alice", "Password123!");
    run_test_case("bob", "my$ecretP@ss");
    run_test_case("charlie", "aVeryLongAndComplexPassword-12345");
    run_test_case("dave", "short");
    run_test_case("eve", "another_password");

    return 0;
}