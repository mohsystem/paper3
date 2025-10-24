#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

// Forward declaration of Master for the function signature
typedef struct Master Master;

// Mock implementation of Master for testing
struct Master {
    const char* secret;
    char** words;
    int wordsSize;
    int allowedGuesses;
    int guesses;
    int found;
};

int countMatches(const char* word1, const char* word2) {
    int matches = 0;
    for (int i = 0; i < 6; i++) {
        if (word1[i] == word2[i]) {
            matches++;
        }
    }
    return matches;
}

int master_guess(Master* master, const char* word) {
    printf("Guessing: %s\n", word);
    master->guesses++;
    if (master->guesses > master->allowedGuesses) {
         printf("Exceeded allowed guesses!\n");
    }

    int in_list = 0;
    for (int i = 0; i < master->wordsSize; i++) {
        if (strcmp(word, master->words[i]) == 0) {
            in_list = 1;
            break;
        }
    }
    if (!in_list) {
        return -1;
    }
    if (strcmp(word, master->secret) == 0) {
        master->found = 1;
    }
    return countMatches(master->secret, word);
}

void printMasterResult(Master* master) {
    if (master->found && master->guesses <= master->allowedGuesses) {
        printf("You guessed the secret word correctly.\n");
    } else {
        printf("Either you took too many guesses, or you did not find the secret word.\n");
    }
}

void findSecretWord(char** words, int wordsSize, Master* master) {
    char** candidates = (char**)malloc(wordsSize * sizeof(char*));
    if (!candidates) return;
    memcpy(candidates, words, wordsSize * sizeof(char*));
    int candidatesSize = wordsSize;

    for (int i = 0; i < 30; ++i) { // Max allowed guesses
        if (candidatesSize == 0) break;

        char* bestGuess;
        if (candidatesSize <= 2) {
             bestGuess = candidates[0];
        } else {
            int minMaxPartition = INT_MAX;
            bestGuess = words[0]; 
            for (int j = 0; j < wordsSize; ++j) {
                char* guessWord = words[j];
                int partitions[7] = {0};
                for (int k = 0; k < candidatesSize; ++k) {
                    char* candidateWord = candidates[k];
                    partitions[countMatches(guessWord, candidateWord)]++;
                }

                int maxPartition = 0;
                for (int k = 0; k < 7; ++k) {
                    if (partitions[k] > maxPartition) {
                        maxPartition = partitions[k];
                    }
                }

                if (maxPartition < minMaxPartition) {
                    minMaxPartition = maxPartition;
                    bestGuess = guessWord;
                }
            }
        }
        
        int matches = master_guess(master, bestGuess);
        if (matches == 6) {
            free(candidates);
            return;
        }

        char** nextCandidates = (char**)malloc(candidatesSize * sizeof(char*));
        if (!nextCandidates) { free(candidates); return; }
        int nextCandidatesSize = 0;
        for (int j = 0; j < candidatesSize; ++j) {
            if (countMatches(bestGuess, candidates[j]) == matches) {
                nextCandidates[nextCandidatesSize++] = candidates[j];
            }
        }
        
        free(candidates);
        candidates = (char**)realloc(nextCandidates, nextCandidatesSize * sizeof(char*));
        if (nextCandidatesSize > 0 && !candidates) return; // Realloc failed
        candidatesSize = nextCandidatesSize;
    }

    if (candidates) {
        free(candidates);
    }
}


int main() {
    // Test Case 1
    printf("--- Test Case 1 ---\n");
    char* words1[] = {"acckzz","ccbazz","eiowzz","abcczz"};
    Master master1 = {"acckzz", words1, 4, 10, 0, 0};
    findSecretWord(words1, 4, &master1);
    printMasterResult(&master1);

    // Test Case 2
    printf("\n--- Test Case 2 ---\n");
    char* words2[] = {"hamada","khaled"};
    Master master2 = {"hamada", words2, 2, 10, 0, 0};
    findSecretWord(words2, 2, &master2);
    printMasterResult(&master2);
    
    // Test Case 3
    printf("\n--- Test Case 3 ---\n");
    char* words3[] = {"wichbx","oahwep","tpulot","eqznrt","vjhszz","pcmatp","xqmsgr",
                      "bnrhml","ccoyyo","ajcwbj","arwfnl","nafmtm","xoaumd","vbejda",
                      "kaefne","swcrkh","reeyhj","vmcwaf","chxitv","qkwjna","vklpkp",
                      "xfnayl","ktgmfn","xrmzzm","fgtuki","zcffuv","srxuus","pydgmq"};
    Master master3 = {"ccoyyo", words3, 28, 15, 0, 0};
    findSecretWord(words3, 28, &master3);
    printMasterResult(&master3);
    
    // Test Case 4
    printf("\n--- Test Case 4 ---\n");
    char* words4[] = {"aaaaaa", "bbbbbb", "cccccc", "dddddd", "eeeeee", "ffffff"};
    Master master4 = {"aaaaaa", words4, 6, 10, 0, 0};
    findSecretWord(words4, 6, &master4);
    printMasterResult(&master4);
    
    // Test Case 5
    printf("\n--- Test Case 5 ---\n");
    char* words5[] = {"abacac", "abadaf", "adafac", "afacab"};
    Master master5 = {"abacac", words5, 4, 10, 0, 0};
    findSecretWord(words5, 4, &master5);
    printMasterResult(&master5);
    
    return 0;
}