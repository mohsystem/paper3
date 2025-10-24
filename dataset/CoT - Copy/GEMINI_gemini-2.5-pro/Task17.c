#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

// Note: The caller is responsible for freeing the memory of the returned string.
char* toJadenCase(const char* phrase) {
    if (phrase == NULL) {
        return NULL;
    }

    size_t len = strlen(phrase);
    // Allocate memory for the new string (+1 for the null terminator)
    char* result = (char*)malloc(len + 1);
    if (result == NULL) {
        // Handle memory allocation failure
        return NULL;
    }

    strcpy(result, phrase);

    bool capitalize = true;
    for (size_t i = 0; i < len; ++i) {
        // isalpha/isspace/toupper expect an int; casting to unsigned char prevents
        // issues with negative char values (undefined behavior).
        if (capitalize && isalpha((unsigned char)result[i])) {
            result[i] = toupper((unsigned char)result[i]);
            capitalize = false;
        } else if (isspace((unsigned char)result[i])) {
            capitalize = true;
        }
    }

    return result;
}

int main() {
    const char* testCases[] = {
        "How can mirrors be real if our eyes aren't real",
        "",
        "most trees are blue",
        "the quick brown fox jumps over the lazy dog",
        "hello world"
    };
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    printf("C Test Cases:\n");
    for (int i = 0; i < numTestCases; ++i) {
        const char* original = testCases[i];
        char* jadenCased = toJadenCase(original);

        printf("Test Case %d:\n", i + 1);
        printf("  Original: \"%s\"\n", original);
        if (jadenCased != NULL) {
            printf("  Jaden-Cased: \"%s\"\n\n", jadenCased);
            free(jadenCased); // Free the dynamically allocated memory
        } else {
            printf("  Jaden-Cased: (null)\n\n");
        }
    }

    return 0;
}