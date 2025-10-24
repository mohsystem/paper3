
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

char* encrypt(const char* input) {
    if (input == NULL || strlen(input) == 0) {
        if (input == NULL) return NULL;
        char* result = (char*)malloc(1);
        result[0] = '\\0';
        return result;
    }
    
    int len = strlen(input);
    char* encrypted = (char*)malloc(len + 1);
    int shift = 3; // Caesar cipher shift
    
    for (int i = 0; i < len; i++) {
        char ch = input[i];
        
        if (isupper(ch)) {
            encrypted[i] = ((ch - 'A' + shift) % 26) + 'A';
        } else if (islower(ch)) {
            encrypted[i] = ((ch - 'a' + shift) % 26) + 'a';
        } else if (isdigit(ch)) {
            encrypted[i] = ((ch - '0' + shift) % 10) + '0';
        } else {
            encrypted[i] = ch;
        }
    }
    
    encrypted[len] = '\\0';
    return encrypted;
}

int main() {
    // Test case 1: Simple lowercase string
    const char* test1 = "hello";
    char* result1 = encrypt(test1);
    printf("Test 1 - Input: %s\\n", test1);
    printf("Encrypted: %s\\n\\n", result1);
    free(result1);
    
    // Test case 2: Mixed case string
    const char* test2 = "HelloWorld";
    char* result2 = encrypt(test2);
    printf("Test 2 - Input: %s\\n", test2);
    printf("Encrypted: %s\\n\\n", result2);
    free(result2);
    
    // Test case 3: String with numbers
    const char* test3 = "Test123";
    char* result3 = encrypt(test3);
    printf("Test 3 - Input: %s\\n", test3);
    printf("Encrypted: %s\\n\\n", result3);
    free(result3);
    
    // Test case 4: String with special characters
    const char* test4 = "Hello, World! 2024";
    char* result4 = encrypt(test4);
    printf("Test 4 - Input: %s\\n", test4);
    printf("Encrypted: %s\\n\\n", result4);
    free(result4);
    
    // Test case 5: Edge case with letters near end of alphabet
    const char* test5 = "xyz XYZ 789";
    char* result5 = encrypt(test5);
    printf("Test 5 - Input: %s\\n", test5);
    printf("Encrypted: %s\\n\\n", result5);
    free(result5);
    
    return 0;
}
