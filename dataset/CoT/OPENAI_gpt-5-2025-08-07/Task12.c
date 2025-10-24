/* 
Step 1: Problem understanding
Implement function to find the highest scoring word based on letter positions.
Step 2: Security requirements
Use safe iteration, avoid buffer overflows, and manage memory properly.
Step 3: Secure coding generation
Step 4: Code review
Step 5: Secure code output
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

char* highestScoringWord(const char* s) {
    if (s == NULL || *s == '\0') {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }

    const char* p = s;
    const char* bestStart = NULL;
    size_t bestLen = 0;
    long bestScore = LONG_MIN;

    const char* wordStart = NULL;
    long currentScore = 0;
    int inWord = 0;

    for (;; ++p) {
        unsigned char uc = (unsigned char)*p;
        if (uc == '\0' || isspace(uc)) {
            if (inWord) {
                // End of current word
                size_t len = (size_t)(p - wordStart);
                if (currentScore > bestScore) {
                    bestScore = currentScore;
                    bestStart = wordStart;
                    bestLen = len;
                }
                inWord = 0;
                currentScore = 0;
            }
            if (uc == '\0') break;
        } else {
            if (!inWord) {
                inWord = 1;
                wordStart = p;
                currentScore = 0;
            }
            if (*p >= 'a' && *p <= 'z') {
                currentScore += (long)(*p - 'a' + 1);
            }
        }
    }

    if (bestStart == NULL) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }

    char* result = (char*)malloc(bestLen + 1);
    if (!result) {
        return NULL; // allocation failure
    }
    memcpy(result, bestStart, bestLen);
    result[bestLen] = '\0';
    return result;
}

int main(void) {
    // 5 test cases
    const char* tests[] = {
        "man i need a taxi up to ubud",
        "what time are we climbing up the volcano",
        "take me to semynak",
        "aa b",
        "zoo aaaa"
    };
    size_t n = sizeof(tests) / sizeof(tests[0]);
    for (size_t i = 0; i < n; ++i) {
        char* res = highestScoringWord(tests[i]);
        if (res) {
            printf("%s\n", res);
            free(res);
        } else {
            printf("\n");
        }
    }
    return 0;
}