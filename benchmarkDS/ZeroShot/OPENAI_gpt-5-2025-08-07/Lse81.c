#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Allocates a buffer of 1024 bytes, zero-initialized */
unsigned char* allocate_buffer(void) {
    unsigned char* p = (unsigned char*)calloc(1024u, 1u);
    return p; /* may be NULL if allocation fails */
}

/* Reads up to 10 bytes from the provided FILE* into buf.
   Returns the number of bytes read. */
size_t read_ten_chars_to_buffer(FILE* in, unsigned char* buf, size_t buf_len) {
    if (in == NULL || buf == NULL) {
        return 0;
    }
    if (buf_len < 10u) {
        return 0;
    }

    size_t count = 0;
    int ch;
    while (count < 10u && (ch = fgetc(in)) != EOF) {
        buf[count++] = (unsigned char)(ch & 0xFF);
    }
    return count;
}

static void run_test_case(int idx, const char* input) {
    FILE* f = tmpfile();
    if (f == NULL) {
        printf("Test %d: tmpfile() failed\n", idx);
        return;
    }

    if (input != NULL && input[0] != '\0') {
        size_t len = strlen(input);
        if (fwrite(input, 1u, len, f) != len) {
            printf("Test %d: fwrite failed\n", idx);
            fclose(f);
            return;
        }
    }
    fflush(f);
    rewind(f);

    unsigned char* buf = allocate_buffer();
    if (buf == NULL) {
        printf("Test %d: allocation failed\n", idx);
        fclose(f);
        return;
    }

    size_t readCount = read_ten_chars_to_buffer(f, buf, 1024u);
    printf("Test %d: readCount=%zu, data=\"", idx, readCount);
    /* Print as a safe string; non-printables may appear as-is */
    for (size_t i = 0; i < readCount; ++i) {
        unsigned char c = buf[i];
        if (c == '"' || c == '\\') {
            putchar('\\');
            putchar((int)c);
        } else if (c >= 32 && c <= 126) {
            putchar((int)c);
        } else {
            /* Print hex escape for non-printables */
            printf("\\x%02X", (unsigned int)c);
        }
    }
    printf("\"\n");

    free(buf);
    fclose(f);
}

int main(void) {
    run_test_case(1, "HelloWorld!");   /* >10 */
    run_test_case(2, "Short");         /* <10 */
    run_test_case(3, "1234567890ABC"); /* >10 */
    run_test_case(4, "");              /* empty */
    run_test_case(5, "ExactTen!!");    /* exactly 10 */
    return 0;
}