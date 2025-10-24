#include <stdio.h>
#include <string.h>

/**
 * @brief Formats an array of 10 integers (0-9) into a phone number string.
 *
 * This function writes the formatted string into a user-provided buffer to avoid
 * memory allocation issues within the function.
 *
 * @param numbers A pointer to an array of 10 integers.
 * @param buffer A pointer to the character buffer where the result will be stored.
 * @param buffer_size The size of the provided buffer.
 * @return A pointer to the buffer on success, or NULL on failure.
 */
char* createPhoneNumber(const int* numbers, char* buffer, size_t buffer_size) {
    // Security: Validate input pointers.
    if (numbers == NULL || buffer == NULL) {
        return NULL;
    }

    // Security: Check if buffer is large enough for "(xxx) xxx-xxxx" + null terminator.
    // The required size is 14 + 1 = 15.
    if (buffer_size < 15) {
        return NULL;
    }

    // Security: Validate that all numbers are single digits before formatting.
    for (int i = 0; i < 10; ++i) {
        if (numbers[i] < 0 || numbers[i] > 9) {
            return NULL; // Invalid digit found
        }
    }

    // Security: Use snprintf to prevent buffer overflows.
    int result = snprintf(buffer, buffer_size, "(%d%d%d) %d%d%d-%d%d%d%d",
                          numbers[0], numbers[1], numbers[2],
                          numbers[3], numbers[4], numbers[5],
                          numbers[6], numbers[7], numbers[8], numbers[9]);

    // Check if snprintf succeeded and did not truncate.
    if (result < 0 || (size_t)result >= buffer_size) {
        return NULL;
    }

    return buffer;
}


int main() {
    char phone_buffer[15];

    // Test Case 1: Valid input
    int case1[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
    if (createPhoneNumber(case1, phone_buffer, sizeof(phone_buffer))) {
        printf("Test Case 1: %s\n", phone_buffer);
    } else {
        fprintf(stderr, "Test Case 1: Failed\n");
    }

    // Test Case 2: Valid input with zeros
    int case2[] = {0, 0, 0, 1, 1, 1, 2, 2, 2, 2};
     if (createPhoneNumber(case2, phone_buffer, sizeof(phone_buffer))) {
        printf("Test Case 2: %s\n", phone_buffer);
    } else {
        fprintf(stderr, "Test Case 2: Failed\n");
    }

    // Test Case 3: Invalid number (out of range)
    int case3[] = {1, 2, 3, 4, 5, 16, 7, 8, 9, 0};
    if (createPhoneNumber(case3, phone_buffer, sizeof(phone_buffer))) {
        printf("Test Case 3: %s\n", phone_buffer);
    } else {
        fprintf(stderr, "Test Case 3: Failed due to invalid input number\n");
    }

    // Test Case 4: Invalid input (NULL numbers array)
    if (createPhoneNumber(NULL, phone_buffer, sizeof(phone_buffer))) {
        printf("Test Case 4: %s\n", phone_buffer);
    } else {
        fprintf(stderr, "Test Case 4: Failed due to NULL input array\n");
    }

    // Test Case 5: Invalid input (insufficient buffer size)
    char small_buffer[10];
    if (createPhoneNumber(case1, small_buffer, sizeof(small_buffer))) {
        printf("Test Case 5: %s\n", small_buffer);
    } else {
        fprintf(stderr, "Test Case 5: Failed due to insufficient buffer size\n");
    }

    return 0;
}