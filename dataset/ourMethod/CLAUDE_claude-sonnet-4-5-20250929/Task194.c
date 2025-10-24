
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

// Master API structure for C
typedef struct {
    char secret[7]; // 6 chars + null terminator
    int guessCount;
    int maxGuesses;
} Master;

// Initialize Master - returns 0 on success, -1 on failure
int Master_init(Master* m, const char* s, int max) {
    // Validate input pointer - prevent null pointer dereference
    if (m == NULL || s == NULL) {
        return -1;
    }
    
    // Validate secret length - bounds checking
    size_t len = strnlen(s, 8); // Check up to 8 to detect overflow
    if (len != 6) {
        return -1;
    }
    
    // Validate secret contains only lowercase letters - input validation
    for (size_t i = 0; i < 6; i++) {
        if (s[i] < 'a' || s[i] > 'z') {
            return -1;
        }
    }
    
    // Safe copy with bounds checking - prevent buffer overflow CWE-120
    strncpy(m->secret, s, 6);
    m->secret[6] = '\\0'; // Ensure null termination
    m->guessCount = 0;
    m->maxGuesses = max;
    
    return 0;
}

// Master guess function
int Master_guess(Master* m, const char* word) {
    // Validate input pointers - prevent null pointer dereference
    if (m == NULL || word == NULL) {
        return -1;
    }
    
    // Validate word length - bounds checking
    size_t len = strnlen(word, 8);
    if (len != 6) {
        return -1;
    }
    
    // Validate word contains only lowercase letters - input validation
    for (size_t i = 0; i < 6; i++) {
        if (word[i] < 'a' || word[i] > 'z') {
            return -1;
        }
    }
    
    if (m->guessCount >= m->maxGuesses) {
        return -1;
    }
    
    m->guessCount++;
    
    int matches = 0;
    // Safe comparison with explicit bounds
    for (size_t i = 0; i < 6; i++) {
        if (word[i] == m->secret[i]) {
            matches++;
        }
    }
    
    return matches;
}

// Calculate match count between two words
int matchCount(const char* a, const char* b) {
    // Validate input pointers - prevent null pointer dereference
    if (a == NULL || b == NULL) {
        return 0;
    }
    
    // Validate lengths - bounds checking
    if (strnlen(a, 7) != 6 || strnlen(b, 7) != 6) {
        return 0;
    }
    
    int count = 0;
    for (size_t i = 0; i < 6; i++) {
        if (a[i] == b[i]) {
            count++;
        }
    }
    return count;
}

// Find secret word function
void findSecretWord(char** words, int wordsSize, Master* master) {
    // Input validation - check for null pointers
    if (words == NULL || master == NULL || wordsSize <= 0) {
        return;
    }
    
    // Validate all words - security measure
    for (int i = 0; i < wordsSize; i++) {
        if (words[i] == NULL) {
            return;
        }
        size_t len = strnlen(words[i], 8);
        if (len != 6) {
            return;
        }
        for (size_t j = 0; j < 6; j++) {
            if (words[i][j] < 'a' || words[i][j] > 'z') {
                return;
            }
        }
    }
    
    // Allocate candidate tracking array - check allocation
    bool* isCandidate = (bool*)calloc(wordsSize, sizeof(bool));
    if (isCandidate == NULL) {
        return; // Memory allocation failure
    }
    
    // Initialize all as candidates
    int candidateCount = wordsSize;
    for (int i = 0; i < wordsSize; i++) {
        isCandidate[i] = true;
    }
    
    const int MAX_ITERATIONS = 30;
    int iterations = 0;
    
    while (candidateCount > 0 && iterations < MAX_ITERATIONS) {
        iterations++;
        
        // Find best guess using minimax strategy
        int bestGuessIdx = -1;
        int minMaxGroup = candidateCount + 1;
        
        for (int i = 0; i < wordsSize; i++) {
            if (!isCandidate[i]) continue;
            
            int groups[7] = {0}; // matches can be 0-6
            
            for (int j = 0; j < wordsSize; j++) {
                if (!isCandidate[j]) continue;
                
                int matches = matchCount(words[i], words[j]);
                // Validate match count range
                if (matches >= 0 && matches <= 6) {
                    groups[matches]++;
                }
            }
            
            int maxGroup = 0;
            for (int g = 0; g < 7; g++) {
                if (groups[g] > maxGroup) {
                    maxGroup = groups[g];
                }
            }
            
            if (maxGroup < minMaxGroup) {
                minMaxGroup = maxGroup;
                bestGuessIdx = i;
            }
        }
        
        if (bestGuessIdx == -1) break;
        
        int matches = Master_guess(master, words[bestGuessIdx]);
        
        // If guessed correctly, exit
        if (matches == 6) {
            free(isCandidate); // Free allocated memory
            return;
        }
        
        // If guess failed, remove from candidates
        if (matches == -1) {
            isCandidate[bestGuessIdx] = false;
            candidateCount--;
            continue;
        }
        
        // Filter candidates
        for (int i = 0; i < wordsSize; i++) {
            if (!isCandidate[i]) continue;
            
            if (matchCount(words[bestGuessIdx], words[i]) != matches) {
                isCandidate[i] = false;
                candidateCount--;
            }
        }
    }
    
    // Free allocated memory - prevent memory leak
    free(isCandidate);
}

int main() {
    // Test case 1
    {
        char* words1[] = {"acckzz", "ccbazz", "eiowzz", "abcczz"};
        Master master1;
        if (Master_init(&master1, "acckzz", 10) == 0) {
            findSecretWord(words1, 4, &master1);
            printf("Test 1: %s\\n", master1.guessCount <= 10 ? "PASS" : "FAIL");
        }
    }
    
    // Test case 2
    {
        char* words2[] = {"hamada", "khaled"};
        Master master2;
        if (Master_init(&master2, "hamada", 10) == 0) {
            findSecretWord(words2, 2, &master2);
            printf("Test 2: %s\\n", master2.guessCount <= 10 ? "PASS" : "FAIL");
        }
    }
    
    // Test case 3
    {
        char* words3[] = {"gaxckt", "terlnx", "jbhanu", "cpjbzf"};
        Master master3;
        if (Master_init(&master3, "gaxckt", 10) == 0) {
            findSecretWord(words3, 4, &master3);
            printf("Test 3: %s\\n", master3.guessCount <= 10 ? "PASS" : "FAIL");
        }
    }
    
    // Test case 4
    {
        char* words4[] = {"abcdef"};
        Master master4;
        if (Master_init(&master4, "abcdef", 10) == 0) {
            findSecretWord(words4, 1, &master4);
            printf("Test 4: %s\\n", master4.guessCount <= 10 ? "PASS" : "FAIL");
        }
    }
    
    // Test case 5
    {
        char* words5[] = {"ccoyyo", "wdhyne", "lqmqhd", "aafljg"};
        Master master5;
        if (Master_init(&master5, "ccoyyo", 10) == 0) {
            findSecretWord(words5, 4, &master5);
            printf("Test 5: %s\\n", master5.guessCount <= 10 ? "PASS" : "FAIL");
        }
    }
    
    return 0;
}
