#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

char* Task141_reverse_string(const char* s) {
    if (s == NULL) {
        char* out = (char*)malloc(1);
        if (out != NULL) {
            out[0] = '\0';
        }
        return out;
    }

    size_t len = strlen(s);
    // Check for overflow in allocation size (len + 1)
    if (len == SIZE_MAX) {
        return NULL; // cannot allocate safely
    }

    char* out = (char*)malloc(len + 1);
    if (out == NULL) {
        return NULL;
    }

    for (size_t i = 0; i < len; ++i) {
        out[i] = s[len - 1 - i];
    }
    out[len] = '\0';
    return out;
}

int main(void) {
    const char* tests[5] = {
        "hello",
        "",
        "A",
        "racecar",
        "Hello, 世界"
    };

    for (int i = 0; i < 5; ++i) {
        char* reversed = Task141_reverse_string(tests[i]);
        if (reversed == NULL) {
            printf("Input: \"%s\" -> Reversed: <allocation failed>\n", tests[i] ? tests[i] : "(null)");
        } else {
            printf("Input: \"%s\" -> Reversed: \"%s\"\n", tests[i] ? tests[i] : "(null)", reversed);
            free(reversed);
        }
    }
    return 0;
}