
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#else
#include <fcntl.h>
#include <unistd.h>
#endif

const char ASCII_LETTERS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
const int ASCII_LETTERS_LEN = 52;

int get_secure_random_byte() {
#ifdef _WIN32
    unsigned char byte;
    BCryptGenRandom(NULL, &byte, 1, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return byte;
#else
    int fd = open("/dev/urandom", O_RDONLY);
    unsigned char byte;
    if (fd < 0 || read(fd, &byte, 1) != 1) {
        close(fd);
        srand(time(NULL));
        return rand();
    }
    close(fd);
    return byte;
#endif
}

char* generateRandomString(int length) {
    if (length < 0) {
        fprintf(stderr, "Error: Length cannot be negative\\n");
        return NULL;
    }
    if (length > 1000000) {
        fprintf(stderr, "Error: Length too large to prevent resource exhaustion\\n");
        return NULL;
    }
    
    char* result = (char*)malloc((length + 1) * sizeof(char));
    if (result == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        return NULL;
    }
    
    for (int i = 0; i < length; i++) {
        int randomByte = get_secure_random_byte();
        result[i] = ASCII_LETTERS[randomByte % ASCII_LETTERS_LEN];
    }
    result[length] = '\\0';
    
    return result;
}

int main() {
    char* test1 = generateRandomString(10);
    printf("Test Case 1 (length=10): %s\\n", test1);
    free(test1);
    
    char* test2 = generateRandomString(5);
    printf("Test Case 2 (length=5): %s\\n", test2);
    free(test2);
    
    char* test3 = generateRandomString(20);
    printf("Test Case 3 (length=20): %s\\n", test3);
    free(test3);
    
    char* test4 = generateRandomString(1);
    printf("Test Case 4 (length=1): %s\\n", test4);
    free(test4);
    
    char* test5 = generateRandomString(0);
    printf("Test Case 5 (length=0): '%s'\\n", test5);
    free(test5);
    
    return 0;
}
