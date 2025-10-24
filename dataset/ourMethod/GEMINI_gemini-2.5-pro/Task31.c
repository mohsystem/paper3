#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to find the complementary DNA strand.
// The caller is responsible for freeing the returned string.
char* dna_strand(const char* dna) {
    // Validate input pointer
    if (dna == NULL) {
        return NULL;
    }

    size_t len = strlen(dna);
    // Allocate memory for the complementary strand (+1 for null terminator)
    char* complement = (char*)malloc(len + 1);
    
    // Check if malloc was successful
    if (complement == NULL) {
        return NULL; // Return NULL on memory allocation failure
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
            // According to the prompt, the input will only contain 'A', 'T', 'C', 'G'.
            // If invalid characters were possible, we should handle them,
            // for example by freeing memory and returning NULL.
        }
    }
    complement[len] = '\0'; // Null-terminate the string
    return complement;
}

// Main function with test cases
int main() {
    const char* inputs[] = {"ATTGC", "GTAT", "AAAA", "CGCG", "GATTACA"};
    const char* expected_outputs[] = {"TAACG", "CATA", "TTTT", "GCGC", "CTAATGT"};
    int num_test_cases = sizeof(inputs) / sizeof(inputs[0]);

    for (int i = 0; i < num_test_cases; ++i) {
        const char* input = inputs[i];
        const char* expected = expected_outputs[i];
        char* actual = dna_strand(input);

        printf("Test Case %d:\n", i + 1);
        printf("Input:    \"%s\"\n", input);
        printf("Expected: \"%s\"\n", expected);

        if (actual != NULL) {
            printf("Actual:   \"%s\"\n", actual);
            if (strcmp(actual, expected) == 0) {
                printf("Result:   PASS\n");
            } else {
                printf("Result:   FAIL\n");
            }
            free(actual); // Free the allocated memory
        } else {
            printf("Actual:   (NULL) - function failed\n");
            printf("Result:   FAIL\n");
        }
        printf("---------------------\n");
    }

    return 0;
}