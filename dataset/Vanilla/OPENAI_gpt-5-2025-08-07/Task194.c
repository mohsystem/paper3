#include <stdio.h>
#include <string.h>

typedef struct {
    const char** words;
    int n;
    char secret[7];
    int allowed;
    int calls;
    int found;
} Master;

int matches6(const char* a, const char* b) {
    int c = 0;
    for (int i = 0; i < 6; ++i) if (a[i] == b[i]) c++;
    return c;
}

int Master_guess(Master* m, const char* word) {
    int in = 0;
    for (int i = 0; i < m->n; ++i) {
        if (strcmp(m->words[i], word) == 0) { in = 1; break; }
    }
    if (!in) return -1;
    m->calls++;
    int mm = matches6(word, m->secret);
    if (mm == 6) m->found = 1;
    return mm;
}

const char* guessSecret(const char* words[], int n, const char* secret, int allowedGuesses) {
    Master master;
    master.words = words;
    master.n = n;
    strncpy(master.secret, secret, 6);
    master.secret[6] = '\0';
    master.allowed = allowedGuesses;
    master.calls = 0;
    master.found = 0;

    int idx[100];
    int candCount = n;
    for (int i = 0; i < n; ++i) idx[i] = i;

    while (candCount > 0 && !master.found && master.calls < master.allowed) {
        // pick best by minimax
        int bestIdxPos = 0;
        int bestWorst = 1000000;
        for (int a = 0; a < candCount; ++a) {
            int counts[7] = {0,0,0,0,0,0,0};
            for (int b = 0; b < candCount; ++b) {
                int m = matches6(words[idx[a]], words[idx[b]]);
                counts[m]++;
            }
            int worst = 0;
            for (int k = 0; k <= 6; ++k) if (counts[k] > worst) worst = counts[k];
            if (worst < bestWorst) {
                bestWorst = worst;
                bestIdxPos = a;
            }
        }

        const char* g = words[idx[bestIdxPos]];
        int res = Master_guess(&master, g);
        if (res == 6) break;

        int newIdx[100];
        int newCount = 0;
        for (int i = 0; i < candCount; ++i) {
            if (strcmp(words[idx[i]], g) == 0) continue;
            if (matches6(words[idx[i]], g) == res) {
                newIdx[newCount++] = idx[i];
            }
        }
        for (int i = 0; i < newCount; ++i) idx[i] = newIdx[i];
        candCount = newCount;
    }

    if (master.found && master.calls <= master.allowed) {
        return "You guessed the secret word correctly.";
    } else {
        return "Either you took too many guesses, or you did not find the secret word.";
    }
}

int main() {
    // Test case 1
    const char* words1[] = {"acckzz","ccbazz","eiowzz","abcczz"};
    printf("%s\n", guessSecret(words1, 4, "acckzz", 10));

    // Test case 2
    const char* words2[] = {"hamada","khaled"};
    printf("%s\n", guessSecret(words2, 2, "hamada", 10));

    // Test case 3
    const char* words3[] = {"python","typhon","thynop","cython","pebble","little","button","people","planet","plenty"};
    printf("%s\n", guessSecret(words3, 10, "python", 10));

    // Test case 4
    const char* words4[] = {"planet","planer","planed","planes","platen","palnet","placet","planar"};
    printf("%s\n", guessSecret(words4, 8, "planet", 10));

    // Test case 5
    const char* words5[] = {"silver","salver","sliver","sailor","slider","silken","sinker","sifter","sinner","single"};
    printf("%s\n", guessSecret(words5, 10, "silver", 10));

    return 0;
}