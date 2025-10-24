#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char** words;
    int n;
    char secret[7];
    int allowed;
    int calls;
} Master;

static int safe_min(int a, int b) { return a < b ? a : b; }

static int match_count(const char* a, const char* b) {
    if (!a || !b) return -1;
    size_t la = strlen(a), lb = strlen(b);
    if (la != lb) return -1;
    int cnt = 0;
    for (size_t i = 0; i < la; ++i) {
        if (a[i] == b[i]) cnt++;
    }
    return cnt;
}

static int in_wordlist(Master* m, const char* w) {
    if (!m || !w) return 0;
    for (int i = 0; i < m->n; ++i) {
        if (strcmp(m->words[i], w) == 0) return 1;
    }
    return 0;
}

static int master_guess(Master* m, const char* w) {
    if (!m || !w) return -1;
    m->calls++;
    if (!in_wordlist(m, w)) return -1;
    return match_count(w, m->secret);
}

static char* choose_best_guess(char** cands, int candCount) {
    if (candCount <= 0) return NULL;
    int bestZero = 1e9;
    int bestIdx = 0;
    for (int i = 0; i < candCount; ++i) {
        int zeros = 0;
        for (int j = 0; j < candCount; ++j) {
            if (i == j) continue;
            if (match_count(cands[i], cands[j]) == 0) zeros++;
        }
        if (zeros < bestZero) {
            bestZero = zeros;
            bestIdx = i;
        }
    }
    return cands[bestIdx];
}

static int find_secret_word(char** words, int n, Master* m) {
    if (!words || n <= 0 || !m) return 0;
    char** cands = (char**)malloc(sizeof(char*) * n);
    if (!cands) return 0;
    int candCount = 0;
    for (int i = 0; i < n; ++i) cands[candCount++] = words[i];

    while (candCount > 0 && m->calls < m->allowed) {
        char* guess = choose_best_guess(cands, candCount);
        if (!guess) break;
        int res = master_guess(m, guess);
        if (res == 6) { free(cands); return 1; }
        if (res < 0) { free(cands); return 0; }
        char** next = (char**)malloc(sizeof(char*) * candCount);
        if (!next) { free(cands); return 0; }
        int nextCount = 0;
        for (int i = 0; i < candCount; ++i) {
            if (match_count(cands[i], guess) == res) {
                next[nextCount++] = cands[i];
            }
        }
        free(cands);
        cands = next;
        candCount = nextCount;
        if (candCount == 1 && m->calls < m->allowed) {
            int finalRes = master_guess(m, cands[0]);
            free(cands);
            return finalRes == 6;
        }
    }
    free(cands);
    return 0;
}

static void free_master(Master* m) {
    if (!m) return;
    if (m->words) {
        for (int i = 0; i < m->n; ++i) {
            if (m->words[i]) free(m->words[i]);
        }
        free(m->words);
        m->words = NULL;
    }
}

const char* solve_test_case(const char* secret, const char** wordsInput, int n, int allowedGuesses) {
    if (!secret || !wordsInput || n <= 0) return "Either you took too many guesses, or you did not find the secret word.";
    // Filter to 6-letter words and copy
    char** words = (char**)malloc(sizeof(char*) * n);
    if (!words) return "Either you took too many guesses, or you did not find the secret word.";
    int count = 0;
    for (int i = 0; i < n; ++i) {
        const char* w = wordsInput[i];
        if (!w) continue;
        if ((int)strlen(w) != 6) continue;
        words[count] = (char*)malloc(7);
        if (!words[count]) {
            // cleanup partial
            for (int k = 0; k < count; ++k) free(words[k]);
            free(words);
            return "Either you took too many guesses, or you did not find the secret word.";
        }
        strncpy(words[count], w, 6);
        words[count][6] = '\0';
        count++;
    }
    // Ensure secret in list
    int found = 0;
    for (int i = 0; i < count; ++i) if (strncmp(words[i], secret, 6) == 0) { found = 1; break; }
    if (!found) {
        // add secret if not present (to match constraints, but be safe)
        char** nw = (char**)realloc(words, sizeof(char*) * (count + 1));
        if (!nw) {
            for (int k = 0; k < count; ++k) free(words[k]);
            free(words);
            return "Either you took too many guesses, or you did not find the secret word.";
        }
        words = nw;
        words[count] = (char*)malloc(7);
        if (!words[count]) {
            for (int k = 0; k < count; ++k) free(words[k]);
            free(words);
            return "Either you took too many guesses, or you did not find the secret word.";
        }
        strncpy(words[count], secret, 6);
        words[count][6] = '\0';
        count++;
    }

    Master m;
    m.n = count;
    m.words = words;
    strncpy(m.secret, secret, 6);
    m.secret[6] = '\0';
    m.allowed = allowedGuesses;
    m.calls = 0;

    int guessed = find_secret_word(m.words, m.n, &m);
    const char* msg;
    if (guessed && m.calls <= allowedGuesses) {
        msg = "You guessed the secret word correctly.";
    } else {
        msg = "Either you took too many guesses, or you did not find the secret word.";
    }
    free_master(&m);
    return msg;
}

int main(void) {
    // Test case 1
    const char* words1[] = {"acckzz","ccbazz","eiowzz","abcczz"};
    printf("%s\n", solve_test_case("acckzz", words1, 4, 10));

    // Test case 2
    const char* words2[] = {"hamada","khaled"};
    printf("%s\n", solve_test_case("hamada", words2, 2, 10));

    // Test case 3
    const char* words3[] = {"spiral","sprint","sprout","sprite","spirit","spring"};
    printf("%s\n", solve_test_case("sprite", words3, 6, 10));

    // Test case 4
    const char* words4[] = {"abacus","absurd","acumen","agenda","zenith","shrimp","zigzag","little","letter","legend"};
    printf("%s\n", solve_test_case("zenith", words4, 10, 15));

    // Test case 5
    const char* words5[] = {"orange","banana","tomato","pepper","potato","carrot","onions","radish","celery","garlic"};
    printf("%s\n", solve_test_case("orange", words5, 10, 12));

    return 0;
}