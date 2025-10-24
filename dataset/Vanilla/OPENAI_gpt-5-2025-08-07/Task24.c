#include <stdio.h>
#include <string.h>
#include <ctype.h>

int validatePin(const char* pin) {
    if (pin == NULL) return 0;
    size_t n = strlen(pin);
    if (n != 4 && n != 6) return 0;
    for (size_t i = 0; i < n; ++i) {
        if (!isdigit((unsigned char)pin[i])) return 0;
    }
    return 1;
}

int main() {
    const char* tests[] = {"1234", "12345", "a234", "0000", "098765"};
    size_t count = sizeof(tests) / sizeof(tests[0]);
    for (size_t i = 0; i < count; ++i) {
        printf("%s -> %s\n", tests[i], validatePin(tests[i]) ? "true" : "false");
    }
    return 0;
}