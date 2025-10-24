#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* SUCCESS_MSG = "You guessed the secret word correctly.";
static const char* FAILURE_MSG = "Either you took too many guesses, or you did not find the secret word.";

typedef struct {
    const char** words;
    int n;
    const char* secret;
    int allowed;
    int guessCount;
    int guessed;
} Master;

static int safe_len(const char* s) {
    if (!s) return 0;
    size_t l = strlen(s);
    return (l > 2147483647) ? 0 : (int)l;
}

static int in_words(Master* m, const char* w) {
    for (int i = 0; i < m->n; i++) {
        if (strncmp(m->words[i], w, 6) == 0 && safe_len(m->words[i]) == 6 && safe_len(w) == 6) return 1;
    }
    return 0;
}

static int count_matches(const char* a, const char* b) {
    int m = 0;
    for (int i = 0; i < 6; i++) {
        if (a[i] == b[i]) m++;
    }
    return m;
}

static int master_guess(Master* m, const char* w) {
    if (m == NULL) return -1;
    m->guessCount++;
    if (w == NULL || safe_len(w) != 6 || !in_words(m, w)) {
        return -1;
    }
    int matches = count_matches(w, m->secret);
    if (matches == 6) m->guessed = 1;
    return matches;
}

static const char* master_result_message(Master* m) {
    if (m->guessed && m->guessCount <= m->allowed) return SUCCESS_MSG;
    return FAILURE_MSG;
}

static const char* solve(const char** words, int n, const char* secret, int allowedGuesses) {
    if (!secret || safe_len(secret) != 6 || n <= 0) return FAILURE_MSG;

    Master master;
    master.words = words;
    master.n = n;
    master.secret = secret;
    master.allowed = (allowedGuesses < 0) ? 0 : allowedGuesses;
    master.guessCount = 0;
    master.guessed = 0;

    int* candidates = (int*)malloc(sizeof(int) * n);
    int candSize = 0;
    for (int i = 0; i < n; i++) {
        if (safe_len(words[i]) == 6) {
            candidates[candSize++] = i;
        }
    }

    while (candSize > 0 && master.guessCount < master.allowed) {
        int bestIdxPos = -1;
        int bestScore = 2147483647;
        const char* bestWord = NULL;

        for (int ci = 0; ci < candSize; ci++) {
            int idx = candidates[ci];
            const char* w = words[idx];
            int buckets[7] = {0,0,0,0,0,0,0};
            for (int cj = 0; cj < candSize; cj++) {
                int jdx = candidates[cj];
                int m = count_matches(w, words[jdx]);
                if (m >= 0 && m <= 6) buckets[m]++;
            }
            int worst = 0;
            for (int k = 0; k < 7; k++) if (buckets[k] > worst) worst = buckets[k];
            if (worst < bestScore || (worst == bestScore && (bestWord == NULL || strncmp(w, bestWord, 6) < 0))) {
                bestScore = worst;
                bestIdxPos = ci;
                bestWord = w;
            }
        }

        if (bestIdxPos < 0) break;

        const char* guessWord = words[candidates[bestIdxPos]];
        int res = master_guess(&master, guessWord);
        if (res == 6) break;
        if (res < 0) {
            // remove guessed candidate
            for (int i = bestIdxPos; i + 1 < candSize; i++) candidates[i] = candidates[i + 1];
            candSize--;
            continue;
        }

        int* next = (int*)malloc(sizeof(int) * candSize);
        int nextSize = 0;
        for (int i = 0; i < candSize; i++) {
            int idx = candidates[i];
            if (count_matches(words[idx], guessWord) == res) {
                next[nextSize++] = idx;
            }
        }
        free(candidates);
        candidates = next;
        candSize = nextSize;
    }

    const char* msg = master_result_message(&master);
    free(candidates);
    return msg;
}

int main(void) {
    const char* w1[] = {"acckzz", "ccbazz", "eiowzz", "abcczz"};
    const char* s1 = "acckzz";
    int a1 = 10;

    const char* w2[] = {"hamada", "khaled"};
    const char* s2 = "hamada";
    int a2 = 10;

    const char* w3[] = {"aaaaaa", "bbbbbb", "cccccc", "dddddd", "eeffee", "ffeeee", "fedcba", "abcdef"};
    const char* s3 = "abcdef";
    int a3 = 10;

    const char* w4[] = {"absdef", "abqdef", "abzdef", "abzcef", "abzcdx", "abzcdz", "abzcdy", "abztdy", "qbztdy", "xyzabc"};
    const char* s4 = "abzcdz";
    int a4 = 10;

    const char* w5[] = {"nearer", "bearer", "hearer", "seared", "neared", "dealer", "leader", "linear"};
    const char* s5 = "neared";
    int a5 = 12;

    printf("%s\n", solve(w1, (int)(sizeof(w1) / sizeof(w1[0])), s1, a1));
    printf("%s\n", solve(w2, (int)(sizeof(w2) / sizeof(w2[0])), s2, a2));
    printf("%s\n", solve(w3, (int)(sizeof(w3) / sizeof(w3[0])), s3, a3));
    printf("%s\n", solve(w4, (int)(sizeof(w4) / sizeof(w4[0])), s4, a4));
    printf("%s\n", solve(w5, (int)(sizeof(w5) / sizeof(w5[0])), s5, a5));
    return 0;
}