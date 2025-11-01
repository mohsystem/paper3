#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

enum { BUFFER_SIZE = 1024 }; // store up to 1023 chars + null terminator

// Reads from provided FILE* until newline or EOF, consumes newline but does not store it.
// Writes up to (bufsize-1) characters into buf, NUL-terminates buf. Returns number of characters stored.
int scan(FILE *in, char *buf, size_t bufsize) {
    if (in == NULL || buf == NULL || bufsize == 0) {
        return 0;
    }
    size_t maxstore = (bufsize > 0 ? bufsize - 1 : 0);
    size_t stored = 0;
    int c;
    while ((c = fgetc(in)) != EOF) {
        if (c == '\n') {
            break; // consume newline
        }
        if (stored < maxstore) {
            buf[stored++] = (char)c;
        } else {
            // discard until newline/EOF
        }
    }
    if (stored >= maxstore) {
        // drain until newline/EOF
        while ((c = fgetc(in)) != EOF) {
            if (c == '\n') break;
        }
    }
    if (bufsize > 0) {
        buf[stored] = '\0';
    }
    return (int)stored;
}

int main(void) {
    char out[BUFFER_SIZE];

    // Test 1: "Hello\n"
    const char *t1 = "Hello\n";
    FILE *f1 = fmemopen((void*)t1, strlen(t1), "r");
    int c1 = scan(f1, out, sizeof(out));
    fclose(f1);
    printf("T1 count=%d data=\"%s\"\n", c1, out);

    // Test 2: "World" (no newline)
    const char *t2 = "World";
    FILE *f2 = fmemopen((void*)t2, strlen(t2), "r");
    int c2 = scan(f2, out, sizeof(out));
    fclose(f2);
    printf("T2 count=%d data=\"%s\"\n", c2, out);

    // Test 3: empty line "\n"
    const char *t3 = "\n";
    FILE *f3 = fmemopen((void*)t3, strlen(t3), "r");
    int c3 = scan(f3, out, sizeof(out));
    fclose(f3);
    printf("T3 count=%d data=\"%s\"\n", c3, out);

    // Test 4: exactly 1023 chars plus newline
    char *t4 = (char*)malloc(1023 + 1 + 1);
    memset(t4, 'A', 1023);
    t4[1023] = '\n';
    t4[1024] = '\0';
    FILE *f4 = fmemopen((void*)t4, 1024, "r");
    int c4 = scan(f4, out, sizeof(out));
    fclose(f4);
    printf("T4 count=%d data length=%zu first5=\"%.5s\" last5=\"%.*s\"\n",
           c4, strlen(out), out, (int)(strlen(out) >= 5 ? 5 : (int)strlen(out)),
           out + (strlen(out) >= 5 ? strlen(out) - 5 : 0));
    free(t4);

    // Test 5: 1500 chars plus newline (truncation expected)
    size_t longlen = 1500;
    char *t5 = (char*)malloc(longlen + 2);
    memset(t5, 'B', longlen);
    t5[longlen] = '\n';
    t5[longlen + 1] = '\0';
    FILE *f5 = fmemopen((void*)t5, longlen + 1, "r");
    int c5 = scan(f5, out, sizeof(out));
    fclose(f5);
    size_t outlen = strlen(out);
    printf("T5 count=%d data length=%zu first5=\"%.5s\" last5=\"%.*s\"\n",
           c5, outlen, out, (int)(outlen >= 5 ? 5 : (int)outlen),
           out + (outlen >= 5 ? outlen - 5 : 0));
    free(t5);

    return 0;
}