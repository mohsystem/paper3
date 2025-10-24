#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

// Struct to simulate the Master object for testing
typedef struct {
    const char* secret;
    char** words;
    int wordsSize;
    int allowedGuesses;
    int guessesMade;
    int found;
} Master;

// Helper function to calculate matches between two 6-letter words.
int match(const char* a, const char* b) {
    int matches = 0;
    for (int i = 0; i < 6; i++) {
        if (a[i] == b[i]) {
            matches++;
        }
    }
    return matches;
}

// Helper to check if a word is in the original word list.
int isInWordList(const char* word, char** words, int wordsSize) {
    for (int i = 0; i < wordsSize; i++) {
        if (strcmp(word, words[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// Simulates the Master.guess function for testing.
int master_guess(Master* master, const char* word) {
    master->guessesMade++;
    if (!isInWordList(word, master->words, master->wordsSize)) {
        return -1;
    }
    int matches = match(word, master->secret);
    if (matches == 6) {
        master->found = 1;
    }
    return matches;
}

void findSecretWord(char** words, int wordsSize, Master* master) {
    char** candidates = (char**)malloc(wordsSize * sizeof(char*));
    if (!candidates) return; // Allocation failed
    memcpy(candidates, words, wordsSize * sizeof(char*));
    int candidatesSize = wordsSize;

    // The problem constraints state allowedGuesses is at most 30.
    for (int i = 0; i < 30; ++i) {
        if (candidatesSize == 0) break;

        // Minimax strategy to find the best guess.
        const char* bestGuess = candidates[0];
        int minMaxGroupSize = INT_MAX;

        for (int j = 0; j < candidatesSize; ++j) {
            const char* w1 = candidates[j];
            int groups[7] = {0};
            for (int k = 0; k < candidatesSize; ++k) {
                const char* w2 = candidates[k];
                groups[match(w1, w2)]++;
            }
            int maxGroup = 0;
            for (int k = 0; k < 7; k++) {
                if (groups[k] > maxGroup) {
                    maxGroup = groups[k];
                }
            }
            if (maxGroup < minMaxGroupSize) {
                minMaxGroupSize = maxGroup;
                bestGuess = w1;
            }
        }

        int matches = master_guess(master, bestGuess);
        if (matches == 6) {
            free(candidates);
            return;
        }

        // Filter candidates in-place for efficiency and safety.
        int nextCount = 0;
        for (int j = 0; j < candidatesSize; ++j) {
            if (match(bestGuess, candidates[j]) == matches) {
                candidates[nextCount++] = candidates[j];
            }
        }
        
        // Optionally resize the candidates array to save memory.
        char** temp = (char**)realloc(candidates, nextCount * sizeof(char*));
        if (!temp && nextCount > 0) { // Realloc failed
            free(candidates); // Free original block
            return; // Exit to prevent further issues
        }
        candidates = temp;
        candidatesSize = nextCount;
    }
    free(candidates);
}

// Reports the result for a test case.
void report(Master* master) {
    if (master->found && master->guessesMade <= master->allowedGuesses) {
        printf("You guessed the secret word correctly.\n");
    } else {
        printf("Either you took too many guesses, or you did not find the secret word.\n");
    }
    printf("Guesses made: %d\n", master->guessesMade);
}

int main() {
    // Test Case 1
    printf("Test Case 1:\n");
    char* words1[] = {"acckzz", "ccbazz", "eiowzz", "abcczz"};
    int wordsSize1 = sizeof(words1) / sizeof(words1[0]);
    Master master1 = {"acckzz", words1, wordsSize1, 10, 0, 0};
    findSecretWord(words1, wordsSize1, &master1);
    report(&master1);

    // Test Case 2
    printf("\nTest Case 2:\n");
    char* words2[] = {"hamada", "khaled"};
    int wordsSize2 = sizeof(words2) / sizeof(words2[0]);
    Master master2 = {"hamada", words2, wordsSize2, 10, 0, 0};
    findSecretWord(words2, wordsSize2, &master2);
    report(&master2);

    // Test Case 3
    printf("\nTest Case 3:\n");
    char* words3[] = {"wichbx", "oahwep", "tpulot", "eqznzs", "vvmplb", "eywinm"};
    int wordsSize3 = sizeof(words3) / sizeof(words3[0]);
    Master master3 = {"oahwep", words3, wordsSize3, 10, 0, 0};
    findSecretWord(words3, wordsSize3, &master3);
    report(&master3);

    // Test Case 4
    printf("\nTest Case 4:\n");
    char* words4[] = {"hbaczn", "aqqbxu", "ajasjw", "rdexkw", "fcceuc", "pcblmb", "bcckqc", "ccbzzw", "agagxt", "ccbazy"};
    int wordsSize4 = sizeof(words4) / sizeof(words4[0]);
    Master master4 = {"hbaczn", words4, wordsSize4, 10, 0, 0};
    findSecretWord(words4, wordsSize4, &master4);
    report(&master4);

    // Test Case 5
    printf("\nTest Case 5:\n");
    char* words5[] = {"aaaaaa", "bbbbbb", "cccccc"};
    int wordsSize5 = sizeof(words5) / sizeof(words5[0]);
    Master master5 = {"bbbbbb", words5, wordsSize5, 10, 0, 0};
    findSecretWord(words5, wordsSize5, &master5);
    report(&master5);

    return 0;
}