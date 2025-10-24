
#include <stdio.h>
#include <stdlib.h>

char findMissingLetter(char array[], int size) {
    // Input validation
    if (array == NULL || size < 2) {
        fprintf(stderr, "Invalid input array\\n");
        exit(1);
    }
    
    // Iterate through the array and find the missing letter
    for (int i = 0; i < size - 1; i++) {
        // Check if the next character is not consecutive
        if (array[i + 1] - array[i] != 1) {
            // Return the missing letter
            return (char)(array[i] + 1);
        }
    }
    
    // This should never happen with valid input
    fprintf(stderr, "No missing letter found\\n");
    exit(1);
}

int main() {
    // Test case 1
    char test1[] = {'a', 'b', 'c', 'd', 'f'};
    printf("Test 1: %c\\n", findMissingLetter(test1, 5)); // Expected: e
    
    // Test case 2
    char test2[] = {'O', 'Q', 'R', 'S'};
    printf("Test 2: %c\\n", findMissingLetter(test2, 4)); // Expected: P
    
    // Test case 3
    char test3[] = {'m', 'n', 'p', 'q'};
    printf("Test 3: %c\\n", findMissingLetter(test3, 4)); // Expected: o
    
    // Test case 4
    char test4[] = {'A', 'B', 'D', 'E'};
    printf("Test 4: %c\\n", findMissingLetter(test4, 4)); // Expected: C
    
    // Test case 5
    char test5[] = {'x', 'z'};
    printf("Test 5: %c\\n", findMissingLetter(test5, 2)); // Expected: y
    
    return 0;
}
