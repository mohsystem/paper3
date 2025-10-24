#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

char* highestScoringWord(const char* s) {
    if (s == NULL) {
        // Return an empty string for NULL input for consistency
        char* empty = (char*)malloc(1);
        if (empty != NULL) empty[0] = '\0';
        return empty;
    }

    int maxScore = -1;
    const char* bestWordStart = NULL;
    int bestWordLen = 0;
    const char* current = s;

    while (*current != '\0') {
        // Skip leading spaces
        while (*current != '\0' && isspace((unsigned char)*current)) {
            current++;
        }
        if (*current == '\0') break; 

        const char* wordStart = current;
        int currentScore = 0;
        int currentLen = 0;

        // Find the end of the word and calculate score
        while (*current != '\0' && !isspace((unsigned char)*current)) {
            currentScore += *current - 'a' + 1;
            current++;
            currentLen++;
        }

        if (currentScore > maxScore) {
            maxScore = currentScore;
            bestWordStart = wordStart;
            bestWordLen = currentLen;
        }
    }

    char* result;
    if (bestWordStart == NULL) {
        // Handle empty or whitespace-only string
        result = (char*)malloc(1);
        if (result == NULL) return NULL;
        result[0] = '\0';
    } else {
        result = (char*)malloc(bestWordLen + 1);
        if (result == NULL) return NULL;
        strncpy(result, bestWordStart, bestWordLen);
        result[bestWordLen] = '\0';
    }

    return result;
}

int main() {
    const char* testCases[] = {
        "man i need a taxi up to ubud",
        "what time are we climbing up the volcano",
        "take me to semynak",
        "aaa b",
        "b aa"
    };
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    printf("Running C Test Cases:\n");
    for (int i = 0; i < numTestCases; i++) {
        char* result = highestScoringWord(testCases[i]);
        printf("Test Case %d: \"%s\"\n", i + 1, testCases[i]);
        if (result != NULL) {
            printf("Result: %s\n", result);
            free(result); 
        } else {
            printf("Result: NULL (Memory allocation failed)\n");
        }
        printf("--------------------\n");
    }

    return 0;
}