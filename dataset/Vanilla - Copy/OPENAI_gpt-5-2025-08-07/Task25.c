#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char** number(const char* lines[], int n) {
    if (n <= 0) return NULL;
    char** out = (char**)malloc(sizeof(char*) * n);
    if (!out) return NULL;
    for (int i = 0; i < n; ++i) {
        int num_len = snprintf(NULL, 0, "%d", i + 1);
        const char* s = lines && lines[i] ? lines[i] : "";
        size_t s_len = strlen(s);
        size_t total = (size_t)num_len + 2 + s_len + 1; // "n: " + s + '\0'
        char* buf = (char*)malloc(total);
        if (!buf) {
            for (int j = 0; j < i; ++j) free(out[j]);
            free(out);
            return NULL;
        }
        snprintf(buf, total, "%d: %s", i + 1, s);
        out[i] = buf;
    }
    return out;
}

void print_array(char** arr, int n) {
    printf("[");
    for (int i = 0; i < n; ++i) {
        if (i) printf(", ");
        printf("\"%s\"", arr[i]);
    }
    printf("]\n");
}

void free_array(char** arr, int n) {
    if (!arr) return;
    for (int i = 0; i < n; ++i) free(arr[i]);
    free(arr);
}

int main() {
    const char** t1_arr = NULL; int t1_n = 0;
    const char* t2_arr[] = {"a", "b", "c"}; int t2_n = 3;
    const char* t3_arr[] = {"", "hello"}; int t3_n = 2;
    const char* t4_arr[] = {"foo bar", "baz"}; int t4_n = 2;
    const char* t5_arr[] = {"x"}; int t5_n = 1;

    char** r1 = number(t1_arr, t1_n);
    char** r2 = number(t2_arr, t2_n);
    char** r3 = number(t3_arr, t3_n);
    char** r4 = number(t4_arr, t4_n);
    char** r5 = number(t5_arr, t5_n);

    print_array(r1, t1_n);
    print_array(r2, t2_n);
    print_array(r3, t3_n);
    print_array(r4, t4_n);
    print_array(r5, t5_n);

    free_array(r1, t1_n);
    free_array(r2, t2_n);
    free_array(r3, t3_n);
    free_array(r4, t4_n);
    free_array(r5, t5_n);
    return 0;
}