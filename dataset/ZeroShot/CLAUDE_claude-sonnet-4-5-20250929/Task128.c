
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Generate random integer within a range
int generateRandomInt(int min, int max) {
    if (min >= max) {
        fprintf(stderr, "Error: Max must be greater than min\\n");
        return -1;
    }
    return (rand() % (max - min)) + min;
}

// Generate random long
long long generateRandomLong() {
    long long result = 0;
    for (int i = 0; i < 8; i++) {
        result = (result << 8) | (rand() & 0xFF);
    }
    return result;
}

// Generate random token (hex encoded)
char* generateRandomToken(int byteLength) {
    if (byteLength <= 0) {
        fprintf(stderr, "Error: Byte length must be positive\\n");
        return NULL;
    }
    char* token = (char*)malloc(byteLength * 2 + 1);
    if (token == NULL) {
        return NULL;
    }
    for (int i = 0; i < byteLength; i++) {
        sprintf(token + i * 2, "%02x", rand() % 256);
    }
    token[byteLength * 2] = '\\0';
    return token;
}

// Generate random alphanumeric string
char* generateRandomAlphanumeric(int length) {
    if (length <= 0) {
        fprintf(stderr, "Error: Length must be positive\\n");
        return NULL;
    }
    const char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    int chars_len = strlen(chars);
    char* result = (char*)malloc(length + 1);
    if (result == NULL) {
        return NULL;
    }
    for (int i = 0; i < length; i++) {
        result[i] = chars[rand() % chars_len];
    }
    result[length] = '\\0';
    return result;
}

int main() {
    srand(time(NULL));
    
    printf("Test Case 1 - Random Integer (1-100):\\n");
    printf("%d\\n", generateRandomInt(1, 100));
    
    printf("\\nTest Case 2 - Random Long:\\n");
    printf("%lld\\n", generateRandomLong());
    
    printf("\\nTest Case 3 - Random Token (32 bytes):\\n");
    char* token = generateRandomToken(32);
    if (token != NULL) {
        printf("%s\\n", token);
        free(token);
    }
    
    printf("\\nTest Case 4 - Random Alphanumeric (16 chars):\\n");
    char* alphanumeric = generateRandomAlphanumeric(16);
    if (alphanumeric != NULL) {
        printf("%s\\n", alphanumeric);
        free(alphanumeric);
    }
    
    printf("\\nTest Case 5 - Multiple Random Integers:\\n");
    for (int i = 0; i < 5; i++) {
        printf("%d\\n", generateRandomInt(1, 1000));
    }
    
    return 0;
}
