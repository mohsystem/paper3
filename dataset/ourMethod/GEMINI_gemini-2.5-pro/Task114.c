#include <stdio.h>
#include <string.h>
#include <stddef.h> // for size_t

// Forward declarations
int safeStringCopy(char* dest, size_t dest_size, const char* src);
int safeStringConcat(char* dest, size_t dest_size, const char* src);
int reverseString(char* str);
void run_copy_tests(void);
void run_concat_tests(void);
void run_reverse_tests(void);

/**
 * @brief Safely copies a string from src to dest.
 * Ensures null-termination and prevents buffer overflows using snprintf.
 * @param dest The destination buffer.
 * @param dest_size The total size of the destination buffer.
 * @param src The source string.
 * @return 0 on success, -1 on error (e.g., null pointers, zero size).
 */
int safeStringCopy(char* dest, size_t dest_size, const char* src) {
    if (dest == NULL || src == NULL) {
        return -1;
    }
    if (dest_size == 0) {
        return -1;
    }

    int written = snprintf(dest, dest_size, "%s", src);
    if (written < 0) {
        // Encoding error or other snprintf failure
        dest[0] = '\0'; // Ensure a valid, empty string state
        return -1;
    }
    // Note: if (size_t)written >= dest_size, truncation occurred, but the operation was safe.
    return 0;
}

/**
 * @brief Safely concatenates src string to the end of dest string.
 * Ensures the destination buffer is not overflowed using strncat.
 * @param dest The destination buffer, which must contain a null-terminated string.
 * @param dest_size The total size of the destination buffer.
 * @param src The source string to append.
 * @return 0 on success, -1 on error (e.g., null pointers).
 */
int safeStringConcat(char* dest, size_t dest_size, const char* src) {
    if (dest == NULL || src == NULL) {
        return -1;
    }
    if (dest_size == 0) {
        return 0; // Nothing can be done
    }

    size_t dest_len = strlen(dest);
    if (dest_len >= dest_size - 1) {
        // No space left to append anything.
        return 0;
    }

    size_t space_left = dest_size - dest_len;
    strncat(dest, src, space_left - 1);

    return 0;
}


/**
 * @brief Reverses a null-terminated string in-place.
 * @param str The string to be reversed.
 * @return 0 on success, -1 on error (null pointer).
 */
int reverseString(char* str) {
    if (str == NULL) {
        return -1;
    }
    size_t len = strlen(str);
    if (len < 2) {
        return 0; // Nothing to reverse for empty or single-char strings.
    }

    char* start = str;
    char* end = str + len - 1;
    while (start < end) {
        char temp = *start;
        *start = *end;
        *end = temp;
        start++;
        end--;
    }
    return 0;
}

void run_copy_tests(void) {
    printf("--- Testing safeStringCopy ---\n");
    char buffer[20];
    const char* sources[] = {
        "short",                   // Fits easily
        "a much longer string",    // Will be truncated
        "",                        // Empty string
        "1234567890123456789",     // Exactly fits with null terminator
        "12345678901234567890"     // Too long by one char
    };
    for (int i = 0; i < 5; ++i) {
        safeStringCopy(buffer, sizeof(buffer), sources[i]);
        printf("Source: \"%s\"\n", sources[i]);
        printf("Dest  : \"%s\" (Buffer size: %zu, strlen: %zu)\n\n", buffer, sizeof(buffer), strlen(buffer));
    }
}

void run_concat_tests(void) {
    printf("--- Testing safeStringConcat ---\n");
    char buffer[20];
    
    // Test 1: Simple concat
    safeStringCopy(buffer, sizeof(buffer), "Hello, ");
    safeStringConcat(buffer, sizeof(buffer), "World!");
    printf("Test 1 Result: \"%s\"\n", buffer);

    // Test 2: Concat to empty string
    safeStringCopy(buffer, sizeof(buffer), "");
    safeStringConcat(buffer, sizeof(buffer), "Append to empty");
    printf("Test 2 Result: \"%s\"\n", buffer);
    
    // Test 3: Concat empty string
    safeStringCopy(buffer, sizeof(buffer), "Start");
    safeStringConcat(buffer, sizeof(buffer), "");
    printf("Test 3 Result: \"%s\"\n", buffer);

    // Test 4: Concat that causes truncation
    safeStringCopy(buffer, sizeof(buffer), "1234567890");
    safeStringConcat(buffer, sizeof(buffer), "abcdefghijklmnop");
    printf("Test 4 Result: \"%s\"\n", buffer);
    
    // Test 5: Concat to a full buffer
    safeStringCopy(buffer, sizeof(buffer), "1234567890123456789");
    safeStringConcat(buffer, sizeof(buffer), "more");
    printf("Test 5 Result: \"%s\"\n\n", buffer);
}

void run_reverse_tests(void) {
    printf("--- Testing reverseString ---\n");
    char test_cases[][50] = {
        "abcdef",
        "racecar",
        "a",
        "",
        "12345"
    };

    for (int i = 0; i < 5; ++i) {
        char original[50];
        safeStringCopy(original, sizeof(original), test_cases[i]);
        reverseString(test_cases[i]);
        printf("Original: \"%s\", Reversed: \"%s\"\n", original, test_cases[i]);
    }
}


int main(void) {
    run_copy_tests();
    run_concat_tests();
    run_reverse_tests();
    return 0;
}