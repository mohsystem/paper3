#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int generate_random_float_strings(int precision,
                                  char *string, size_t stringSize,
                                  char *string2, size_t string2Size,
                                  char *string3, size_t string3Size) {
    if (!string || !string2 || !string3 || stringSize == 0 || string2Size == 0 || string3Size == 0) {
        return -1;
    }

    if (precision < 0) precision = 0;
    if (precision > 9) precision = 9;

    float f1 = (float)rand() / (float)RAND_MAX;
    float f2 = (float)rand() / (float)RAND_MAX;
    float f3 = (float)rand() / (float)RAND_MAX;

    int n1 = snprintf(string, stringSize, "%.*f", precision, f1);
    int n2 = snprintf(string2, string2Size, "%.*f", precision, f2);
    int n3 = snprintf(string3, string3Size, "%.*f", precision, f3);

    if (n1 < 0 || n1 >= (int)stringSize) return -2;
    if (n2 < 0 || n2 >= (int)string2Size) return -3;
    if (n3 < 0 || n3 >= (int)string3Size) return -4;

    return 0;
}

int main(void) {
    srand((unsigned)time(NULL));

    int tests[5] = {2, 3, 0, 5, 7};
    for (int i = 0; i < 5; ++i) {
        char string[64];
        char string2[64];
        char string3[64];

        int rc = generate_random_float_strings(tests[i], string, sizeof(string),
                                               string2, sizeof(string2),
                                               string3, sizeof(string3));
        if (rc == 0) {
            printf("Precision %d -> [%s, %s, %s]\n", tests[i], string, string2, string3);
        } else {
            printf("Error generating strings for precision %d (code %d)\n", tests[i], rc);
        }
    }

    return 0;
}