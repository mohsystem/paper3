#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Finds the complementary strand of a DNA sequence.
 * 'A' is complement of 'T', 'C' is complement of 'G'.
 * 
 * @param dna A constant character pointer to the input DNA string.
 * @return A new dynamically allocated string with the complement.
 *         The caller is responsible for freeing this memory.
 *         Returns NULL if input is NULL or if memory allocation fails.
 */
char* dnaStrand(const char* dna) {
    if (dna == NULL) {
        return NULL; // Sanity check for null input
    }

    size_t len = strlen(dna);
    // Allocate memory for the new string (+1 for the null terminator)
    char* complement = (char*)malloc(len + 1);

    // Secure code: Always check the return value of malloc
    if (complement == NULL) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
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
                // As per prompt, input is always valid.
                // A robust solution might clean up and return NULL here.
                complement[i] = dna[i];
                break;
        }
    }
    complement[len] = '\0'; // Ensure the new string is null-terminated

    return complement;
}

// Helper function to run a test case and handle memory management
void run_test_case(const char* input) {
    char* result = dnaStrand(input);
    if (result != NULL) {
        printf("\"%s\" --> \"%s\"\n", input, result);
        free(result); // Crucial for preventing memory leaks
    }
}

int main() {
    run_test_case("ATTGC");
    run_test_case("GTAT");
    run_test_case("AAAA");
    run_test_case("CGCG");
    run_test_case("TAACG");

    return 0;
}