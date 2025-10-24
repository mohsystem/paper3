
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if defined(_WIN32)
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#else
#include <fcntl.h>
#include <unistd.h>
#endif

/* CWE-330, CWE-338: Use cryptographically secure random number generator */
/* Never use rand() for security-sensitive operations */
int getSecureRandomBytes(unsigned char* buffer, size_t length) {
    /* Validate inputs - CWE-476: NULL pointer dereference prevention */
    if (buffer == NULL || length == 0) {
        return -1;
    }

#if defined(_WIN32)
    /* Windows: Use BCryptGenRandom for CSPRNG */
    NTSTATUS status = BCryptGenRandom(
        NULL,
        buffer,
        (ULONG)length,
        BCRYPT_USE_SYSTEM_PREFERRED_RNG
    );
    if (status != 0) {
        return -1;
    }
    return 0;
#else
    /* Linux/Unix: Read from /dev/urandom (cryptographically secure) */
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) {
        return -1;
    }

    size_t totalRead = 0;
    while (totalRead < length) {
        ssize_t bytesRead = read(fd, buffer + totalRead, length - totalRead);
        if (bytesRead <= 0) {
            close(fd);
            return -1;
        }
        totalRead += (size_t)bytesRead;
    }

    close(fd);
    return 0;
#endif
}

/* CWE-120: Buffer overflow prevention with bounds checking */
/* CWE-129: Validate array index to prevent out-of-bounds access */
/* CWE-703: Input validation and error handling */
char* generateRandomString(int length) {
    /* Input validation: length must be non-negative */
    if (length < 0) {
        fprintf(stderr, "Error: Length must be non-negative\\n");
        return NULL;
    }

    /* Handle zero length case */
    if (length == 0) {
        char* empty = (char*)malloc(1);
        if (empty == NULL) {
            return NULL;
        }
        empty[0] = '\\0';
        return empty;
    }

    /* Prevent integer overflow and resource exhaustion - CWE-190, CWE-770 */
    const int MAX_LENGTH = 1000000; /* 1MB safety limit */
    if (length > MAX_LENGTH) {
        fprintf(stderr, "Error: Length exceeds maximum allowed size\\n");
        return NULL;
    }

    /* ASCII letters: A-Z and a-z = 52 total letters */
    const char letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    const size_t numLetters = 52;

    /* Allocate buffer for random bytes - CWE-401: Check allocation */
    unsigned char* randomBytes = (unsigned char*)malloc((size_t)length);
    if (randomBytes == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for random bytes\\n");
        return NULL;
    }

    /* Allocate result string (+1 for null terminator) - CWE-170 */
    char* result = (char*)malloc((size_t)length + 1);
    if (result == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for result\\n");
        /* CWE-401: Free allocated memory before returning */
        memset_s(randomBytes, (size_t)length, 0, (size_t)length);
        free(randomBytes);
        return NULL;
    }

    /* Initialize result buffer - CWE-665 */
    memset(result, 0, (size_t)length + 1);

    /* Generate cryptographically secure random bytes */
    if (getSecureRandomBytes(randomBytes, (size_t)length) != 0) {
        fprintf(stderr, "Error: Failed to generate secure random bytes\\n");
        /* CWE-401: Clean up allocated resources */
        memset_s(randomBytes, (size_t)length, 0, (size_t)length);
        free(randomBytes);
        memset_s(result, (size_t)length + 1, 0, (size_t)length + 1);
        free(result);
        return NULL;
    }

    /* Build result string with bounds checking */
    for (int i = 0; i < length; i++) {
        /* Use modulo to map random byte to valid letter index */
        /* Provides uniform distribution over ASCII letters */
        size_t index = randomBytes[i] % numLetters;
        result[i] = letters[index];
    }

    /* Null-terminate string - CWE-170 */
    result[length] = '\\0';

    /* Clear sensitive random data from memory using memset_s */
    /* memset_s prevents compiler optimization that might remove clearing */
    memset_s(randomBytes, (size_t)length, 0, (size_t)length);
    
    /* CWE-401: Free random bytes buffer */
    free(randomBytes);
    randomBytes = NULL;

    return result;
}

int main(void) {
    char* str = NULL;

    /* Test case 1: Small string */
    printf("Test 1 (length 8): ");
    str = generateRandomString(8);
    if (str != NULL) {
        printf("%s\\n", str);
        memset_s(str, 9, 0, 9);
        free(str);
        str = NULL;
    }

    /* Test case 2: Medium string */
    printf("Test 2 (length 16): ");
    str = generateRandomString(16);
    if (str != NULL) {
        printf("%s\\n", str);
        memset_s(str, 17, 0, 17);
        free(str);
        str = NULL;
    }

    /* Test case 3: Larger string */
    printf("Test 3 (length 32): ");
    str = generateRandomString(32);
    if (str != NULL) {
        printf("%s\\n", str);
        memset_s(str, 33, 0, 33);
        free(str);
        str = NULL;
    }

    /* Test case 4: Empty string */
    printf("Test 4 (length 0): \\"");
    str = generateRandomString(0);
    if (str != NULL) {
        printf("%s\\"\\n", str);
        memset_s(str, 1, 0, 1);
        free(str);
        str = NULL;
    }

    /* Test case 5: Single character */
    printf("Test 5 (length 1): ");
    str = generateRandomString(1);
    if (str != NULL) {
        printf("%s\\n", str);
        memset_s(str, 2, 0, 2);
        free(str);
        str = NULL;
    }

    /* Test case 6: Error handling for negative length */
    printf("Test 6 (negative length): ");
    str = generateRandomString(-5);
    if (str == NULL) {
        printf("Correctly rejected negative length\\n");
    }

    return 0;
}
