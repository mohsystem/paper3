#include <stdio.h>
#include <stdlib.h>

char* createPhoneNumber(const int numbers[10]) {
    // The resulting string will have a fixed length:
    // (xxx) xxx-xxxx is 14 chars + 1 for null terminator = 15
    char* phoneNumber = (char*)malloc(15 * sizeof(char));
    if (phoneNumber == NULL) {
        return NULL; // Memory allocation failed
    }

    sprintf(phoneNumber, "(%d%d%d) %d%d%d-%d%d%d%d",
            numbers[0], numbers[1], numbers[2],
            numbers[3], numbers[4], numbers[5],
            numbers[6], numbers[7], numbers[8], numbers[9]);
            
    return phoneNumber;
}

int main() {
    // Test cases
    int test_cases[5][10] = {
        {1, 2, 3, 4, 5, 6, 7, 8, 9, 0},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {8, 0, 0, 5, 5, 5, 1, 2, 3, 4},
        {0, 9, 8, 7, 6, 5, 4, 3, 2, 1},
        {5, 1, 9, 5, 5, 5, 4, 4, 6, 8}
    };

    for (int i = 0; i < 5; ++i) {
        char* result = createPhoneNumber(test_cases[i]);
        if (result != NULL) {
            printf("%s\n", result);
            free(result); // Free the allocated memory
        }
    }

    return 0;
}