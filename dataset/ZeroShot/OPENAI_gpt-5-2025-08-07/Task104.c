#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#define BUFFER_SIZE 16

static size_t safe_strnlen_local(const char* s, size_t maxlen) {
    if (s == NULL) {
        return 0;
    }
    size_t i = 0;
    for (; i < maxlen; ++i) {
        if (s[i] == '\0') {
            break;
        }
    }
    return i;
}

char* handle_input(const char* input) {
    if (input == NULL) {
        input = "";
    }
    char* buf = (char*)malloc(BUFFER_SIZE);
    if (buf == NULL) {
        return NULL;
    }
    size_t toCopy = safe_strnlen_local(input, BUFFER_SIZE - 1);
    if (toCopy > 0) {
        memcpy(buf, input, toCopy);
    }
    buf[toCopy] = '\0';
    return buf;
}

int main(void) {
    const char* tests[5] = {
        "Hello",
        "1234567890ABCDEF",
        "",
        "SafeBuffer!",
        "This is a very long string that exceeds the buffer size!"
    };
    for (int i = 0; i < 5; ++i) {
        char* res = handle_input(tests[i]);
        if (res == NULL) {
            printf("Test %d: Allocation failed\n", i + 1);
        } else {
            printf("Test %d: %s\n", i + 1, res);
            free(res);
        }
    }
    return 0;
}