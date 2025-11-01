#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { MAX_BUFFER_SIZE = 1024 }; // bytes; will store at most 1023 characters

// Reads from the provided FILE* up to (bufsize-1) characters or until a newline.
// The newline (if encountered during reading) is consumed but not stored.
// Returns the number of characters stored in 'buf' (excluding newline). Ensures null termination.
size_t scan(FILE *in, char *buf, size_t bufsize) {
    if (in == NULL || buf == NULL || bufsize == 0) {
        return 0;
    }
    size_t limit = (bufsize > 0) ? (bufsize - 1) : 0;
    size_t count = 0;

    int ch;
    while (count < limit) {
        ch = fgetc(in);
        if (ch == EOF) {
            break;
        }
        if (ch == '\n') {
            break; // consume newline; do not store
        }
        buf[count++] = (char)ch;
    }

    // Do not attempt to read further if limit reached; leave any newline for the next consumer
    buf[count] = '\0';
    return count;
}

// Utility to make control characters visible in output
static void print_visible(const char *s) {
    for (const unsigned char *p = (const unsigned char *)s; *p; ++p) {
        if (*p == '\n') fputs("\\n", stdout);
        else if (*p == '\r') fputs("\\r", stdout);
        else if (*p == '\t') fputs("\\t", stdout);
        else if (*p < 0x20 || *p == 0x7F) {
            char buf[5];
            snprintf(buf, sizeof(buf), "\\x%02X", (unsigned)*p);
            fputs(buf, stdout);
        } else fputc(*p, stdout);
    }
}

// Create an input FILE* from a memory buffer safely (copying to a mutable buffer for fmemopen)
static FILE* mem_input(const char *data, size_t len) {
    if (data == NULL) return NULL;
    void *copy = malloc(len);
    if (!copy) return NULL;
    memcpy(copy, data, len);
    FILE *f = fmemopen(copy, len, "r");
    if (!f) {
        free(copy);
        return NULL;
    }
    // The buffer will be freed when the stream is closed (as per GNU fmemopen); if not, we'll free manually.
    return f;
}

int main(void) {
    // Test case 1: Simple line with newline
    {
        const char *t1 = "Hello\nWorld";
        FILE *f = mem_input(t1, strlen(t1));
        char buf[MAX_BUFFER_SIZE];
        size_t n = scan(f, buf, sizeof(buf));
        printf("T1 count=%lu text=\"", (unsigned long)n);
        print_visible(buf);
        printf("\"\n");
        fclose(f); // releases the associated buffer in GNU fmemopen
    }

    // Test case 2: Empty line (just newline)
    {
        const char *t2 = "\n";
        FILE *f = mem_input(t2, strlen(t2));
        char buf[MAX_BUFFER_SIZE];
        size_t n = scan(f, buf, sizeof(buf));
        printf("T2 count=%lu text=\"", (unsigned long)n);
        print_visible(buf);
        printf("\"\n");
        fclose(f);
    }

    // Test case 3: Longer than buffer (1200 'A's and newline)
    {
        size_t len = 1201; // 1200 'A' + '\n'
        char *t3 = (char*)malloc(len + 1);
        if (!t3) return 1;
        memset(t3, 'A', 1200);
        t3[1200] = '\n';
        t3[1201] = '\0';
        FILE *f = mem_input(t3, strlen(t3));
        char buf[MAX_BUFFER_SIZE];
        size_t n = scan(f, buf, sizeof(buf));
        printf("T3 count=%lu (expected 1023) text_len=%lu\n", (unsigned long)n, (unsigned long)strlen(buf));
        fclose(f);
        free(t3);
    }

    // Test case 4: No newline before EOF
    {
        const char *t4 = "NoNewlineEOF";
        FILE *f = mem_input(t4, strlen(t4));
        char buf[MAX_BUFFER_SIZE];
        size_t n = scan(f, buf, sizeof(buf));
        printf("T4 count=%lu text=\"", (unsigned long)n);
        print_visible(buf);
        printf("\"\n");
        fclose(f);
    }

    // Test case 5: CRLF before next data
    {
        const char *t5 = "CR\rLF\r\nNext";
        FILE *f = mem_input(t5, strlen(t5));
        char buf[MAX_BUFFER_SIZE];
        size_t n = scan(f, buf, sizeof(buf));
        printf("T5 count=%lu text=\"", (unsigned long)n);
        print_visible(buf);
        printf("\"\n");
        fclose(f);
    }

    return 0;
}