#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char* high(const char* s) {
    int i = 0;
    int currScore = 0, currLen = 0;
    const char* currStart = NULL;

    int bestScore = -1, bestLen = 0;
    const char* bestStart = NULL;

    while (1) {
        char ch = s[i];
        if (ch >= 'a' && ch <= 'z') {
            if (currLen == 0) currStart = &s[i];
            currLen++;
            currScore += (ch - 'a' + 1);
            i++;
        } else if (ch == ' ' || ch == '\0') {
            if (currLen > 0) {
                if (currScore > bestScore) {
                    bestScore = currScore;
                    bestLen = currLen;
                    bestStart = currStart;
                }
                currLen = 0;
                currScore = 0;
                currStart = NULL;
            }
            if (ch == '\0') break;
            i++;
        } else {
            // Unexpected char; finalize current word and continue
            if (currLen > 0) {
                if (currScore > bestScore) {
                    bestScore = currScore;
                    bestLen = currLen;
                    bestStart = currStart;
                }
                currLen = 0;
                currScore = 0;
                currStart = NULL;
            }
            i++;
        }
    }

    if (bestStart == NULL) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    char* result = (char*)malloc((size_t)bestLen + 1);
    if (!result) return NULL;
    memcpy(result, bestStart, (size_t)bestLen);
    result[bestLen] = '\0';
    return result;
}

int main() {
    const char* tests[5] = {
        "man i need a taxi up to ubud",
        "what time are we climbing up the volcano",
        "take me to semynak",
        "aa b",
        "b aa"
    };
    for (int i = 0; i < 5; ++i) {
        char* res = high(tests[i]);
        if (res) {
            printf("%s\n", res);
            free(res);
        } else {
            printf("\n");
        }
    }
    return 0;
}