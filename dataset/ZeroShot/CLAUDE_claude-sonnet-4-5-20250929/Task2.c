
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* createPhoneNumber(int numbers[], int size, char* result) {
    if (numbers == NULL || size != 10 || result == NULL) {
        return NULL;
    }
    
    for (int i = 0; i < size; i++) {
        if (numbers[i] < 0 || numbers[i] > 9) {
            return NULL;
        }
    }
    
    snprintf(result, 15, "(%d%d%d) %d%d%d-%d%d%d%d",
        numbers[0], numbers[1], numbers[2],
        numbers[3], numbers[4], numbers[5],
        numbers[6], numbers[7], numbers[8], numbers[9]);
    
    return result;
}

int main() {
    char result[15];
    
    // Test case 1
    int test1[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
    if (createPhoneNumber(test1, 10, result) != NULL) {
        printf("%s\\n", result);
    }
    
    // Test case 2
    int test2[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    if (createPhoneNumber(test2, 10, result) != NULL) {
        printf("%s\\n", result);
    }
    
    // Test case 3
    int test3[] = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
    if (createPhoneNumber(test3, 10, result) != NULL) {
        printf("%s\\n", result);
    }
    
    // Test case 4
    int test4[] = {5, 5, 5, 1, 2, 3, 4, 5, 6, 7};
    if (createPhoneNumber(test4, 10, result) != NULL) {
        printf("%s\\n", result);
    }
    
    // Test case 5
    int test5[] = {1, 1, 1, 2, 2, 2, 3, 3, 3, 4};
    if (createPhoneNumber(test5, 10, result) != NULL) {
        printf("%s\\n", result);
    }
    
    return 0;
}
