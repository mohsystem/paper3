
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
    printf("Test 1: %s\\n", oddOrEven(test1, 1)); // Expected: "even"
    
    // Test case 2
    int test2[] = {0, 1, 4};
    printf("Test 2: %s\\n", oddOrEven(test2, 3)); // Expected: "odd"
    
    // Test case 3
    int test3[] = {0, -1, -5};
    printf("Test 3: %s\\n", oddOrEven(test3, 3)); // Expected: "even"
    
    // Test case 4
    int* test4 = NULL;
    printf("Test 4: %s\\n", oddOrEven(test4, 0)); // Expected: "even"
    
    // Test case 5
    int test5[] = {1, 2, 3, 4, 5};
    printf("Test 5: %s\\n", oddOrEven(test5, 5)); // Expected: "odd"
    
    return 0;
}
