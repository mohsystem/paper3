
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* createPhoneNumber(int* numbers, int size) {
    // Input validation
    if (numbers == NULL) {
        fprintf(stderr, "Error: Input array cannot be null\\n");
        return NULL;
    }
    
    if (size != 10) {
        fprintf(stderr, "Error: Array must contain exactly 10 elements\\n");
        return NULL;
    }
    
    // Validate each element is between 0-9
    for (int i = 0; i < size; i++) {
        if (numbers[i] < 0 || numbers[i] > 9) {
            fprintf(stderr, "Error: All elements must be between 0 and 9\\n");
            return NULL;
        }
    }
    
    // Allocate memory for phone number string (14 chars + null terminator)
    char* phoneNumber = (char*)malloc(15 * sizeof(char));
    if (phoneNumber == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        return NULL;
    }
    
    // Build phone number using secure snprintf
    snprintf(phoneNumber, 15, "(%d%d%d) %d%d%d-%d%d%d%d",
             numbers[0], numbers[1], numbers[2],
             numbers[3], numbers[4], numbers[5],
             numbers[6], numbers[7], numbers[8], numbers[9]);
    
    return phoneNumber;
}

int main() {
    // Test case 1: Standard input
    int test1[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
    char* result1 = createPhoneNumber(test1, 10);
    if (result1 != NULL) {
        printf("Test 1: %s\\n", result1);
        free(result1);
    }
    
    // Test case 2: All zeros
    int test2[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    char* result2 = createPhoneNumber(test2, 10);
    if (result2 != NULL) {
        printf("Test 2: %s\\n", result2);
        free(result2);
    }
    
    // Test case 3: All nines
    int test3[] = {9, 9, 9, 9, 9, 9, 9, 9, 9, 9};
    char* result3 = createPhoneNumber(test3, 10);
    if (result3 != NULL) {
        printf("Test 3: %s\\n", result3);
        free(result3);
    }
    
    // Test case 4: Mixed numbers
    int test4[] = {5, 5, 5, 1, 2, 3, 4, 5, 6, 7};
    char* result4 = createPhoneNumber(test4, 10);
    if (result4 != NULL) {
        printf("Test 4: %s\\n", result4);
        free(result4);
    }
    
    // Test case 5: Another mixed pattern
    int test5[] = {8, 0, 0, 5, 5, 5, 1, 2, 3, 4};
    char* result5 = createPhoneNumber(test5, 10);
    if (result5 != NULL) {
        printf("Test 5: %s\\n", result5);
        free(result5);
    }
    
    return 0;
}
