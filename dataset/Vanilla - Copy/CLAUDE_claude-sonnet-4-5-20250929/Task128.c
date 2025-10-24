
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

// Generate random integer between min and max (inclusive)
int generateRandomInt(int min, int max) {
    return rand() % (max - min + 1) + min;
}

// Generate random double between 0.0 and 1.0
double generateRandomDouble() {
    return (double)rand() / (double)RAND_MAX;
}

// Generate random alphanumeric token of specified length
void generateRandomToken(int length, char* token) {
    const char characters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    int charactersLen = strlen(characters);
    
    for (int i = 0; i < length; i++) {
        int index = rand() % charactersLen;
        token[i] = characters[index];
    }
    token[length] = '\\0';
}

// Generate random boolean
bool generateRandomBoolean() {
    return rand() % 2 == 1;
}

// Generate array of random integers
void generateRandomIntArray(int size, int min, int max, int* array) {
    for (int i = 0; i < size; i++) {
        array[i] = rand() % (max - min + 1) + min;
    }
}

int main() {
    srand(time(NULL));
    
    printf("Test Case 1: Random integer between 1 and 100\\n");
    printf("%d\\n", generateRandomInt(1, 100));
    
    printf("\\nTest Case 2: Random double\\n");
    printf("%f\\n", generateRandomDouble());
    
    printf("\\nTest Case 3: Random token of length 16\\n");
    char token[17];
    generateRandomToken(16, token);
    printf("%s\\n", token);
    
    printf("\\nTest Case 4: Random boolean\\n");
    printf("%s\\n", generateRandomBoolean() ? "true" : "false");
    
    printf("\\nTest Case 5: Array of 5 random integers between 10 and 50\\n");
    int randomArray[5];
    generateRandomIntArray(5, 10, 50, randomArray);
    for (int i = 0; i < 5; i++) {
        printf("%d ", randomArray[i]);
    }
    printf("\\n");
    
    return 0;
}
