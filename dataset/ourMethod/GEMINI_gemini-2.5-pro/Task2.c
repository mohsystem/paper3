#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/**
 * @brief Creates a phone number string from an array of 10 integers.
 *
 * This function validates that the input array contains exactly 10 integers,
 * each between 0 and 9. It formats the numbers into a phone number string
 * like "(123) 456-7890" and writes it to a caller-provided buffer.
 *
 * @param out_buf The character buffer to write the result to.
 * @param out_buf_size The total size of the output buffer.
 * @param numbers A pointer to the array of integers.
 * @param numbers_size The number of elements in the numbers array.
 * @return 0 on success, -1 on failure (e.g., invalid input, small buffer).
 */
int createPhoneNumber(char* out_buf, size_t out_buf_size, const int* numbers, size_t numbers_size) {
    // Rule #1: Initialize pointers to NULL (if applicable, here we check for NULL)
    // and check all function return values.
    
    // Rule #3: Treat all inputs as untrusted. Validate ranges, lengths, and types.
    if (out_buf == NULL || numbers == NULL) {
        return -1; // Fail on NULL pointers
    }

    // Rule #4: The required size is 15: "(123) 456-7890\0"
    const size_t required_size = 15;
    if (out_buf_size < required_size) {
        if (out_buf_size > 0) out_buf[0] = '\0';
        return -1; // Output buffer is too small
    }
    
    // Rule #3: Validate input array size
    if (numbers_size != 10) {
        out_buf[0] = '\0';
        return -1; // Invalid number of digits
    }

    // Rule #3: Validate input range for each element
    // Rule #1: All array accesses must be bounds-checked (implicit by checking numbers_size first)
    for (size_t i = 0; i < numbers_size; ++i) {
        if (numbers[i] < 0 || numbers[i] > 9) {
            out_buf[0] = '\0';
            return -1; // Invalid digit found
        }
    }

    // Rule #1, #2, #4: Use snprintf for safe, bounded string formatting instead of sprintf.
    int written = snprintf(out_buf, out_buf_size, "(%d%d%d) %d%d%d-%d%d%d%d",
                           numbers[0], numbers[1], numbers[2],
                           numbers[3], numbers[4], numbers[5],
                           numbers[6], numbers[7], numbers[8], numbers[9]);

    // Check snprintf's return value for errors or truncation.
    if (written < 0 || (size_t)written >= out_buf_size) {
        out_buf[0] = '\0'; // Ensure buffer is safely terminated on error
        return -1;
    }

    return 0; // Success
}

void print_array(const int* arr, size_t size) {
    printf("{");
    for (size_t i = 0; i < size; ++i) {
        printf("%d%s", arr[i], (i == size - 1) ? "" : ", ");
    }
    printf("}");
}

void run_test_case(const char* test_name, const int* input, size_t size) {
    printf("Test Case: %s\n", test_name);
    printf("Input: ");
    print_array(input, size);
    printf("\n");
    
    char buffer[20]; // A buffer larger than the required 15 chars
    // Initialize buffer to prevent using uninitialized memory
    memset(buffer, 0, sizeof(buffer));

    if (createPhoneNumber(buffer, sizeof(buffer), input, size) == 0) {
        printf("Output: \"%s\"\n", buffer);
    } else {
        printf("Output: Invalid input\n");
    }
    printf("------------------------\n");
}

int main() {
    // Test Case 1: Standard valid input
    int test1[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
    run_test_case("Standard", test1, sizeof(test1)/sizeof(test1[0]));

    // Test Case 2: All zeros
    int test2[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    run_test_case("All Zeros", test2, sizeof(test2)/sizeof(test2[0]));

    // Test Case 3: All nines
    int test3[] = {9, 9, 9, 9, 9, 9, 9, 9, 9, 9};
    run_test_case("All Nines", test3, sizeof(test3)/sizeof(test3[0]));

    // Test Case 4: Invalid size (too short)
    int test4[] = {1, 2, 3};
    run_test_case("Invalid Size", test4, sizeof(test4)/sizeof(test4[0]));

    // Test Case 5: Invalid digit (out of range)
    int test5[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    run_test_case("Invalid Digit", test5, sizeof(test5)/sizeof(test5[0]));

    return 0;
}