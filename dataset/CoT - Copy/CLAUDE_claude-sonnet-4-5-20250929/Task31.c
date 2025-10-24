
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* makeComplement(const char* dna) {
    if (dna == NULL) {
        return NULL;
    }
    
    size_t length = strlen(dna);
    char* complement = (char*)malloc((length + 1) * sizeof(char));
    
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
    char* result1 = makeComplement("ATTGC");
    printf("Input: ATTGC, Output: %s\\n", result1);
    free(result1);
    
    // Test case 2
    char* result2 = makeComplement("GTAT");
    printf("Input: GTAT, Output: %s\\n", result2);
    free(result2);
    
    // Test case 3
    char* result3 = makeComplement("AAAA");
    printf("Input: AAAA, Output: %s\\n", result3);
    free(result3);
    
    // Test case 4
    char* result4 = makeComplement("CGCG");
    printf("Input: CGCG, Output: %s\\n", result4);
    free(result4);
    
    // Test case 5
    char* result5 = makeComplement("ATCGATCG");
    printf("Input: ATCGATCG, Output: %s\\n", result5);
    free(result5);
    
    return 0;
}
