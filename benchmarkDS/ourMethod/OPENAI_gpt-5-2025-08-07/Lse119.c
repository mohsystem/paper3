#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*
 * Generates three random floats in [0,1) using rand() and converts them to strings.
 * The results are written into s1, s2, s3 with the specified precision.
 * Returns 0 on success, non-zero on failure.
 */
int generate_three_random_float_strings(
    int precision,
    char *s1, size_t s1sz,
    char *s2, size_t s2sz,
    char *s3, size_t s3sz
) {
    if (precision < 0 || precision > 9) {
        return 1; // invalid precision
    }
    if (!s1 || !s2 || !s3) {
        return 2; // null buffer
    }
    if (s1sz == 0 || s2sz == 0 || s3sz == 0) {
        return 3; // zero buffer
    }

    // rand() is non-cryptographic; suitable for simple randomized data
    double v1 = (double)rand() / (double)RAND_MAX;
    double v2 = (double)rand() / (double)RAND_MAX;
    double v3 = (double)rand() / (double)RAND_MAX;

    int n = snprintf(s1, s1sz, "%.*f", precision, v1);
    if (n < 0 || (size_t)n >= s1sz) return 4;

    n = snprintf(s2, s2sz, "%.*f", precision, v2);
    if (n < 0 || (size_t)n >= s2sz) return 5;

    n = snprintf(s3, s3sz, "%.*f", precision, v3);
    if (n < 0 || (size_t)n >= s3sz) return 6;

    return 0;
}

int main(void) {
    // Seed rand() once per program execution
    unsigned int seed = (unsigned int)time(NULL);
    srand(seed);

    const size_t BUF_SZ = 64;
    char s1[BUF_SZ], s2[BUF_SZ], s3[BUF_SZ];

    int tests[5] = {0, 2, 4, 6, 9};
    for (int i = 0; i < 5; ++i) {
        int precision = tests[i];
        int rc = generate_three_random_float_strings(precision, s1, sizeof(s1), s2, sizeof(s2), s3, sizeof(s3));
        if (rc != 0) {
            printf("Error generating strings (code %d)\n", rc);
        } else {
            printf("Precision %d -> string: %s, string2: %s, string3: %s\n", precision, s1, s2, s3);
        }
    }

    return 0;
}