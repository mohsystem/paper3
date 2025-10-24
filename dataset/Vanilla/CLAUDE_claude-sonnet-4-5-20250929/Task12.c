
#include <stdio.h>
#include <string.h>
#include <ctype.h>

void highestScoringWord(const char* s, char* result) {
    char word[100];
    char highestWord[100] = "";
    int highestScore = 0;
    int i = 0, j = 0;
    
    while (s[i] != '\\0') {
        if (s[i] != ' ') {
            word[j++] = s[i];
        } else {
            word[j] = '\\0';
            int score = 0;
            for (int k = 0; k < j; k++) {
                score += word[k] - 'a' + 1;
            }
            if (score > highestScore) {
                highestScore = score;
                strcpy(highestWord, word);
            }
            j = 0;
        }
        i++;
    }
    
    // Process last word
    word[j] = '\\0';
    int score = 0;
    for (int k = 0; k < j; k++) {
        score += word[k] - 'a' + 1;
    }
    if (score > highestScore) {
        highestScore = score;
        strcpy(highestWord, word);
    }
    
    strcpy(result, highestWord);
}

int main() {
    char result[100];
    
    // Test case 1
    highestScoringWord("man i need a taxi up to ubud", result);
    printf("%s\\n", result);
    
    // Test case 2
    highestScoringWord("what time are we climbing up the volcano", result);
    printf("%s\\n", result);
    
    // Test case 3
    highestScoringWord("take me to semynak", result);
    printf("%s\\n", result);
    
    // Test case 4
    highestScoringWord("aa b", result);
    printf("%s\\n", result);
    
    // Test case 5
    highestScoringWord("abad", result);
    printf("%s\\n", result);
    
    return 0;
}
