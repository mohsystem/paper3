
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#define ARRAY_SIZE 10

static const int id_sequence[ARRAY_SIZE] = {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000};

int getValueAtIndex(int index, int* result) {
    /* Validate index bounds, return 1 on success, 0 on failure */
    if (index < 0 || index >= ARRAY_SIZE) {
        printf("Error: Index out of bounds. Valid range: 0 to %d\\n", ARRAY_SIZE - 1);
        return 0;
    }
    *result = id_sequence[index];
    return 1;
}

int getValueWithPrompt(int* result) {
    char input[100];
    char* endptr;
    long index;
    
    printf("Enter an index value: ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("Error: Failed to read input\\n");
        return 0;
    }
    
    /* Remove newline */
    input[strcspn(input, "\\n")] = 0;
    
    /* Validate input is not empty */
    if (strlen(input) == 0) {
        printf("Error: Empty input\\n");
        return 0;
    }
    
    /* Parse integer with error checking */
    errno = 0;
    index = strtol(input, &endptr, 10);
    
    if (errno != 0 || *endptr != '\\0' || index > INT_MAX || index < INT_MIN) {
        printf("Error: Invalid input. Please enter a valid integer.\\n");
        return 0;
    }
    
    return getValueAtIndex((int)index, result);
}

int main() {
    int result;
    
    printf("Test Case 1: Valid index 0\\n");
    if (getValueAtIndex(0, &result)) {
        printf("Result: %d\\n\\n", result);
    } else {
        printf("Result: null\\n\\n");
    }
    
    printf("Test Case 2: Valid index 5\\n");
    if (getValueAtIndex(5, &result)) {
        printf("Result: %d\\n\\n", result);
    } else {
        printf("Result: null\\n\\n");
    }
    
    printf("Test Case 3: Valid index 9 (last element)\\n");
    if (getValueAtIndex(9, &result)) {
        printf("Result: %d\\n\\n", result);
    } else {
        printf("Result: null\\n\\n");
    }
    
    printf("Test Case 4: Invalid negative index -1\\n");
    if (getValueAtIndex(-1, &result)) {
        printf("Result: %d\\n\\n", result);
    } else {
        printf("Result: null\\n\\n");
    }
    
    printf("Test Case 5: Invalid index 10 (out of bounds)\\n");
    if (getValueAtIndex(10, &result)) {
        printf("Result: %d\\n\\n", result);
    } else {
        printf("Result: null\\n\\n");
    }
    
    return 0;
}
