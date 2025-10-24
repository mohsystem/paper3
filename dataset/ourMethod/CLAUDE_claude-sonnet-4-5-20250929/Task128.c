
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <openssl/rand.h>
#include <openssl/err.h>

/* CWE-338 mitigation: Use cryptographically secure RNG from OpenSSL */
/* CWE-327 mitigation: Never use rand() or insecure PRNGs for security-sensitive operations */

/* Generate a secure random integer in range [min, max]
 * CWE-190 mitigation: Check for overflow in range calculation
 * CWE-330 mitigation: Use CSPRNG for all random generation
 */
int generate_secure_random_int(int64_t min, int64_t max, int64_t* result) {
    /* CWE-20 mitigation: Validate input parameters */
    if (result == NULL) {
        return -1;
    }
    
    if (min > max) {
        return -1;
    }
    
    /* CWE-190 mitigation: Check for potential overflow */
    if (max > 0 && min < 0 && (max - min) < 0) {
        return -1;
    }
    
    uint64_t range = (uint64_t)(max - min);
    uint64_t random_value;
    
    /* CWE-327 mitigation: Use rejection sampling to avoid modulo bias */
    const uint64_t rejection_threshold = (UINT64_MAX - range) % (range + 1);
    
    do {
        /* CWE-330 mitigation: Use OpenSSL CSPRNG */
        if (RAND_bytes((unsigned char*)&random_value, sizeof(random_value)) != 1) {
            return -1;
        }
    } while (random_value < rejection_threshold);
    
    *result = min + (int64_t)(random_value % (range + 1));
    return 0;
}

/* Generate a secure random token as hex string
 * CWE-330 mitigation: Use CSPRNG for token generation
 * CWE-120 mitigation: Bounds-check all buffer operations
 */
int generate_secure_token(size_t byte_length, char* output, size_t output_size) {
    /* CWE-20 mitigation: Validate all input parameters */
    if (output == NULL || byte_length == 0 || byte_length > 1024) {
        return -1;
    }
    
    /* CWE-120 mitigation: Ensure output buffer is large enough for hex + null terminator */
    if (output_size < (byte_length * 2 + 1)) {
        return -1;
    }
    
    /* CWE-401 mitigation: Initialize pointer to NULL for safe cleanup */
    unsigned char* buffer = NULL;
    
    /* CWE-401 mitigation: Allocate memory and check for NULL */
    buffer = (unsigned char*)malloc(byte_length);
    if (buffer == NULL) {
        return -1;
    }
    
    /* CWE-327 mitigation: Generate cryptographically secure random bytes */
    if (RAND_bytes(buffer, (int)byte_length) != 1) {
        /* CWE-401 mitigation: Free memory on error path */
        free(buffer);
        return -1;
    }
    
    /* CWE-120 mitigation: Use snprintf with size limit to prevent buffer overflow */
    size_t pos = 0;
    for (size_t i = 0; i < byte_length; i++) {
        /* CWE-120 mitigation: Check bounds before each write */
        if (pos + 2 >= output_size) {
            free(buffer);
            return -1;
        }
        
        int written = snprintf(output + pos, output_size - pos, "%02x", buffer[i]);
        if (written < 0 || written >= (int)(output_size - pos)) {
            free(buffer);
            return -1;
        }
        pos += written;
    }
    
    /* CWE-134 mitigation: Ensure null termination */
    output[pos] = '\\0';
    
    /* CWE-401 mitigation: Free allocated memory */
    free(buffer);
    return 0;
}

/* Generate secure random bytes
 * CWE-330 mitigation: Use CSPRNG for all random byte generation
 */
int generate_secure_bytes(size_t length, unsigned char* output) {
    /* CWE-20 mitigation: Validate input parameters */
    if (output == NULL || length == 0 || length > 1024) {
        return -1;
    }
    
    /* CWE-327 mitigation: Use OpenSSL CSPRNG */
    if (RAND_bytes(output, (int)length) != 1) {
        return -1;
    }
    
    return 0;
}

int main(void) {
    int ret;
    int64_t random_num;
    char token[256]; /* Buffer for hex tokens */
    unsigned char bytes[32]; /* Buffer for random bytes */
    
    printf("=== Secure Random Number and Token Generator ===\\n\\n");
    
    /* Test Case 1: Generate random integers in range [1, 100] */
    printf("Test 1: Generate 5 random integers in range [1, 100]\\n");
    for (int i = 0; i < 5; i++) {
        ret = generate_secure_random_int(1, 100, &random_num);
        if (ret != 0) {
            fprintf(stderr, "Error generating random integer\\n");
            return 1;
        }
        printf("  Random number %d: %lld\\n", i + 1, (long long)random_num);
    }
    printf("\\n");
    
    /* Test Case 2: Generate random integers in range [0, 255] */
    printf("Test 2: Generate 5 random integers in range [0, 255]\\n");
    for (int i = 0; i < 5; i++) {
        ret = generate_secure_random_int(0, 255, &random_num);
        if (ret != 0) {
            fprintf(stderr, "Error generating random integer\\n");
            return 1;
        }
        printf("  Random number %d: %lld\\n", i + 1, (long long)random_num);
    }
    printf("\\n");
    
    /* Test Case 3: Generate 16-byte (128-bit) secure tokens */
    printf("Test 3: Generate 5 secure tokens (16 bytes each)\\n");
    for (int i = 0; i < 5; i++) {
        /* CWE-120 mitigation: Clear buffer before use */
        memset(token, 0, sizeof(token));
        ret = generate_secure_token(16, token, sizeof(token));
        if (ret != 0) {
            fprintf(stderr, "Error generating token\\n");
            return 1;
        }
        printf("  Token %d: %s\\n", i + 1, token);
    }
    printf("\\n");
    
    /* Test Case 4: Generate 32-byte (256-bit) secure tokens */
    printf("Test 4: Generate 5 secure tokens (32 bytes each)\\n");
    for (int i = 0; i < 5; i++) {
        /* CWE-120 mitigation: Clear buffer before use */
        memset(token, 0, sizeof(token));
        ret = generate_secure_token(32, token, sizeof(token));
        if (ret != 0) {
            fprintf(stderr, "Error generating token\\n");
            return 1;
        }
        printf("  Token %d: %s\\n", i + 1, token);
    }
    printf("\\n");
    
    /* Test Case 5: Generate secure random bytes */
    printf("Test 5: Generate 5 sets of secure random bytes (8 bytes each)\\n");
    for (int i = 0; i < 5; i++) {
        /* CWE-120 mitigation: Clear buffer before use */
        memset(bytes, 0, sizeof(bytes));
        ret = generate_secure_bytes(8, bytes);
        if (ret != 0) {
            fprintf(stderr, "Error generating random bytes\\n");
            return 1;
        }
        printf("  Bytes %d: ", i + 1);
        for (size_t j = 0; j < 8; j++) {
            printf("%02x", bytes[j]);
        }
        printf("\\n");
    }
    printf("\\n");
    
    return 0;
}
