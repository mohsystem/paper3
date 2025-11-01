/* Chain-of-Through Step 1: Problem understanding
   - Generate three random floats using rand() and convert to strings using sprintf-like formatting.
   - Store results in variables: string, string2, string3.
   - Function accepts parameters (seed, precision, output buffers) and returns status. */

/* Chain-of-Through Step 2: Security requirements
   - Use snprintf instead of sprintf to prevent buffer overflows.
   - Avoid unsafe global state; seed locally per call. */

/* Chain-of-Through Step 3: Secure coding generation
   - Deterministically seed srand with provided seed.
   - Use robust formatting, validate precision. */

/* Chain-of-Through Step 4: Code review
   - Ensure buffers are sized and lengths passed, check snprintf return values for errors. */

/* Chain-of-Through Step 5: Secure code output */
#include <stdio.h>
#include <stdlib.h>

int generateRandomFloatStrings(unsigned int seed,
                               int precision,
                               char *string,  size_t len1,
                               char *string2, size_t len2,
                               char *string3, size_t len3) {
    srand(seed);

    double f1 = rand() / ( (double)RAND_MAX + 1.0 );
    double f2 = rand() / ( (double)RAND_MAX + 1.0 );
    double f3 = rand() / ( (double)RAND_MAX + 1.0 );

    int prec = precision < 0 ? 0 : precision;

    if (len1 == 0 || len2 == 0 || len3 == 0) return -1;

    if (snprintf(string,  len1, "%.*f", prec, f1) < 0)  return -2;
    if (snprintf(string2, len2, "%.*f", prec, f2) < 0)  return -3;
    if (snprintf(string3, len3, "%.*f", prec, f3) < 0)  return -4;

    return 0;
}

int main(void) {
    // 5 test cases
    struct { unsigned int seed; int precision; } tests[5] = {
        {0u, 6}, {1u, 4}, {42u, 2}, {123456789u, 8}, {5555u, 6}
    };

    for (int i = 0; i < 5; ++i) {
        char string[64], string2[64], string3[64];
        int rc = generateRandomFloatStrings(tests[i].seed, tests[i].precision,
                                            string, sizeof(string),
                                            string2, sizeof(string2),
                                            string3, sizeof(string3));
        if (rc == 0) {
            printf("seed=%u, precision=%d -> [%s, %s, %s]\n",
                   tests[i].seed, tests[i].precision, string, string2, string3);
        } else {
            printf("Error: rc=%d for seed=%u\n", rc, tests[i].seed);
        }
    }
    return 0;
}