#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Accepts an array of 10 integers and returns a dynamically allocated
 * string in the form of a phone number.
 *
 * @param numbers An array of 10 integers.
 * @return A new string that must be freed by the caller. Returns NULL on failure.
 */
char* createPhoneNumber(const int numbers[10]) {
    // The required buffer size is 15: 14 characters for "(XXX) XXX-XXXX" and 1 for the null terminator.
    // Allocate memory on the heap.
    char* phoneNumber = (char*)malloc(15 * sizeof(char));
    
    // Always check if malloc succeeded.
    if (phoneNumber == NULL) {
        perror("Failed to allocate memory");
        return NULL;
    }

    // Use snprintf for safe, formatted string creation to prevent buffer overflows.
    snprintf(phoneNumber, 15, "(%d%d%d) %d%d%d-%d%d%d%d",
             numbers[0], numbers[1], numbers[2],
             numbers[3], numbers[4], numbers[5],
             numbers[6], numbers[7], numbers[8], numbers[9]);

    return phoneNumber;
}

int main() {
    // 5 Test Cases
    int testCases[5][10] = {
        {1, 2, 3, 4, 5, 6, 7, 8, 9, 0},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {0, 9, 8, 7, 6, 5, 4, 3, 2, 1},
        {5, 5, 5, 8, 6, 7, 5, 3, 0, 9},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };

    for (int i = 0; i < 5; i++) {
        char* phoneNumber = createPhoneNumber(testCases[i]);
        if (phoneNumber != NULL) {
            printf("Test Case %d: %s\n", i + 1, phoneNumber);
            
            // IMPORTANT: The caller is responsible for freeing the memory
            // allocated by createPhoneNumber to prevent memory leaks.
            free(phoneNumber);
        } else {
            printf("Test Case %d: Failed to create phone number.\n", i + 1);
        }
    }

    return 0;
}