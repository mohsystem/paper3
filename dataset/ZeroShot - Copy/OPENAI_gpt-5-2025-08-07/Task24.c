#include <stdio.h>
#include <string.h>

int validate_pin(const char* s) {
    if (s == NULL) return 0;
    size_t len = strlen(s);
    if (len != 4 && len != 6) return 0;
    for (size_t i = 0; i < len; i++) {
        if (s[i] < '0' || s[i] > '9') return 0;
    }
    return 1;
}

int main(void) {
    const char* tests[] = {
        "1234",
        "12345",
        "a234",
        "0000",
        "098765"
    };
    size_t n = sizeof(tests) / sizeof(tests[0]);
    for (size_t i = 0; i < n; i++) {
        printf("%s -> %s\n", tests[i], validate_pin(tests[i]) ? "true" : "false");
    }
    return 0;
}