#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

// Returns a newly allocated C-string containing the complementary DNA strand.
// Mapping: A <-> T, C <-> G
// Returns NULL on invalid input (NULL pointer, empty string, or invalid character).
char* dna_complement(const char* dna) {
    if (dna == NULL) {
        return NULL;
    }
    size_t len = strlen(dna);
    if (len == 0) {
        return NULL;
    }
    if (len > SIZE_MAX - 1) { // defensive check
        return NULL;
    }

    char* out = (char*)malloc(len + 1);
    if (out == NULL) {
        return NULL;
    }

    for (size_t i = 0; i < len; ++i) {
        char ch = dna[i];
        switch (ch) {
            case 'A': out[i] = 'T'; break;
            case 'T': out[i] = 'A'; break;
            case 'C': out[i] = 'G'; break;
            case 'G': out[i] = 'C'; break;
            default:
                free(out);
                return NULL;
        }
    }
    out[len] = '\0';
    return out;
}

static void run_test(int index, const char* input, const char* expected, int expect_valid) {
    char* result = dna_complement(input);
    printf("Test %d: input=\"%s\" -> ", index, input ? input : "(null)");
    if (result == NULL) {
        printf("INVALID");
        if (!expect_valid) {
            printf(" | expected=INVALID | PASS\n");
        } else {
            printf(" | expected=\"%s\" | FAIL\n", expected ? expected : "(null)");
        }
    } else {
        printf("\"%s\"", result);
        if (expect_valid && expected != NULL && strcmp(result, expected) == 0) {
            printf(" | expected=\"%s\" | PASS\n", expected);
        } else if (!expect_valid) {
            printf(" | expected=INVALID | FAIL\n");
        } else {
            printf(" | expected=\"%s\" | FAIL\n", expected ? expected : "(null)");
        }
        free(result);
    }
}

int main(void) {
    run_test(1, "ATTGC", "TAACG", 1);
    run_test(2, "GTAT",  "CATA",  1);
    run_test(3, "AAAA",  "TTTT",  1);
    run_test(4, "CGCG",  "GCGC",  1);
    run_test(5, "ABCD",  NULL,    0);
    return 0;
}