
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#else
#include <fcntl.h>
#include <unistd.h>
#endif

#define ALPHANUMERIC "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
#define ALPHANUMERIC_LEN 62

/**
 * Generates cryptographically secure random bytes
 */
int generate_secure_bytes(unsigned char* buffer, size_t length) {
    if (buffer == NULL || length == 0) {
        return -1;
    }
    
#ifdef _WIN32
    // Windows: Use BCryptGenRandom
    if (BCryptGenRandom(NULL, buffer, (ULONG)length, 
                       BCRYPT_USE_SYSTEM_PREFERRED_RNG) != 0) {
        return -1;
    }
#else
    // Unix/Linux: Use /dev/urandom
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) {
        return -1;
    }
    
    ssize_t result = read(fd, buffer, length);
    close(fd);
    
    if (result != (ssize_t)length) {
        return -1;
    }
#endif
    
    return 0;
}

/**
 * Generates a cryptographically secure random integer within a range
 */
int generate_secure_random_int(int min, int max) {
    if (min > max) {
        fprintf(stderr, "Error: min must be less than or equal to max\\n");
        return -1;
    }
    
    unsigned char random_bytes[4];
    if (generate_secure_bytes(random_bytes, 4) != 0) {
        fprintf(stderr, "Error: Failed to generate secure random bytes\\n");
        return -1;
    }
    
    unsigned int random_value = 0;
    for (int i = 0; i < 4; i++) {
        random_value = (random_value << 8) | random_bytes[i];
    }
    
    int range = max - min + 1;
    return (random_value % range) + min;
}

/**
 * Generates a cryptographically secure hexadecimal token
 */
char* generate_hex_token(size_t byte_length) {
    if (byte_length == 0) {
        return NULL;
    }
    
    unsigned char* random_bytes = (unsigned char*)malloc(byte_length);
    if (random_bytes == NULL) {
        return NULL;
    }
    
    if (generate_secure_bytes(random_bytes, byte_length) != 0) {
        free(random_bytes);
        return NULL;
    }
    
    char* hex_token = (char*)malloc(byte_length * 2 + 1);
    if (hex_token == NULL) {
        free(random_bytes);
        return NULL;
    }
    
    for (size_t i = 0; i < byte_length; i++) {
        sprintf(hex_token + (i * 2), "%02x", random_bytes[i]);
    }
    hex_token[byte_length * 2] = '\\0';
    
    free(random_bytes);
    return hex_token;
}

/**
 * Generates a cryptographically secure alphanumeric token
 */
char* generate_alphanumeric_token(size_t length) {
    if (length == 0) {
        return NULL;
    }
    
    char* token = (char*)malloc(length + 1);
    if (token == NULL) {
        return NULL;
    }
    
    for (size_t i = 0; i < length; i++) {
        unsigned char random_byte;
        if (generate_secure_bytes(&random_byte, 1) != 0) {
            free(token);
            return NULL;
        }
        token[i] = ALPHANUMERIC[random_byte % ALPHANUMERIC_LEN];
    }
    token[length] = '\\0';
    
    return token;
}

/**
 * Generates a cryptographically secure random long
 */
long long generate_secure_random_long() {
    unsigned char random_bytes[8];
    if (generate_secure_bytes(random_bytes, 8) != 0) {
        return -1;
    }
    
    long long result = 0;
    for (int i = 0; i < 8; i++) {
        result = (result << 8) | random_bytes[i];
    }
    
    return result;
}

int main() {
    printf("=== Cryptographically Secure Random Number/Token Generator ===\\n\\n");
    
    // Test Case 1: Random integer in range
    printf("Test Case 1: Random integer between 1 and 100\\n");
    int random_int = generate_secure_random_int(1, 100);
    printf("Result: %d\\n\\n", random_int);
    
    // Test Case 2: Hexadecimal token (32 bytes)
    printf("Test Case 2: Hexadecimal token (32 bytes)\\n");
    char* hex_token32 = generate_hex_token(32);
    if (hex_token32 != NULL) {
        printf("Result: %s\\n", hex_token32);
        free(hex_token32);
    }
    printf("\\n");
    
    // Test Case 3: Alphanumeric token (16 characters)
    printf("Test Case 3: Alphanumeric token (16 chars)\\n");
    char* alpha_token = generate_alphanumeric_token(16);
    if (alpha_token != NULL) {
        printf("Result: %s\\n", alpha_token);
        free(alpha_token);
    }
    printf("\\n");
    
    // Test Case 4: Hexadecimal token (16 bytes)
    printf("Test Case 4: Hexadecimal token (16 bytes)\\n");
    char* hex_token16 = generate_hex_token(16);
    if (hex_token16 != NULL) {
        printf("Result: %s\\n", hex_token16);
        free(hex_token16);
    }
    printf("\\n");
    
    // Test Case 5: Random long value
    printf("Test Case 5: Random long value\\n");
    long long random_long = generate_secure_random_long();
    printf("Result: %lld\\n\\n", random_long);
    
    return 0;
}
