
#include <stdio.h>
#include <string.h>

void createPhoneNumber(int numbers[], char* result) {
    sprintf(result, "(%d%d%d) %d%d%d-%d%d%d%d",
        numbers[0], numbers[1], numbers[2],
        numbers[3], numbers[4], numbers[5],
        numbers[6], numbers[7], numbers[8], numbers[9]);
}

int main() {
    char result[15];
    
    // Test case 1
    int test1[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
    createPhoneNumber(test1, result);
    printf("%s\\n", result);
    
    // Test case 2
    int test2[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    createPhoneNumber(test2, result);
    printf("%s\\n", result);
    
    // Test case 3
    int test3[] = {9, 9, 9, 9, 9, 9, 9, 9, 9, 9};
    createPhoneNumber(test3, result);
    printf("%s\\n", result);
    
    // Test case 4
    int test4[] = {5, 5, 5, 1, 2, 3, 4, 5, 6, 7};
    createPhoneNumber(test4, result);
    printf("%s\\n", result);
    
    // Test case 5
    int test5[] = {8, 6, 7, 5, 3, 0, 9, 1, 2, 3};
    createPhoneNumber(test5, result);
    printf("%s\\n", result);
    
    return 0;
}
