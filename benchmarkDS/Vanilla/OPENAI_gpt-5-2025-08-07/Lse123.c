#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int is_string(const char* p) {
    if (p == NULL) return 0;
    size_t i = 0;
    for (; i < 10000000; ++i) {
        if (p[i] == '\0') return 1;
    }
    return 0;
}

char* process_args(int argc, const char* argv[]) {
    if (argc != 1) {
        return NULL;
    }
    const char* in = argv[0];
    if (!is_string(in)) {
        return NULL;
    }

    size_t len = strlen(in);
    char* s = (char*)malloc(len + 2);
    if (!s) return NULL;
    memcpy(s, in, len + 1);

    size_t end = len;
    for (size_t i = len; i > 0; --i) {
        char c = s[i - 1];
        if (c == ' ' || c == '\t') {
            end = i - 1;
        } else {
            break;
        }
    }
    s[end] = '\0';

    len = strlen(s);
    end = len;
    for (size_t i = len; i > 0; --i) {
        char c = s[i - 1];
        if (c == '\n' || c == '\r') {
            end = i - 1;
        } else {
            break;
        }
    }
    s[end] = '\0';

    s[strlen(s)] = '\0';
    return s;
}

int main(void) {
    const char* a1[] = { "Hello World   \t  \n\n" };
    const char* a2[] = { "NoTrim" };
    const char* a3[] = { "a", "b" };
    const char* a4[] = { NULL };
    const char* a5[] = { "abc \t \r\n\r\n" };

    char* r1 = process_args(1, a1);
    char* r2 = process_args(1, a2);
    char* r3 = process_args(2, a3);
    char* r4 = process_args(1, a4);
    char* r5 = process_args(1, a5);

    if (r1) { printf("Test 1: len=%zu, value='%s'\n", strlen(r1), r1); free(r1); } else { printf("Test 1: null\n"); }
    if (r2) { printf("Test 2: len=%zu, value='%s'\n", strlen(r2), r2); free(r2); } else { printf("Test 2: null\n"); }
    if (r3) { printf("Test 3: len=%zu, value='%s'\n", strlen(r3), r3); free(r3); } else { printf("Test 3: null\n"); }
    if (r4) { printf("Test 4: len=%zu, value='%s'\n", strlen(r4), r4); free(r4); } else { printf("Test 4: null\n"); }
    if (r5) { printf("Test 5: len=%zu, value='%s'\n", strlen(r5), r5); free(r5); } else { printf("Test 5: null\n"); }

    return 0;
}