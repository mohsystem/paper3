#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

static inline int letter_score(char c) {
    if (c >= 'a' && c <= 'z') return (int)(c - 'a' + 1);
    return 0;
}

char* highest_scoring_word(const char* s) {
    if (s == NULL) {
        return NULL;
    }

    size_t i = 0;
    size_t best_start = 0;
    size_t best_len = 0;
    long long best_score = LLONG_MIN;

    size_t start = 0;
    long long score = 0;
    int in_word = 0;

    while (s[i] != '\0') {
        if (s[i] == ' ') {
            if (in_word) {
                size_t len = i - start;
                if (score > best_score) {
                    best_score = score;
                    best_start = start;
                    best_len = len;
                }
                // Tie: keep earliest (do nothing)
                in_word = 0;
                score = 0;
            }
            // skip space
            i++;
            continue;
        } else {
            if (!in_word) {
                in_word = 1;
                start = i;
                score = 0;
            }
            score += letter_score(s[i]);
            i++;
        }
    }

    // finalize last word if string didn't end with space
    if (in_word) {
        size_t len = i - start;
        if (score > best_score) {
            best_score = score;
            best_start = start;
            best_len = len;
        }
    }

    if (best_score == LLONG_MIN) {
        char* empty = (char*)malloc(1);
        if (empty != NULL) empty[0] = '\0';
        return empty; // no words found
    }

    char* out = (char*)malloc(best_len + 1);
    if (out == NULL) {
        return NULL; // allocation failure
    }
    if (best_len > 0) {
        memcpy(out, s + best_start, best_len);
    }
    out[best_len] = '\0';
    return out;
}

int main(void) {
    const char* tests[5] = {
        "man i need a taxi up to ubud",
        "what time are we climbing up the volcano",
        "take me to semynak",
        "aa b",
        "b aa"
    };

    for (int t = 0; t < 5; ++t) {
        char* res = highest_scoring_word(tests[t]);
        if (res == NULL) {
            printf("Input: \"%s\" -> Highest: <allocation error or NULL>\n", tests[t]);
        } else {
            printf("Input: \"%s\" -> Highest: \"%s\"\n", tests[t], res);
            free(res);
        }
    }
    return 0;
}