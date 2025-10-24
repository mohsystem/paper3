
#include <stdio.h>
#include <string.h>

const char* oddOrEven(int* array, int size) {
    if (array == NULL || size == 0) {
        return "even";
    }
    
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += array[i];
    }
    
    return (sum % 2 == 0) ? "even" : "odd";
}

int main() {
    // Test case 1
    int test1[] = {0};
    printf("Test 1: %s\\n", oddOrEven(test1, 1));
    
    // Test case 2
    int test2[] = {0, 1, 4};
    printf("Test 2: %s\\n", oddOrEven(test2, 3));
    
    // Test case 3
    int test3[] = {0, -1, -5};
    printf("Test 3: %s\\n", oddOrEven(test3, 3));
    
    // Test case 4
    printf("Test 4: %s\\n", oddOrEven(NULL, 0));
    
    // Test case 5
    int test5[] = {2, 4, 6, 8};
    printf("Test 5: %s\\n", oddOrEven(test5, 4));
    
    return 0;
}
