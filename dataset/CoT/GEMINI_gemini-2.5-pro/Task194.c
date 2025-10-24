#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// --- Master "object" Simulation for Testing ---
typedef struct {
    const char* secret;
    char** wordlist;
    int wordlist_size;
    int guess_count;
    int allowed_guesses;
    bool solved;
} Master;

int match(const char* a, const char* b) {
    int matches = 0;
    for (int i = 0; i < 6; i++) {
        if (a[i] == b[i]) {
            matches++;
        }
    }
    return matches;
}

// This function simulates the Master.guess API call
int master_guess_impl(Master* master, const char* word) {
    master->guess_count++;
    
    bool in_list = false;
    for (int i = 0; i < master->wordlist_size; i++) {
        if (strcmp(master->wordlist[i], word) == 0) {
            in_list = true;
            break;
        }
    }
    if (!in_list) {
        return -1;
    }
    
    int matches = match(master->secret, word);
    if (matches == 6) {
        master->solved = true;
    }
    return matches;
}

// Function pointer type for the guess function to be passed to the solver
typedef int (*GuessFunc)(Master*, const char*);


void findSecretWord(char* words[], int wordsSize, Master* master, GuessFunc guess_func, int allowedGuesses) {
    char* candidates[100]; // Max size from constraints
    int candidatesSize = wordsSize;
    for (int i = 0; i < wordsSize; i++) {
        candidates[i] = words[i];
    }

    for (int i = 0; i < allowedGuesses; i++) {
        if (candidatesSize == 0) return;

        char* best_guess = candidates[0];
        int minMaxGroupSize = candidatesSize;

        for (int j = 0; j < candidatesSize; j++) {
            char* w1 = candidates[j];
            int groups[7] = {0};
            for (int k = 0; k < candidatesSize; k++) {
                char* w2 = candidates[k];
                groups[match(w1, w2)]++;
            }

            int maxGroupSize = 0;
            for (int k = 0; k < 7; k++) {
                if (groups[k] > maxGroupSize) {
                    maxGroupSize = groups[k];
                }
            }
            if (maxGroupSize < minMaxGroupSize) {
                minMaxGroupSize = maxGroupSize;
                best_guess = w1;
            }
        }
        
        int matches = guess_func(master, best_guess);
        if (matches == 6) {
            return;
        }

        char* newCandidates[100];
        int newCandidatesSize = 0;
        for (int j = 0; j < candidatesSize; j++) {
            if (match(candidates[j], best_guess) == matches) {
                newCandidates[newCandidatesSize++] = candidates[j];
            }
        }

        candidatesSize = newCandidatesSize;
        for (int j = 0; j < candidatesSize; j++) {
            candidates[j] = newCandidates[j];
        }
    }
}

void print_master_result(Master* master) {
    if (master->solved && master->guess_count <= master->allowed_guesses) {
        printf("You guessed the secret word correctly.\n");
    } else {
        printf("Either you took too many guesses, or you did not find the secret word.\n");
    }
}

int main() {
    // Test Case 1
    printf("Test Case 1:\n");
    char* words1[] = {"acckzz","ccbazz","eiowzz","abcczz"};
    Master master1 = { "acckzz", words1, 4, 0, 10, false };
    findSecretWord(words1, 4, &master1, master_guess_impl, 10);
    print_master_result(&master1);

    // Test Case 2
    printf("\nTest Case 2:\n");
    char* words2[] = {"hamada", "khaled"};
    Master master2 = { "hamada", words2, 2, 0, 10, false };
    findSecretWord(words2, 2, &master2, master_guess_impl, 10);
    print_master_result(&master2);

    // Test Case 3
    printf("\nTest Case 3:\n");
    char* words3[] = {"rccqcb","acckzz","ccbazz","eiowzz","abcczz", "hamada", "bobson", "vrtrva"};
    Master master3 = { "bobson", words3, 8, 0, 10, false };
    findSecretWord(words3, 8, &master3, master_guess_impl, 10);
    print_master_result(&master3);

    // Test Case 4
    printf("\nTest Case 4:\n");
    char* words4[] = {"banana","cabana","banaba","bandan","banban","bonbon"};
    Master master4 = { "banana", words4, 6, 0, 15, false };
    findSecretWord(words4, 6, &master4, master_guess_impl, 15);
    print_master_result(&master4);

    // Test Case 5
    printf("\nTest Case 5:\n");
    char* words5[] = {"abcdef","abdfed","acbdef","aefdcb","abdcef","abcefd"};
    Master master5 = { "abcdef", words5, 6, 0, 10, false };
    findSecretWord(words5, 6, &master5, master_guess_impl, 10);
    print_master_result(&master5);

    return 0;
}