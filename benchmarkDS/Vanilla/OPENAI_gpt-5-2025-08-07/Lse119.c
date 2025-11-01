#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct RandomStrings {
    char string[64];
    char string2[64];
    char string3[64];
};

struct RandomStrings generateRandomFloatStrings(unsigned int seed) {
    srand(seed);
    float f1 = (float)rand() / (float)RAND_MAX;
    float f2 = (float)rand() / (float)RAND_MAX;
    float f3 = (float)rand() / (float)RAND_MAX;

    char string[64];
    char string2[64];
    char string3[64];

    sprintf(string, "%.6f", f1);
    sprintf(string2, "%.6f", f2);
    sprintf(string3, "%.6f", f3);

    struct RandomStrings result;
    strcpy(result.string, string);
    strcpy(result.string2, string2);
    strcpy(result.string3, string3);
    return result;
}

int main() {
    unsigned int seeds[5] = {1, 2, 3, 4, 5};
    for (int i = 0; i < 5; ++i) {
        struct RandomStrings r = generateRandomFloatStrings(seeds[i]);
        printf("Seed %u: [%s, %s, %s]\n", seeds[i], r.string, r.string2, r.string3);
    }
    return 0;
}