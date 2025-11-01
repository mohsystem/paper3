
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#define SALT_LENGTH 16
#define HASH_ITERATIONS 100000
#define HASH_LENGTH 32
#define MIN_PASSWORD_LENGTH 8
#define MAX_PASSWORD_LENGTH 128
#define MAX_USERNAME_LENGTH 20

typedef struct {
    int success;
    char message[100];
    char hashed_password[100];
    char salt[50];
} RegistrationResult;

static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void base64_encode(const unsigned char* data, size_t len, char* output) {
    size_t i = 0, j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    size_t out_len = 0;

    while (len--) {
        char_array_3[i++] = *(data++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for(i = 0; i < 4; i++)
                output[out_len++] = base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for(j = i; j < 3; j++)
            char_array_3[j] = '\\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

        for (j = 0; j < i + 1; j++)
            output[out_len++] = base64_chars[char_array_4[j]];

        while(i++ < 3)
            output[out_len++] = '=';
    }
    output[out_len] = '\\0';
}

int validate_username(const char* username) {
    if (username == NULL || strlen(username) == 0)
        return 0;
    
    size_t len = strlen(username);
    if (len < 3 || len > MAX_USERNAME_LENGTH)
        return 0;
    
    for (size_t i = 0; i < len; i++) {
        if (!isalnum(username[i]) && username[i] != '_')
            return 0;
    }
    return 1;
}

int hash_password(const char* password, const unsigned char* salt, unsigned char* output) {
    return PKCS5_PBKDF2_HMAC(password, strlen(password),
                             salt, SALT_LENGTH,
                             HASH_ITERATIONS,
                             EVP_sha256(),
                             HASH_LENGTH,
                             output) == 1;
}

RegistrationResult register_user(const char* username, const char* password) {
    RegistrationResult result;
    result.success = 0;
    strcpy(result.hashed_password, "");
    strcpy(result.salt, "");

    // Input validation
    if (username == NULL || strlen(username) == 0) {
        strcpy(result.message, "Username cannot be empty");
        return result;
    }

    if (!validate_username(username)) {
        strcpy(result.message, "Invalid username format");
        return result;
    }

    if (password == NULL || strlen(password) < MIN_PASSWORD_LENGTH) {
        strcpy(result.message, "Password too short");
        return result;
    }

    if (strlen(password) > MAX_PASSWORD_LENGTH) {
        strcpy(result.message, "Password too long");
        return result;
    }

    // Generate secure random salt
    unsigned char salt[SALT_LENGTH];
    if (RAND_bytes(salt, SALT_LENGTH) != 1) {
        strcpy(result.message, "Registration failed");
        return result;
    }

    // Hash password
    unsigned char hash[HASH_LENGTH];
    if (!hash_password(password, salt, hash)) {
        strcpy(result.message, "Registration failed");
        return result;
    }

    base64_encode(hash, HASH_LENGTH, result.hashed_password);
    base64_encode(salt, SALT_LENGTH, result.salt);

    result.success = 1;
    strcpy(result.message, "Registration successful");
    return result;
}

int main() {
    printf("Registration Form - Test Cases\\n\\n");

    // Test case 1: Valid registration
    RegistrationResult result1 = register_user("john_doe", "SecurePass123!");
    printf("Test 1 - Valid user:\\n");
    printf("Success: %s, Message: %s\\n", result1.success ? "true" : "false", result1.message);
    if (result1.success) {
        printf("Hash: %.20s...\\n", result1.hashed_password);
    }

    // Test case 2: Short password
    RegistrationResult result2 = register_user("alice", "pass");
    printf("\\nTest 2 - Short password:\\n");
    printf("Success: %s, Message: %s\\n", result2.success ? "true" : "false", result2.message);

    // Test case 3: Invalid username
    RegistrationResult result3 = register_user("user@name!", "ValidPass123");
    printf("\\nTest 3 - Invalid username:\\n");
    printf("Success: %s, Message: %s\\n", result3.success ? "true" : "false", result3.message);

    // Test case 4: Empty username
    RegistrationResult result4 = register_user("", "ValidPass123");
    printf("\\nTest 4 - Empty username:\\n");
    printf("Success: %s, Message: %s\\n", result4.success ? "true" : "false", result4.message);

    // Test case 5: Valid registration
    RegistrationResult result5 = register_user("user_123", "MyP@ssw0rd!2024");
    printf("\\nTest 5 - Valid user with special chars:\\n");
    printf("Success: %s, Message: %s\\n", result5.success ? "true" : "false", result5.message);
    if (result5.success) {
        printf("Hash: %.20s...\\n", result5.hashed_password);
    }

    return 0;
}
