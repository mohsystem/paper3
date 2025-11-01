#include <stdio.h>
#include <stdlib.h>
#include <time.h>

char* a(float value) {
    char* buf = (char*)malloc(64);
    if (!buf) return NULL;
    snprintf(buf, 64, "%f", value);
    return buf;
}

float randomFloat() {
    return (float)rand() / (float)RAND_MAX;
}

int main() {
    srand((unsigned int)time(NULL));

    // 5 test cases
    for (int i = 1; i <= 5; ++i) {
        float a_val = randomFloat();
        float b_val = randomFloat();
        float c_val = randomFloat();

        char* str_a = a(a_val);
        char* str_b = a(b_val);
        char* str_c = a(c_val);

        printf("Test Case %d:\n", i);
        printf("str_a = %s\n", str_a ? str_a : "(null)");
        printf("str_b = %s\n", str_b ? str_b : "(null)");
        printf("str_c = %s\n", str_c ? str_c : "(null)");
        printf("\n");

        free(str_a);
        free(str_b);
        free(str_c);
    }
    return 0;
}