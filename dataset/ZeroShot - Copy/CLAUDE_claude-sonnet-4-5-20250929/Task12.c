
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

void highestScoringWord(const char* s, char* result) {
    if (s == NULL || strlen(s) == 0) {
        result[0] = '\\0';
        return;
    }
    
    char temp[1000];
    strcpy(temp, s);
    
    char* word = strtok(temp, " ");
    int highestScore = 0;
    result[0] = '\\0';
    
    while (word != NULL) {
        int score = calculateScore(word);
        if (score > highestScore) {
            highestScore = score;
            strcpy(result, word);
        }
        word = strtok(NULL, " ");
    }
}

int main() {
    char result[100];
    
    // Test case 1
    highestScoringWord("man i need a taxi up to ubud", result);
    printf("Test 1: %s\\n", result);
    
    // Test case 2
    highestScoringWord("what time are we climbing up the volcano", result);
    printf("Test 2: %s\\n", result);
    
    // Test case 3
    highestScoringWord("take me to semynak", result);
    printf("Test 3: %s\\n", result);
    
    // Test case 4
    highestScoringWord("aa b", result);
    printf("Test 4: %s\\n", result);
    
    // Test case 5
    highestScoringWord("abad", result);
    printf("Test 5: %s\\n", result);
    
    return 0;
}
