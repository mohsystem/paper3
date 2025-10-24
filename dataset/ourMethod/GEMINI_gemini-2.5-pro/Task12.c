#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Function to calculate the score of a word based on alphabet positions.
int calculateScore(const char* word) {
    int score = 0;
    if (word == NULL) {
        return 0;
    }
    for (size_t i = 0; word[i] != '\0'; ++i) {
        if (word[i] >= 'a' && word[i] <= 'z') {
            score += word[i] - 'a' + 1;
        }
    }
    return score;
}

// Function to find the highest scoring word in a string.
// If scores are tied, the word that appears first is returned.
// The caller is responsible for freeing the returned string.
char* highestScoringWord(const char* str) {
    if (str == NULL) {
        return NULL;
    }

    size_t str_len = strlen(str);
    if (str_len == 0) {
        char* empty_result = (char*)malloc(1);
        if (empty_result != NULL) {
            empty_result[0] = '\0';
        }
        return empty_result;
    }

    char* str_copy = (char*)malloc(str_len + 1);
    if (str_copy == NULL) {
        return NULL;
    }
    memcpy(str_copy, str, str_len + 1);

    char* highestWord = (char*)malloc(str_len + 1);
    if (highestWord == NULL) {
        free(str_copy);
        return NULL;
    }
    highestWord[0] = '\0';

    int maxScore = -1;
    const char* delimiters = " ";
    char* token = strtok(str_copy, delimiters);

    while (token != NULL) {
        int currentScore = calculateScore(token);
        if (currentScore > maxScore) {
            maxScore = currentScore;
            size_t token_len = strlen(token);
            memcpy(highestWord, token, token_len + 1);
        }
        token = strtok(NULL, delimiters);
    }

    free(str_copy);

    size_t result_len = strlen(highestWord);
    char* final_result = (char*)realloc(highestWord, result_len + 1);
    if (final_result == NULL) {
        // If realloc fails, return the original buffer
        return highestWord;
    }
    
    return final_result;
}

int main() {
    // Test Case 1
    const char* input1 = "man i need a taxi up to ubud";
    char* result1 = highestScoringWord(input1);
    if (result1) {
        printf("Input: \"%s\"\nHighest scoring word: %s\n\n", input1, result1);
        free(result1);
    }

    // Test Case 2
    const char* input2 = "what time are we climbing up the volcano";
    char* result2 = highestScoringWord(input2);
    if (result2) {
        printf("Input: \"%s\"\nHighest scoring word: %s\n\n", input2, result2);
        free(result2);
    }

    // Test Case 3
    const char* input3 = "take me to semynak";
    char* result3 = highestScoringWord(input3);
    if (result3) {
        printf("Input: \"%s\"\nHighest scoring word: %s\n\n", input3, result3);
        free(result3);
    }

    // Test Case 4: Tie, 'b' (2) vs 'aa' (2). 'aa' appears first.
    const char* input4 = "aa b";
    char* result4 = highestScoringWord(input4);
    if (result4) {
        printf("Input: \"%s\"\nHighest scoring word: %s\n\n", input4, result4);
        free(result4);
    }

    // Test Case 5: Tie, 'b' (2) vs 'aa' (2). 'b' appears first.
    const char* input5 = "b aa";
    char* result5 = highestScoringWord(input5);
    if (result5) {
        printf("Input: \"%s\"\nHighest scoring word: %s\n\n", input5, result5);
        free(result5);
    }
    
    return 0;
}