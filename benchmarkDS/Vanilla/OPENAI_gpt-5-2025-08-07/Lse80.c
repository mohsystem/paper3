#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    char text[1024];   // buffer size 1024, store up to 1023 chars + null terminator
    int count;         // number of characters stored (not including newline)
    size_t consumed;   // total characters consumed from input (including newline if present)
} ScanResult;

// Reads up to 1023 characters from input until a newline or end-of-input.
// The newline (if present) is consumed but not stored.
// Returns ScanResult containing the stored text, count, and consumed.
ScanResult scan(const char* input) {
    ScanResult r;
    r.count = 0;
    r.consumed = 0;
    r.text[0] = '\0';
    if (input == NULL) {
        return r;
    }
    size_t i = 0;
    while (input[i] != '\0') {
        char ch = input[i];
        if (ch == '\n') {
            r.consumed += 1;
            break;
        }
        if (r.count < 1023) {
            r.text[r.count++] = ch;
        }
        r.consumed += 1;
        i += 1;
    }
    r.text[r.count] = '\0';
    return r;
}

int main() {
    const char* test1 = "hello\nworld"; // normal line with newline
    const char* test2 = "\nstart";      // newline first

    // Build test3: 1100 'a's followed by newline and "NEXT"
    size_t len3 = 1100 + 1 + 4;
    char* test3 = (char*)malloc(len3 + 1);
    for (int i = 0; i < 1100; ++i) test3[i] = 'a';
    test3[1100] = '\n';
    memcpy(test3 + 1101, "NEXT", 4);
    test3[len3] = '\0';

    const char* test4 = "no newline here"; // no newline
    const char* test5 = "";                 // empty input

    const char* tests[5] = { test1, test2, test3, test4, test5 };

    for (int i = 0; i < 5; ++i) {
        ScanResult r = scan(tests[i]);
        printf("Test %d: text=\"%s\", count=%d, consumed=%zu\n", i+1, r.text, r.count, r.consumed);
    }

    free(test3);
    return 0;
}