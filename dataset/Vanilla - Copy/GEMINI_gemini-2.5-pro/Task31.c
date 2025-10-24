#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* dnaStrand(const char* dna) {
    if (dna == NULL) {
        return NULL;
    }
    
    size_t len = strlen(dna);
    char* result = (char*)malloc(len + 1); // +1 for the null terminator
    
    if (result == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    for (size_t i = 0; i < len; i++) {
        switch (dna[i]) {
            case 'A':
                result[i] = 'T';
                break;
            case 'T':
                result[i] = 'A';
                break;
            case 'C':
                result[i] = 'G';
                break;
            case 'G':
                result[i] = 'C';
                break;
            default:
                result[i] = dna[i];
                break;
        }
    }
    result[len] = '\0'; // Null-terminate the new string
    
    return result;
}

int main() {
    const char* testCases[] = {"ATTGC", "GTAT", "AAAA", "CGCG", "TAGCTAGC"};
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < numTestCases; i++) {
        const char* testCase = testCases[i];
        char* complementaryStrand = dnaStrand(testCase);
        if (complementaryStrand != NULL) {
            printf("Input: %s --> Output: %s\n", testCase, complementaryStrand);
            free(complementaryStrand); // Free the dynamically allocated memory
        }
    }
    
    return 0;
}