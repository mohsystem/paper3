
#include <stdio.h>
#include <stdlib.h>

char findMissingLetter(char* array, int length) {
    if (array == NULL || length < 2) {
        fprintf(stderr, "Invalid input array\\n");
        exit(1);
    }
    
    for (int i = 0; i < length - 1; i++) {
        if (array[i + 1] - array[i] != 1) {
            return array[i] + 1;
        }
    }
    
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
    char test3[] = {'a', 'b', 'd'};
    printf("Test 3: %c\\n", findMissingLetter(test3, 3)); // Expected: c
    
    // Test case 4
    char test4[] = {'m', 'n', 'o', 'q', 'r'};
    printf("Test 4: %c\\n", findMissingLetter(test4, 5)); // Expected: p
    
    // Test case 5
    char test5[] = {'X', 'Z'};
    printf("Test 5: %c\\n", findMissingLetter(test5, 2)); // Expected: Y
    
    return 0;
}
