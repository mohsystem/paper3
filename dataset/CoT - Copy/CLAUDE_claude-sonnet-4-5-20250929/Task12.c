
#include <stdio.h>
#include <string.h>
#include <ctype.h>

int calculateScore(const char* word) {
    int score = 0;
    for (int i = 0; word[i] != '\\0'; i++) {
        if (word[i] >= 'a' && word[i] <= 'z') {
            score += (word[i] - 'a' + 1);
        }
    }
    return score;
}

void highestScoringWord(const char* str, char* result) {
    if (str == NULL || strlen(str) == 0) {
        result[0] = '\\0';
        return;
    }
    
    char buffer[1000];
    strncpy(buffer, str, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\\0';
    
    char* token = strtok(buffer, " ");
    int maxScore = 0;
    result[0] = '\\0';
    
    while (token != NULL) {
        int score = calculateScore(token);
        if (score > maxScore) {
            maxScore = score;
            strncpy(result, token, 999);
            result[999] = '\\0';
        }
        token = strtok(NULL, " ");
    }
}

int main() {
    char result[1000];
    
    highestScoringWord("man i need a taxi up to ubud", result);
    printf("Test 1: %s\\n", result);
    
    highestScoringWord("what time are we climbing up the volcano", result);
    printf("Test 2: %s\\n", result);
    
    highestScoringWord("take me to semynak", result);
    printf("Test 3: %s\\n", result);
    
    highestScoringWord("aa b", result);
    printf("Test 4: %s\\n", result);
    
    highestScoringWord("a z", result);
    printf("Test 5: %s\\n", result);
    
    return 0;
}
