#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <stddef.h>

bool validate_pin(const char *s) {
    if (s == NULL) {
        return false;
    }

    // Determine length safely up to 7 (since valid lengths are 4 or 6)
    size_t len = 0;
    while (len < 7 && s[len] != '\0') {
        len++;
    }

    if (!((len == 4 || len == 6) && s[len] == '\0')) {
        return false;
    }

    for (size_t i = 0; i < len; i++) {
        unsigned char ch = (unsigned char)s[i];
        if (!isdigit(ch)) {
            return false;
        }
    }
    return true;
}

static const char* bool_to_str(bool v) {
    return v ? "true" : "false";
}

int main(void) {
    // 5 test cases
    const char *tests[] = {"1234", "12345", "a234", "000000", ""};
    const size_t count = sizeof(tests) / sizeof(tests[0]);

    for (size_t i = 0; i < count; i++) {
        bool res = validate_pin(tests[i]);
        printf("Input: \"%s\" -> %s\n", tests[i], bool_to_str(res));
    }
    return 0;
}