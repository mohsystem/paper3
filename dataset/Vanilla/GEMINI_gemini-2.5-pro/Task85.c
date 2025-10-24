#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
 * Generates a random string of a given length.
 * The string consists only of ASCII letters (uppercase and lowercase).
 * The caller is responsible for freeing the returned string.
 * @param length The length of the string to generate.
 * @return A pointer to the dynamically allocated random string, or NULL on error.
 */
char* generateRandomString(int length) {
    if (length < 0) {
        // Returning NULL is a common way to signal an error in C
        return NULL;
    }

    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    const int charset_size = sizeof(charset) - 1;

    // Allocate memory for the string plus the null terminator
    char* random_string = (char*)malloc((length + 1) * sizeof(char));
    if (random_string == NULL) {
        // Memory allocation failed
        perror("Unable to allocate memory for string");
        return NULL;
    }

    for (int i = 0; i < length; i++) {
        int key = rand() % charset_size;
        random_string[i] = charset[key];
    }

    random_string[length] = '\0'; // Null-terminate the string
    return random_string;
}

int main() {
    // Seed the random number generator once at the beginning of the program
    srand((unsigned int)time(NULL));

    printf("C Test Cases:\n");

    int testLengths[] = {10, 15, 8, 0, 25};
    int numTests = sizeof(testLengths) / sizeof(testLengths[0]);

    for (int i = 0; i < numTests; i++) {
        int len = testLengths[i];
        char* randomString = generateRandomString(len);

        if (randomString != NULL) {
            printf("Length %d: %s\n", len, randomString);
            free(randomString); // Free the allocated memory after use
        } else {
            fprintf(stderr, "Error generating string for length %d\n", len);
        }
    }

    return 0;
}