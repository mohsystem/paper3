/* C implementation
   Chain-of-Through process (high-level):
   1) Understand: compute DNA complement.
   2) Security: validate input pointer and characters.
   3) Secure coding: allocate exact size; handle errors; avoid buffer overflow.
   4) Review: free on error paths; return NULL on invalid input.
   5) Output: function returns heap-allocated string; tests included. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* dna_complement(const char* dna) {
    if (dna == NULL) {
        return NULL;
    }
    size_t n = strlen(dna);
    char* out = (char*)malloc(n + 1);
    if (out == NULL) {
        return NULL;
    }
    for (size_t i = 0; i < n; ++i) {
        char c = dna[i];
        switch (c) {
            case 'A': case 'a': out[i] = 'T'; break;
            case 'T': case 't': out[i] = 'A'; break;
            case 'C': case 'c': out[i] = 'G'; break;
            case 'G': case 'g': out[i] = 'C'; break;
            default:
                free(out);
                return NULL;
        }
    }
    out[n] = '\0';
    return out;
}

/* 5 test cases */
int main(void) {
    const char* tests[5] = {
        "ATTGC",
        "GTAT",
        "A",
        "GATTACA",
        "CCGGTTAA"
    };
    for (int i = 0; i < 5; ++i) {
        char* res = dna_complement(tests[i]);
        if (res != NULL) {
            printf("%s -> %s\n", tests[i], res);
            free(res);
        } else {
            printf("Error for \"%s\": invalid input or memory error\n", tests[i]);
        }
    }
    return 0;
}