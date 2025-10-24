#include <stdio.h>
#include <stddef.h>

char findMissingLetter(const char array[], size_t size) {
    for (size_t i = 0; i < size - 1; i++) {
        if (array[i+1] != array[i] + 1) {
            return (char)(array[i] + 1);
        }
    }
    // This part should not be reachable given the problem constraints
    return '\0';
}

void print_array(const char array[], size_t size) {
    printf("[");
    for(size_t i = 0; i < size; ++i) {
        printf("'%c'", array[i]);
        if (i < size - 1) {
            printf(",");
        }
    }
    printf("]");
}

int main() {
    // Test Case 1
    char test1[] = {'a', 'b', 'c', 'd', 'f'};
    size_t size1 = sizeof(test1) / sizeof(test1[0]);
    printf("Test Case 1: ");
    print_array(test1, size1);
    printf(" -> %c\n", findMissingLetter(test1, size1));

    // Test Case 2
    char test2[] = {'O', 'Q', 'R', 'S'};
    size_t size2 = sizeof(test2) / sizeof(test2[0]);
    printf("Test Case 2: ");
    print_array(test2, size2);
    printf(" -> %c\n", findMissingLetter(test2, size2));

    // Test Case 3
    char test3[] = {'x', 'z'};
    size_t size3 = sizeof(test3) / sizeof(test3[0]);
    printf("Test Case 3: ");
    print_array(test3, size3);
    printf(" -> %c\n", findMissingLetter(test3, size3));

    // Test Case 4
    char test4[] = {'g', 'h', 'i', 'j', 'l'};
    size_t size4 = sizeof(test4) / sizeof(test4[0]);
    printf("Test Case 4: ");
    print_array(test4, size4);
    printf(" -> %c\n", findMissingLetter(test4, size4));

    // Test Case 5
    char test5[] = {'A', 'B', 'C', 'E'};
    size_t size5 = sizeof(test5) / sizeof(test5[0]);
    printf("Test Case 5: ");
    print_array(test5, size5);
    printf(" -> %c\n", findMissingLetter(test5, size5));

    return 0;
}