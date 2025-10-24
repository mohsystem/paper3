#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * @brief Calculates the score of a word based on alphabet positions.
 * a=1, b=2, etc.
 * @param word The word to score.
 * @return The integer score of the word.
 */
int getScore(const char* word) {
    int score = 0;
    for (int i = 0; word[i] != '\0'; ++i) {
        score += word[i] - 'a' + 1;
    }
    return score;
}

/**
 * @brief Finds the highest scoring word in a string.
 * If scores are tied, the word appearing earliest is returned.
 * The caller is responsible for freeing the returned string.
 * @param s The input string of words.
 * @return A new heap-allocated string containing the highest scoring word.
 */
char* highestScoringWord(const char* s) {
    if (s == NULL) {
        return NULL;
    }

    // strdup allocates memory for a copy of s, which we can modify with strtok.
    char* s_copy = strdup(s);
    if (s_copy == NULL) {
        return NULL; // Memory allocation failed
    }

    char* highestWordToken = NULL;
    int maxScore = -1;
    const char* delim = " ";

    char* token = strtok(s_copy, delim);
    while (token != NULL) {
        int currentScore = getScore(token);
        if (currentScore > maxScore) {
            maxScore = currentScore;
            highestWordToken = token;
        }
        token = strtok(NULL, delim);
    }
    
    char* result = NULL;
    if (highestWordToken != NULL) {
        // Allocate memory for the final result and copy the best word into it.
        result = strdup(highestWordToken);
    } else {
        // Handle empty or whitespace-only input string
        result = (char*)malloc(1);
        if (result != NULL) {
            result[0] = '\0';
        }
    }
    
    // Free the memory used by the copy of the input string.
    free(s_copy);
    
    return result;
}

int main() {
    // Test cases
    const char* testCases[] = {
        "man i need a taxi up to ubud",
        "what time are we climbing up the volcano",
        "take me to semynak",
        "aa b",
        "b aa"
    };

    const char* expectedResults[] = {
        "taxi",
        "volcano",
        "semynak",
        "aa",
        "b"
    };
    
    int num_tests = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < num_tests; ++i) {
        char* result = highestScoringWord(testCases[i]);
        printf("Input: \"%s\"\n", testCases[i]);
        if (result != NULL) {
            printf("Output: %s\n", result);
            printf("Expected: %s\n", expectedResults[i]);
            // The caller must free the memory allocated by highestScoringWord
            free(result);
        } else {
            printf("Output: (null)\n");
        }
        printf("--------------------\n");
    }

    return 0;
}