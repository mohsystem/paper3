
#include <stdio.h>

char findMissingLetter(char array[], int size) {
    for (int i = 0; i < size - 1; i++) {
        if (array[i + 1] - array[i] != 1) {
            return (char)(array[i] + 1);
        }
    }
    return ' ';
}

int main() {
    // Test case 1
    char test1[] = {'a', 'b', 'c', 'd', 'f'};
    printf("Test 1: %c\\n", findMissingLetter(test1, 5)); // Expected: e
    
    // Test case 2
    char test2[] = {'O', 'Q', 'R', 'S'};
    printf("Test 2: %c\\n", findMissingLetter(test2, 4)); // Expected: P
    
    // Test case 3
    char test3[] = {'a', 'b', 'd', 'e'};
    printf("Test 3: %c\\n", findMissingLetter(test3, 4)); // Expected: c
    
    // Test case 4
    char test4[] = {'m', 'n', 'o', 'q', 'r'};
    printf("Test 4: %c\\n", findMissingLetter(test4, 5)); // Expected: p
    
    // Test case 5
    char test5[] = {'X', 'Z'};
    printf("Test 5: %c\\n", findMissingLetter(test5, 2)); // Expected: Y
    
    return 0;
}
