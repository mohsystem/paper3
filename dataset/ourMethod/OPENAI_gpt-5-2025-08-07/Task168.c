#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

static int is_lower_ascii(char c) {
    return (c >= 'a' && c <= 'z');
}

static char* tweak_letters(const char* s, const int* shifts, size_t len) {
    if (s == NULL || shifts == NULL) {
        return NULL;
    }
    size_t slen = strlen(s);
    if (slen != len) {
        return NULL;
    }
    char* out = (char*)malloc(len + 1);
    if (!out) {
        return NULL;
    }
    for (size_t i = 0; i < len; ++i) {
        char ch = s[i];
        if (!is_lower_ascii(ch)) {
            free(out);
            return NULL;
        }
        int delta = shifts[i] % 26;
        if (delta < 0) delta += 26;
        int pos = ch - 'a';
        out[i] = (char)('a' + (pos + delta) % 26);
    }
    out[len] = '\0';
    return out;
}

static void run_test(const char* input, const int* shifts, size_t len, const char* expected) {
    char* result = tweak_letters(input, shifts, len);
    if (result == NULL) {
        printf("Error: invalid input or memory allocation failed\n");
        return;
    }
    printf("tweak_letters(\"%s\", [", input);
    for (size_t i = 0; i < len; ++i) {
        printf("%d", shifts[i]);
        if (i + 1 < len) printf(", ");
    }
    printf("]) => \"%s\"", result);
    if (expected) {
        printf(" | expected: \"%s\"", expected);
    }
    printf("\n");
    free(result);
}

int main(void) {
    int shifts1[] = {0, 1, -1, 0, -1};
    run_test("apple", shifts1, 5, "aqold");

    int shifts2[] = {0, 0, 0, -1};
    run_test("many", shifts2, 4, "manx");

    int shifts3[] = {1, 1, 1, 1, 1};
    run_test("rhino", shifts3, 5, "sijop");

    int shifts4[] = {1, 1, 1};
    run_test("zzz", shifts4, 3, "aaa");

    int shifts5[] = {-1, 0, 1};
    run_test("abc", shifts5, 3, "zbd");

    return 0;
}