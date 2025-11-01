/* Chain-of-Through process in comments:
 * 1) Understand: allocate two 1024-byte buffers (buf, buf1), copy up to 100 chars each; also allocate 1024 buffer to read up to 1024 chars.
 * 2) Security: bounds checks, NULL checks, avoid overflows.
 * 3) Secure coding: use size-bounded loops; validate inputs; free memory.
 * 4) Review: loops capped at 100/1024; all allocations checked; frees done.
 * 5) Output: final code below.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char* first;  /* data read into buf */
    size_t first_len;
    char* second; /* data read into buf1 */
    size_t second_len;
} Copy100Out;

typedef struct {
    char* data;
    size_t len;
} Read1024Out;

/* Functions that accept a FILE* stream (can pass stdin) */
Copy100Out allocate_and_copy_100_twice(FILE* in) {
    if (in == NULL) {
        Copy100Out err = {NULL, 0, NULL, 0};
        return err;
    }
    char* buf = (char*)malloc(1024);
    char* buf1 = (char*)malloc(1024);
    if (!buf || !buf1) {
        free(buf); free(buf1);
        Copy100Out err = {NULL, 0, NULL, 0};
        return err;
    }

    size_t n1 = 0;
    int ch;
    while (n1 < 100 && (ch = fgetc(in)) != EOF) {
        buf[n1++] = (char)ch;
    }

    size_t n2 = 0;
    while (n2 < 100 && (ch = fgetc(in)) != EOF) {
        buf1[n2++] = (char)ch;
    }

    char* out1 = (char*)malloc(n1 + 1);
    char* out2 = (char*)malloc(n2 + 1);
    if ((!out1 && n1 > 0) || (!out2 && n2 > 0)) {
        free(buf); free(buf1);
        free(out1); free(out2);
        Copy100Out err = {NULL, 0, NULL, 0};
        return err;
    }
    if (n1 > 0) memcpy(out1, buf, n1);
    if (n2 > 0) memcpy(out2, buf1, n2);
    if (out1) out1[n1] = '\0';
    if (out2) out2[n2] = '\0';

    free(buf);
    free(buf1);

    Copy100Out res = {out1, n1, out2, n2};
    return res;
}

Read1024Out allocate_and_read_up_to_1024(FILE* in) {
    Read1024Out res = {NULL, 0};
    if (in == NULL) return res;

    char* buf = (char*)malloc(1024);
    if (!buf) return res;

    size_t total = 0;
    int ch;
    while (total < 1024 && (ch = fgetc(in)) != EOF) {
        buf[total++] = (char)ch;
    }

    char* out = (char*)malloc(total + 1);
    if (!out && total > 0) {
        free(buf);
        return res;
    }
    if (total > 0) memcpy(out, buf, total);
    if (out) out[total] = '\0';
    free(buf);

    res.data = out;
    res.len = total;
    return res;
}

/* Parameterized versions for testing without FILE*: consume from C-string input */
Copy100Out allocate_and_copy_100_twice_from_str(const char* input) {
    Copy100Out res = {NULL, 0, NULL, 0};
    if (!input) return res;

    char* buf = (char*)malloc(1024);
    char* buf1 = (char*)malloc(1024);
    if (!buf || !buf1) { free(buf); free(buf1); return res; }

    size_t inlen = strlen(input);
    size_t n1 = inlen < 100 ? inlen : 100;
    memcpy(buf, input, n1);

    size_t remaining = inlen > n1 ? inlen - n1 : 0;
    size_t n2 = remaining < 100 ? remaining : 100;
    if (n2 > 0) memcpy(buf1, input + n1, n2);

    char* out1 = (char*)malloc(n1 + 1);
    char* out2 = (char*)malloc(n2 + 1);
    if ((!out1 && n1 > 0) || (!out2 && n2 > 0)) {
        free(buf); free(buf1);
        free(out1); free(out2);
        return res;
    }
    if (n1 > 0) memcpy(out1, buf, n1);
    if (n2 > 0) memcpy(out2, buf1, n2);
    if (out1) out1[n1] = '\0';
    if (out2) out2[n2] = '\0';

    free(buf);
    free(buf1);

    res.first = out1; res.first_len = n1;
    res.second = out2; res.second_len = n2;
    return res;
}

Read1024Out allocate_and_read_up_to_1024_from_str(const char* input) {
    Read1024Out res = {NULL, 0};
    if (!input) return res;

    char* buf = (char*)malloc(1024);
    if (!buf) return res;

    size_t inlen = strlen(input);
    size_t n = inlen < 1024 ? inlen : 1024;
    if (n > 0) memcpy(buf, input, n);

    char* out = (char*)malloc(n + 1);
    if (!out && n > 0) { free(buf); return res; }
    if (n > 0) memcpy(out, buf, n);
    if (out) out[n] = '\0';
    free(buf);

    res.data = out;
    res.len = n;
    return res;
}

/* Helper printing */
static void print_bytes(const char* label, const char* data, size_t len) {
    printf("%s len=%zu data=", label, len);
    if (!data) { printf("(null)\n"); return; }
    for (size_t i = 0; i < len; ++i) {
        char ch = data[i];
        if (ch == '\n') { fputs("\\n", stdout); }
        else if (ch == '\r') { fputs("\\r", stdout); }
        else { fputc(ch, stdout); }
    }
    fputc('\n', stdout);
}

int main(void) {
    /* 5 test cases */
    const char* inputs[5];
    char* a = (char*)malloc(101); memset(a, 'A', 100); a[100] = '\0';
    char* b = (char*)malloc(151); memset(b, 'B', 150); b[150] = '\0';
    char* c = (char*)malloc(221); memset(c, 'C', 220); c[220] = '\0';
    char* d = (char*)malloc(1501); memset(d, 'D', 1500); d[1500] = '\0';
    inputs[0] = "HelloWorld";
    inputs[1] = a;
    inputs[2] = b;
    inputs[3] = c;
    inputs[4] = d;

    for (int i = 0; i < 5; ++i) {
        printf("=== C Test Case %d ===\n", i + 1);
        Copy100Out r = allocate_and_copy_100_twice_from_str(inputs[i]);
        print_bytes("First 100 buf", r.first, r.first_len);
        print_bytes("Second 100 buf1", r.second, r.second_len);
        Read1024Out r2 = allocate_and_read_up_to_1024_from_str(inputs[i]);
        printf("Read up to 1024 len=%zu\n", r2.len);

        free(r.first);
        free(r.second);
        free(r2.data);
    }

    free(a); free(b); free(c); free(d);
    return 0;
}