#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

static void seed_rng_once(void) {
    static int seeded = 0;
    if (!seeded) {
        unsigned int seed = (unsigned int)time(NULL);
        seed ^= (unsigned int)(uintptr_t)&seed;
        srand(seed);
        seeded = 1;
    }
}

char* generate_and_write(const char* filePath) {
    if (filePath == NULL) {
        return NULL;
    }

    seed_rng_once();

    float f1 = (float)rand() / (float)RAND_MAX;
    float f2 = (float)rand() / (float)RAND_MAX;
    float f3 = (float)rand() / (float)RAND_MAX;

    int needed = snprintf(NULL, 0, "%f,%f,%f", f1, f2, f3);
    if (needed < 0) {
        return NULL;
    }

    char* concatenated = (char*)malloc((size_t)needed + 1);
    if (!concatenated) {
        return NULL;
    }

    int written = snprintf(concatenated, (size_t)needed + 1, "%f,%f,%f", f1, f2, f3);
    if (written < 0 || written > needed) {
        free(concatenated);
        return NULL;
    }

    FILE* f = fopen(filePath, "w");
    if (!f) {
        free(concatenated);
        return NULL;
    }

    size_t len = strlen(concatenated);
    size_t w = fwrite(concatenated, 1, len, f);
    if (w != len) {
        fclose(f);
        free(concatenated);
        return NULL;
    }

    if (fclose(f) != 0) {
        free(concatenated);
        return NULL;
    }

    return concatenated;
}

int main(void) {
    const char* files[5] = {
        "c_out_1.txt",
        "c_out_2.txt",
        "c_out_3.txt",
        "c_out_4.txt",
        "c_out_5.txt"
    };

    for (int i = 0; i < 5; ++i) {
        char* result = generate_and_write(files[i]);
        if (result) {
            printf("Wrote to %s: %s\n", files[i], result);
            free(result);
        } else {
            printf("Failed for %s\n", files[i]);
        }
    }

    return 0;
}