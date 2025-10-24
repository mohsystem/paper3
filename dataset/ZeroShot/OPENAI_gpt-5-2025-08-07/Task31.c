#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char* dna_complement(const char* dna) {
    if (dna == NULL) {
        return NULL;
    }
    size_t n = strlen(dna);
    if (n == 0) {
        return NULL;
    }
    char* out = (char*)malloc(n + 1);
    if (out == NULL) {
        return NULL;
    }
    for (size_t i = 0; i < n; ++i) {
        unsigned char uc = (unsigned char)dna[i];
        char up = (char)toupper(uc);
        switch (up) {
            case 'A': out[i] = 'T'; break;
            case 'T': out[i] = 'A'; break;
            case 'C': out[i] = 'G'; break;
            case 'G': out[i] = 'C'; break;
            default:
                free(out);
                return NULL;
        }
    }
    out[n] = '\0';
    return out;
}

int main(void) {
    const char* tests[] = {
        "ATTGC",   /* TAACG */
        "GTAT",    /* CATA */
        "A",       /* T */
        "aaaa",    /* TTTT */
        "gCatTa"   /* CGTAAT */
    };
    const size_t num_tests = sizeof(tests) / sizeof(tests[0]);
    for (size_t i = 0; i < num_tests; ++i) {
        char* res = dna_complement(tests[i]);
        if (res != NULL) {
            printf("Input: %s -> %s\n", tests[i], res);
            free(res);
        } else {
            printf("Input: %s -> Error: invalid input or memory allocation failed\n", tests[i]);
        }
    }
    return 0;
}