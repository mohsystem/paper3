
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* makeComplement(const char* dna) {
    int len = strlen(dna);
    char* result = (char*)malloc((len + 1) * sizeof(char));
    
    for (int i = 0; i < len; i++) {
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
        }
    }
    result[len] = '\\0';
    return result;
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
