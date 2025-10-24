#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Given one side of a DNA strand, returns the other complementary side.
 * 'A' and 'T' are complements, 'C' and 'G' are complements.
 * 
 * @param dna The input DNA strand (e.g., "ATTGC").
 * @return A new dynamically allocated string with the complementary DNA strand.
 *         The caller is responsible for freeing this memory. Returns NULL on failure.
 */
char* dna_strand(const char *dna) {
    if (dna == NULL) {
        return NULL;
    }

    size_t len = strlen(dna);
    // Allocate memory for the new string (+1 for the null terminator)
    char *complement = (char*)malloc(len + 1);
    
    // Check if memory allocation was successful
    if (complement == NULL) {
        perror("Failed to allocate memory");
        return NULL; 
    }

    for (size_t i = 0; i < len; ++i) {
        switch (dna[i]) {
            case 'A':
                complement[i] = 'T';
                break;
            case 'T':
                complement[i] = 'A';
                break;
            case 'C':
                complement[i] = 'G';
                break;
            case 'G':
                complement[i] = 'C';
                break;
            default:
                // Assuming valid input. For robustness, could copy the char
                // or handle the error in a specific way.
                complement[i] = dna[i];
                break;
        }
    }
    
    // Add the null terminator to the end of the new string
    complement[len] = '\0';
    
    return complement;
}

int main() {
    const char* testCases[] = {
        "ATTGC",
        "GTAT",
        "AAAA",
        "GATTACA",
        "CGCG"
    };
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);
    
    printf("C Test Cases:\n");
    for (int i = 0; i < numTestCases; ++i) {
        char *result = dna_strand(testCases[i]);
        if (result != NULL) {
            printf("Input: %s -> Output: %s\n", testCases[i], result);
            // Free the memory allocated by dna_strand
            free(result);
        } else {
            printf("Input: %s -> Output: Failed to get complement\n", testCases[i]);
        }
    }

    return 0;
}