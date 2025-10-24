
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const char* ASCII_LETTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

char* generateRandomString(int length) {
    if (length < 0) {
        return NULL;
    }
    
    char* result = (char*)malloc((length + 1) * sizeof(char));
    if (result == NULL) {
        return NULL;
    }
    
    int letters_len = strlen(ASCII_LETTERS);
    
    for (int i = 0; i < length; i++) {
        int randomIndex = rand() % letters_len;
        result[i] = ASCII_LETTERS[randomIndex];
    }
    result[length] = '\\0';
    
    return result;
}

int main() {
    srand(time(NULL));
    
    char* test1 = generateRandomString(10);
    printf("Test case 1 (length 10): %s\\n", test1);
    free(test1);
    
    char* test2 = generateRandomString(5);
    printf("Test case 2 (length 5): %s\\n", test2);
    free(test2);
    
    char* test3 = generateRandomString(20);
    printf("Test case 3 (length 20): %s\\n", test3);
    free(test3);
    
    char* test4 = generateRandomString(0);
    printf("Test case 4 (length 0): %s\\n", test4);
    free(test4);
    
    char* test5 = generateRandomString(15);
    printf("Test case 5 (length 15): %s\\n", test5);
    free(test5);
    
    return 0;
}
