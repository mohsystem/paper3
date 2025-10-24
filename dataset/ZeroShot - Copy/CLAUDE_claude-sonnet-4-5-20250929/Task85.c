
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const char ASCII_LETTERS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

char* generateRandomString(int length) {
    if (length < 0) {
        fprintf(stderr, "Length must be non-negative\\n");
        return NULL;
    }
    
    char* result = (char*)malloc((length + 1) * sizeof(char));
    if (result == NULL) {
        fprintf(stderr, "Memory allocation failed\\n");
        return NULL;
    }
    
    int letters_length = strlen(ASCII_LETTERS);
    
    for (int i = 0; i < length; i++) {
        int random_index = rand() % letters_length;
        result[i] = ASCII_LETTERS[random_index];
    }
    result[length] = '\\0';
    
    return result;
}

int main() {
    srand(time(NULL));
    
    // Test case 1: Generate string of length 10
    char* test1 = generateRandomString(10);
    printf("Test 1 (length 10): %s\\n", test1);
    free(test1);
    
    // Test case 2: Generate string of length 5
    char* test2 = generateRandomString(5);
    printf("Test 2 (length 5): %s\\n", test2);
    free(test2);
    
    // Test case 3: Generate string of length 20
    char* test3 = generateRandomString(20);
    printf("Test 3 (length 20): %s\\n", test3);
    free(test3);
    
    // Test case 4: Generate string of length 0
    char* test4 = generateRandomString(0);
    printf("Test 4 (length 0): '%s'\\n", test4);
    free(test4);
    
    // Test case 5: Generate string of length 15
    char* test5 = generateRandomString(15);
    printf("Test 5 (length 15): %s\\n", test5);
    free(test5);
    
    return 0;
}
