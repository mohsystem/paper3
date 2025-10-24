#include <stdio.h>
#include <stddef.h> // Required for size_t

/**
 * @brief Finds the missing letter in an array of consecutive, increasing letters.
 * The array will always be valid and have exactly one letter missing.
 * 
 * @param arr A constant pointer to a character array.
 * @param size The number of elements in the array.
 * @return The missing character.
 */
char findMissingLetter(const char arr[], size_t size) {
    // Since the array length is at least 2, arr[i-1] is always a safe access.
    for (size_t i = 1; i < size; i++) {
        // Check if the current character's ASCII value is not one greater than the previous one.
        if (arr[i] != arr[i-1] + 1) {
            // If not, the missing character is the previous character + 1.
            return (char)(arr[i-1] + 1);
        }
    }
    // This part is unreachable given the problem constraints.
    // It is included to satisfy compiler warnings about control reaching the end of a non-void function.
    return ' ';
}

int main() {
    // Test Case 1
    char test1[] = {'a', 'b', 'c', 'd', 'f'};
    size_t size1 = sizeof(test1) / sizeof(test1[0]);
    printf("Test 1: ['a','b','c','d','f'] -> Expected: e, Got: %c\n", findMissingLetter(test1, size1));

    // Test Case 2
    char test2[] = {'O', 'Q', 'R', 'S'};
    size_t size2 = sizeof(test2) / sizeof(test2[0]);
    printf("Test 2: ['O','Q','R','S'] -> Expected: P, Got: %c\n", findMissingLetter(test2, size2));

    // Test Case 3
    char test3[] = {'x', 'z'};
    size_t size3 = sizeof(test3) / sizeof(test3[0]);
    printf("Test 3: ['x','z'] -> Expected: y, Got: %c\n", findMissingLetter(test3, size3));
    
    // Test Case 4
    char test4[] = {'A', 'B', 'D'};
    size_t size4 = sizeof(test4) / sizeof(test4[0]);
    printf("Test 4: ['A','B','D'] -> Expected: C, Got: %c\n", findMissingLetter(test4, size4));

    // Test Case 5
    char test5[] = {'m', 'n', 'o', 'q'};
    size_t size5 = sizeof(test5) / sizeof(test5[0]);
    printf("Test 5: ['m','n','o','q'] -> Expected: p, Got: %c\n", findMissingLetter(test5, size5));

    return 0;
}