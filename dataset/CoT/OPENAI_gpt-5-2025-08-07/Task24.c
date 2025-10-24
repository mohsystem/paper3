#include <stdio.h>
#include <stdbool.h>
#include <string.h>

bool isValidPIN(const char* pin) {
    if (pin == NULL) return false;
    size_t len = strlen(pin);
    if (len != 4 && len != 6) return false;
    for (size_t i = 0; i < len; ++i) {
        char ch = pin[i];
        if (ch < '0' || ch > '9') return false; // ASCII digit check only
    }
    return true;
}

int main(void) {
    const char* tests[5] = {"1234", "12345", "a234", "098765", ""};
    for (int i = 0; i < 5; ++i) {
        printf("%s -> %s\n", tests[i], isValidPIN(tests[i]) ? "true" : "false");
    }
    return 0;
}