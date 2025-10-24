#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char complement_char(char c) {
    switch (c) {
        case 'A': return 'T';
        case 'T': return 'A';
        case 'C': return 'G';
        case 'G': return 'C';
        default:  return c; // In case of unexpected characters
    }
}

char* dna_strand(const char* dna) {
    size_t n = strlen(dna);
    char* out = (char*)malloc(n + 1);
    if (!out) return NULL;
    for (size_t i = 0; i < n; ++i) {
        out[i] = complement_char(dna[i]);
    }
    out[n] = '\0';
    return out;
}

int main(void) {
    const char* tests[5] = { "ATTGC", "GTAT", "AAAA", "CGCGTTAA", "A" };
    for (int i = 0; i < 5; ++i) {
        char* res = dna_strand(tests[i]);
        if (res) {
            printf("%s\n", res);
            free(res);
        }
    }
    return 0;
}