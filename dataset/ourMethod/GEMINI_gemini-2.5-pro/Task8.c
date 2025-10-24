#include <stdio.h>
#include <stddef.h> // For size_t

/**
 * @brief Finds the missing letter in an array of consecutive letters.
 *
 * @param arr A pointer to an array of characters. The array must contain
 *            a sequence of consecutive, increasing letters with exactly
 *            one letter missing.
 * @param size The number of elements in the array. Must be at least 2.
 * @return The missing character.
 */
char findMissingLetter(const char arr[], size_t size) {
    // Although the prompt guarantees valid input, it's good practice
    // to check for NULL pointers and minimum size.
    if (arr == NULL || size < 2) {
        return '\0'; // Return a sentinel value for invalid input
    }

    // Loop starts from the second element to compare with the previous one.
    // The loop condition `i < size` ensures we don't access out of bounds.
    for (size_t i = 1; i < size; ++i) {
        // If the ASCII value of the current char is not one greater than the previous,
        // we've found the gap. The missing letter is the character after the previous one.
        if ((unsigned char)arr[i] != (unsigned char)arr[i-1] + 1) {
            return (char)(arr[i-1] + 1);
        }
    }
    // This part of the code should be unreachable given the problem constraints
    // (always exactly one letter missing). Return a null character as a fallback.
    return '\0';
}

// Helper function to print test case results
void run_test_case(const char* name, const char arr[], size_t size, char expected) {
    char result = findMissingLetter(arr, size);
    printf("%s: Result = '%c', Expected = '%c'\n", name, result, expected);
}

int main() {
    // Test Case 1
    char test1[] = {'a', 'b', 'c', 'd', 'f'};
    size_t size1 = sizeof(test1) / sizeof(test1[0]);
    run_test_case("Test Case 1", test1, size1, 'e');

    // Test Case 2
    char test2[] = {'O', 'Q', 'R', 'S'};
    size_t size2 = sizeof(test2) / sizeof(test2[0]);
    run_test_case("Test Case 2", test2, size2, 'P');

    // Test Case 3
    char test3[] = {'x', 'z'};
    size_t size3 = sizeof(test3) / sizeof(test3[0]);
    run_test_case("Test Case 3", test3, size3, 'y');

    // Test Case 4
    char test4[] = {'F', 'G', 'H', 'J'};
    size_t size4 = sizeof(test4) / sizeof(test4[0]);
    run_test_case("Test Case 4", test4, size4, 'I');

    // Test Case 5
    char test5[] = {'m', 'n', 'o', 'q', 'r'};
    size_t size5 = sizeof(test5) / sizeof(test5[0]);
    run_test_case("Test Case 5", test5, size5, 'p');

    return 0;
}