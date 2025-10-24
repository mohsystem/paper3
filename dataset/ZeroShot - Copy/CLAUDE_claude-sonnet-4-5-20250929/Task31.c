
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* getDNAComplement(const char* dna) {
    if (dna == NULL || strlen(dna) == 0) {
        char* empty = (char*)malloc(1);
        empty[0] = '\\0';
        return empty;
    }
    
    size_t length = strlen(dna);
    char* complement = (char*)malloc(length + 1);
    
    if (complement == NULL) {
        return NULL;
    }
    
    for (size_t i = 0; i < length; i++) {
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
                free(complement);
                return NULL;
        }
    }
    
    complement[length] = '\\0';
    return complement;
}

int main() {
    // Test case 1
    printf("Input: ATTGC\\n");
    char* result1 = getDNAComplement("ATTGC");
    printf("Output: %s\\n", result1);
    printf("Expected: TAACG\\n\\n");
    free(result1);
    
    // Test case 2
    printf("Input: GTAT\\n");
    char* result2 = getDNAComplement("GTAT");
    printf("Output: %s\\n", result2);
    printf("Expected: CATA\\n\\n");
    free(result2);
    
    // Test case 3
    printf("Input: AAAA\\n");
    char* result3 = getDNAComplement("AAAA");
    printf("Output: %s\\n", result3);
    printf("Expected: TTTT\\n\\n");
    free(result3);
    
    // Test case 4
    printf("Input: CGCG\\n");
    char* result4 = getDNAComplement("CGCG");
    printf("Output: %s\\n", result4);
    printf("Expected: GCGC\\n\\n");
    free(result4);
    
    // Test case 5
    printf("Input: ATCGATCG\\n");
    char* result5 = getDNAComplement("ATCGATCG");
    printf("Output: %s\\n", result5);
    printf("Expected: TAGCTAGC\\n\\n");
    free(result5);
    
    return 0;
}
