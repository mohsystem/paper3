#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

char* highest_scoring_word(const char* s) {
    if (s == NULL) {
        return NULL;
    }

    const char* p = s;
    const char* bestStart = NULL;
    size_t bestLen = 0;
    long long bestScore = LLONG_MIN;

    const char* curStart = NULL;
    long long curScore = 0;

    for (;; ++p) {
        char c = *p;
        if (c >= 'a' && c <= 'z') {
            if (curStart == NULL) {
                curStart = p;
                curScore = 0;
            }
            curScore += (long long)(c - 'a' + 1);
        } else {
            if (curStart != NULL) {
                size_t curLen = (size_t)(p - curStart);
                if (curScore > bestScore) {
                    bestScore = curScore;
                    bestStart = curStart;
                    bestLen = curLen;
                }
                curStart = NULL;
                curScore = 0;
            }
            if (c == '\0') {
                break;
            }
        }
    }

    if (bestStart == NULL) {
        char* res = (char*)malloc(1);
        if (res != NULL) {
            res[0] = '\0';
        }
        return res;
    }

    char* res = (char*)malloc(bestLen + 1);
    if (res == NULL) {
        return NULL;
    }
    memcpy(res, bestStart, bestLen);
    res[bestLen] = '\0';
    return res;
}

int main(void) {
    const char* tests[] = {
        "abad aa bb",
        "man i need a taxi up to ubud",
        "what time are we climbing up the volcano",
        "take me to semynak",
        "aa b"
    };

    for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); ++i) {
        char* result = highest_scoring_word(tests[i]);
        if (result != NULL) {
            printf("%s\n", result);
            free(result);
        } else {
            printf("\n");
        }
    }
    return 0;
}