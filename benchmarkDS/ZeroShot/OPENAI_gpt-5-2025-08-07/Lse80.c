#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Secure scan function that simulates reading from standard input by taking the
 * input as a C-string parameter. It reads up to 1023 characters or until a newline,
 * consumes the newline if present, stores the data in buf, null-terminates it,
 * and returns the number of characters stored.
 */
int scan_from_cstr(const char *input, char *buf, size_t bufsize) {
    if (buf == NULL || bufsize == 0) {
        return 0;
    }
    size_t max = (bufsize - 1);
    if (max > 1023) {
        max = 1023; // enforce 1023 character limit
    }

    size_t count = 0;
    if (input == NULL) {
        buf[0] = '\0';
        return 0;
    }

    const unsigned char *p = (const unsigned char *)input;
    while (*p != '\0') {
        unsigned char ch = *p++;
        if (ch == '\n') {
            break; // consume newline, do not store
        }
        if (count < max) {
            buf[count++] = (char)ch;
        } else {
            // buffer full, discard until newline or end
            while (*p != '\0' && *p != '\n') {
                p++;
            }
            if (*p == '\n') {
                p++; // consume newline
            }
            break;
        }
    }
    buf[count] = '\0';
    return (int)count;
}

static void run_test(const char *label, const char *input) {
    char buf[1024];
    int n = scan_from_cstr(input, buf, sizeof(buf));
    printf("%s -> count=%d, out=\"%.*s\"\n", label, n, n, buf);
}

int main(void) {
    run_test("Test1", "Hello, World!\nTrailing");
    run_test("Test2", "");
    run_test("Test3", "NoNewline");

    // Build long input: 1500 'A's, then newline and some extra chars
    size_t long_len = 1500 + 1 + 4; // 1500 'A' + '\n' + "BBBB"
    char *long_input = (char *)malloc(long_len + 1);
    if (!long_input) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    memset(long_input, 'A', 1500);
    long_input[1500] = '\n';
    memcpy(long_input + 1500 + 1, "BBBB", 4);
    long_input[long_len] = '\0';
    run_test("Test4", long_input);
    free(long_input);

    run_test("Test5", "Line1\nLine2\n");
    return 0;
}