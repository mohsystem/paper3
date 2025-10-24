#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

// The function returns a new string that needs to be freed by the caller.
char* toJadenCase(const char *phrase) {
    if (phrase == NULL) {
        return NULL;
    }
    
    size_t len = strlen(phrase);
    char *result = (char *)malloc(len + 1);
    if (result == NULL) {
        // Memory allocation failed
        return NULL;
    }
    strcpy(result, phrase);

    bool capitalizeNext = true;
    for (size_t i = 0; i < len; i++) {
        if (isspace((unsigned char)result[i])) {
            capitalizeNext = true;
        } else if (capitalizeNext) {
            result[i] = toupper((unsigned char)result[i]);
            capitalizeNext = false;
        }
    }

    return result;
}

int main() {
    const char* testCases[] = {
        "How can mirrors be real if our eyes aren't real",
        "most trees are blue",
        "",
        "School is the tool to brainwash the youth.",
        NULL
    };

    for (int i = 0; i < 5; ++i) {
        printf("Test Case %d:\n", i + 1);
        
        if (testCases[i] != NULL) {
            printf("Original:    \"%s\"\n", testCases[i]);
        } else {
            printf("Original:    (null)\n");
        }
        
        char* jadenCased = toJadenCase(testCases[i]);
        if (jadenCased != NULL) {
            printf("Jaden-Cased: \"%s\"\n", jadenCased);
            free(jadenCased); // Free the dynamically allocated memory
        } else {
            printf("Jaden-Cased: (null)\n");
        }
        printf("\n");
    }

    return 0;
}