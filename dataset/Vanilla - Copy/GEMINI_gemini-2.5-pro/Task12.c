#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* high(const char* s) {
    if (s == NULL) return NULL;
    
    // Use strdup to create a mutable copy of the string for strtok
    char* str_copy = strdup(s);
    if (str_copy == NULL) {
        return NULL; // Memory allocation failed
    }

    char* highestScoringWord = (char*)malloc(1);
    if (highestScoringWord == NULL) {
        free(str_copy);
        return NULL; // Memory allocation failed
    }
    highestScoringWord[0] = '\0';
    
    int highestScore = 0;
    const char* delimiters = " ";
    char* token = strtok(str_copy, delimiters);

    while (token != NULL) {
        int currentScore = 0;
        for (int i = 0; token[i] != '\0'; i++) {
            currentScore += token[i] - 'a' + 1;
        }

        if (currentScore > highestScore) {
            highestScore = currentScore;
            // Reallocate memory for the new highest scoring word and copy it
            char* new_highest = (char*)realloc(highestScoringWord, strlen(token) + 1);
            if (new_highest == NULL) {
                free(highestScoringWord);
                free(str_copy);
                return NULL; // Memory allocation failed
            }
            highestScoringWord = new_highest;
            strcpy(highestScoringWord, token);
        }

        token = strtok(NULL, delimiters);
    }

    free(str_copy);
    return highestScoringWord; // Caller is responsible for freeing this memory
}

int main() {
    const char* testCases[] = {
        "man i need a taxi up to ubud",
        "what time are we climbing up the volcano",
        "take me to semynak",
        "bb d",
        "d bb"
    };
    int num_tests = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < num_tests; i++) {
        printf("Input: \"%s\"\n", testCases[i]);
        char* result = high(testCases[i]);
        if (result != NULL) {
            printf("Output: \"%s\"\n\n", result);
            free(result); // Free the memory allocated by high()
        } else {
            printf("An error occurred during processing.\n\n");
        }
    }

    return 0;
}