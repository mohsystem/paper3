#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/*
Chain-of-Through process:
1) Understand: Guess a secret 6-letter word by querying a Master.guess within a limit.
2) Security: Validate inputs, fixed-size buffers, avoid UB, bounds checks.
3) Secure coding: Use const where possible, no dynamic allocations beyond necessity, check lengths.
4) Review: Minimax strategy; controlled loops; safe string ops.
5) Output: Return only specified messages.
*/

#define MAX_WORDS 110
#define WORD_LEN 6

typedef struct {
    const char* words[MAX_WORDS];
    int n;
    const char* secret;
    int allowed;
    int calls;
    bool success;
} Master;

static int safe_match(const char* a, const char* b) {
    int m = 0;
    for (int i=0;i<WORD_LEN;i++) if (a[i]==b[i]) m++;
    return m;
}

static bool in_dict(Master* m, const char* w) {
    for (int i=0;i<m->n;i++) {
        if (strncmp(m->words[i], w, WORD_LEN)==0 && m->words[i][WORD_LEN]=='\0' && w[WORD_LEN]=='\0') return true;
    }
    return false;
}

static int master_guess(Master* m, const char* w) {
    if (!w || (int)strlen(w) != WORD_LEN || !in_dict(m, w)) return -1;
    m->calls++;
    int res = safe_match(m->secret, w);
    if (res == WORD_LEN) m->success = true;
    return res;
}

static const char* master_result(Master* m) {
    if (m->success && m->calls <= m->allowed) return "You guessed the secret word correctly.";
    return "Either you took too many guesses, or you did not find the secret word.";
}

static int choose_guess_index(int candIdx[], int candCount, Master* master) {
    int bestIdx = candIdx[0];
    int bestScore = 1<<30;
    for (int ii=0; ii<candCount; ii++) {
        int i = candIdx[ii];
        int bucket[7] = {0,0,0,0,0,0,0};
        for (int jj=0; jj<candCount; jj++) {
            int j = candIdx[jj];
            int mc = safe_match(master->words[i], master->words[j]);
            if (mc < 0) mc = 0; if (mc > 6) mc = 6;
            bucket[mc]++;
        }
        int worst = 0;
        for (int k=0;k<7;k++) if (bucket[k] > worst) worst = bucket[k];
        if (worst < bestScore) {
            bestScore = worst;
            bestIdx = i;
        }
    }
    return bestIdx;
}

const char* solve(const char* words[], int wordsCount, const char* secret, int allowedGuesses) {
    if (!secret || (int)strlen(secret)!=WORD_LEN || wordsCount<1 || wordsCount>100) {
        return "Either you took too many guesses, or you did not find the secret word.";
    }
    Master m;
    m.n = 0;
    for (int i=0;i<wordsCount && m.n < MAX_WORDS;i++) {
        if (words[i] && (int)strlen(words[i])==WORD_LEN) {
            m.words[m.n++] = words[i];
        }
    }
    bool found = false;
    for (int i=0;i<m.n;i++) {
        if (strncmp(m.words[i], secret, WORD_LEN)==0 && m.words[i][WORD_LEN]=='\0' && secret[WORD_LEN]=='\0') { found = true; break; }
    }
    if (!found) return "Either you took too many guesses, or you did not find the secret word.";
    m.secret = secret;
    m.allowed = allowedGuesses < 0 ? 0 : allowedGuesses;
    m.calls = 0;
    m.success = false;

    int candIdx[MAX_WORDS];
    int candCount = 0;
    for (int i=0;i<m.n;i++) candIdx[candCount++] = i;

    for (int g=0; g<m.allowed && candCount>0; g++) {
        int gi = choose_guess_index(candIdx, candCount, &m);
        int res = master_guess(&m, m.words[gi]);
        if (res == WORD_LEN) break;
        int nextIdx[MAX_WORDS]; int nextCount = 0;
        for (int ci=0; ci<candCount; ci++) {
            int wi = candIdx[ci];
            if (safe_match(m.words[wi], m.words[gi]) == res) {
                nextIdx[nextCount++] = wi;
            }
        }
        candCount = nextCount;
        for (int t=0;t<candCount;t++) candIdx[t] = nextIdx[t];
    }
    return master_result(&m);
}

int main(void) {
    const char* w1[] = {"acckzz","ccbazz","eiowzz","abcczz"};
    const char* w2[] = {"hamada","khaled"};
    const char* w3[] = {"bbbbbb"};
    const char* w4[] = {"kitten","kittey","bitten","sitten","mitten","kittzz"};
    const char* w5[] = {"acbdef","ghijkl","monkey","abcdef","zzzzzz","qwerty","yellow","hammer","flower","planet"};

    printf("%s\n", solve(w1, 4, "acckzz", 10));
    printf("%s\n", solve(w2, 2, "hamada", 10));
    printf("%s\n", solve(w3, 1, "bbbbbb", 1));
    printf("%s\n", solve(w4, 6, "mitten", 10));
    printf("%s\n", solve(w5, 10, "flower", 20));
    return 0;
}