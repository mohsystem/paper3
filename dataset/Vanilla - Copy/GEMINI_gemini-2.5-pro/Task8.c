#include <stdio.h>

char findMissingLetter(const char arr[], int size) {
    for (int i = 0; i < size - 1; i++) {
        // If the difference between ASCII values is not 1, a letter is missing.
        if (arr[i+1] - arr[i] > 1) {
            // The missing letter is the current letter's ASCII value + 1
            return (char)(arr[i] + 1);
        }
    }
    // This part should not be reached given the problem constraints.
    return ' ';
}

int main() {
    // Test Case 1
    char test1[] = {'a', 'b', 'c', 'd', 'f'};
    int size1 = sizeof(test1) / sizeof(test1[0]);
    printf("['a','b','c','d','f'] -> %c\n", findMissingLetter(test1, size1));

    // Test Case 2
    char test2[] = {'O', 'Q', 'R', 'S'};
    int size2 = sizeof(test2) / sizeof(test2[0]);
    printf("['O','Q','R','S'] -> %c\n", findMissingLetter(test2, size2));

    // Test Case 3
    char test3[] = {'x', 'z'};
    int size3 = sizeof(test3) / sizeof(test3[0]);
    printf("['x','z'] -> %c\n", findMissingLetter(test3, size3));

    // Test Case 4
    char test4[] = {'F', 'G', 'H', 'J'};
    int size4 = sizeof(test4) / sizeof(test4[0]);
    printf("['F','G','H','J'] -> %c\n", findMissingLetter(test4, size4));

    // Test Case 5
    char test5[] = {'m', 'n', 'o', 'q', 'r'};
    int size5 = sizeof(test5) / sizeof(test5[0]);
    printf("['m','n','o','q','r'] -> %c\n", findMissingLetter(test5, size5));

    return 0;
}