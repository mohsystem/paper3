#include <stdio.h>
#include <stdlib.h>
#include <time.h>

char* a(float x) {
    char* buf = (char*)malloc(64);
    if (!buf) return NULL;
    snprintf(buf, 64, "%f", x);
    return buf;
}

void run_test_case() {
    float fa = (float)rand() / (float)RAND_MAX;
    float fb = (float)rand() / (float)RAND_MAX;
    float fc = (float)rand() / (float)RAND_MAX;

    char* str_a = a(fa);
    char* str_b = a(fb);
    char* str_c = a(fc);

    if (str_a && str_b && str_c) {
        printf("str_a=%s, str_b=%s, str_c=%s\n", str_a, str_b, str_c);
    }

    free(str_a);
    free(str_b);
    free(str_c);
}

int main() {
    srand((unsigned)time(NULL));
    for (int i = 0; i < 5; i++) {
        run_test_case();
    }
    return 0;
}