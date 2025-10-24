#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

// For testing purposes, we simulate the Master object.
typedef struct {
    const char* secret;
    const char* const* word_list;
    int word_list_size;
    int allowed_guesses;
    int guesses_made;
    bool found;
} Master;

int countMatches(const char* w1, const char* w2) {
    int matches = 0;
    for (int i = 0; i < 6; ++i) { // Words are 6 letters long
        if (w1[i] == w2[i]) {
            matches++;
        }
    }
    return matches;
}

int master_guess(Master* master, const char* word) {
    master->guesses_made++;
    
    bool in_list = false;
    for (int i = 0; i < master->word_list_size; ++i) {
        if (strcmp(word, master->word_list[i]) == 0) {
            in_list = true;
            break;
        }
    }
    if (!in_list) {
        return -1;
    }

    if (strcmp(word, master->secret) == 0) {
        master->found = true;
    }
    return countMatches(master->secret, word);
}

void findSecretWord(const char* const* words, int wordsSize, Master* master, int allowedGuesses) {
    int* candidates = (int*)malloc(wordsSize * sizeof(int));
    if (candidates == NULL) return;
    for (int i = 0; i < wordsSize; ++i) {
        candidates[i] = i;
    }
    int num_candidates = wordsSize;

    for (int i = 0; i < allowedGuesses && num_candidates > 0; ++i) {
        const char* bestGuess = "";
        int minMaxGroupSize = INT_MAX;

        for (int j = 0; j < wordsSize; ++j) {
            const char* w1 = words[j];
            int groups[7] = {0}; // 0-6 matches

            for (int k = 0; k < num_candidates; ++k) {
                const char* w2 = words[candidates[k]];
                groups[countMatches(w1, w2)]++;
            }
            
            int maxGroupSize = 0;
            for (int k = 0; k < 7; ++k) {
                if (groups[k] > maxGroupSize) {
                    maxGroupSize = groups[k];
                }
            }

            if (maxGroupSize < minMaxGroupSize) {
                minMaxGroupSize = maxGroupSize;
                bestGuess = w1;
            }
        }

        int matches = master_guess(master, bestGuess);
        if (matches == 6) {
            free(candidates);
            return;
        }

        int write_idx = 0;
        for (int j = 0; j < num_candidates; ++j) {
            const char* candidate_word = words[candidates[j]];
            if (countMatches(bestGuess, candidate_word) == matches) {
                candidates[write_idx++] = candidates[j];
            }
        }
        num_candidates = write_idx;
    }
    
    free(candidates);
}

void run_test(const char* test_name, const char* const* words, int wordsSize, const char* secret, int allowedGuesses) {
    Master master = {secret, words, wordsSize, allowedGuesses, 0, false};
    findSecretWord(words, wordsSize, &master, allowedGuesses);

    printf("%s: ", test_name);
    if (master.found && master.guesses_made <= master.allowed_guesses) {
        printf("You guessed the secret word correctly.\n");
    } else {
        printf("Either you took too many guesses, or you did not find the secret word.\n");
    }
}

int main() {
    // Test Case 1
    const char* const words1[] = {"acckzz", "ccbazz", "eiowzz", "abcczz"};
    run_test("Test Case 1", words1, 4, "acckzz", 10);

    // Test Case 2
    const char* const words2[] = {"hamada", "khaled"};
    run_test("Test Case 2", words2, 2, "hamada", 10);

    // Test Case 3
    const char* const words3[] = {"aaaaaa", "bbbbbb", "cccccc", "dddddd", "eeeeee", "ffffff"};
    run_test("Test Case 3", words3, 6, "aaaaaa", 10);

    // Test Case 4
    const char* const words4[] = {"abacab", "abacba", "ababca", "ababac", "ababab", "bababa", "babacb", "bababc", "bacaba", "bacbab"};
    run_test("Test Case 4", words4, 10, "abacab", 10);

    // Test Case 5
    const char* const words5[] = {"abcdef", "zzzzzz", "aaaaaa", "bbbbbb", "cccccc", "dddddd", "eeeeee", "ffffff", "gggggg", "hhhhhh"};
    run_test("Test Case 5", words5, 10, "zzzzzz", 15);

    return 0;
}