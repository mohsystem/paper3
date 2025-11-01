/* Chain-of-Through process in code generation:
 * 1) Problem understanding: Implement a 1024-byte allocation and a function that reads 10 characters from stdin into a buffer.
 * 2) Security requirements: Validate inputs, avoid buffer overflows, handle EOF, and avoid unsafe functions.
 * 3) Secure coding generation: Use checked sizes and fread with explicit limits; verify pointers.
 * 4) Code review: Ensure all paths validate sizes and pointers; no unchecked writes.
 * 5) Secure code output: Final code mitigates issues and uses tmpfile-based tests to avoid stdin blocking.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char* allocate_buffer(void) {
    unsigned char* p = (unsigned char*)calloc(1024, 1);
    return p;
}

size_t read_ten_from_stream(unsigned char* buf, size_t bufsize, FILE* in) {
    if (buf == NULL || in == NULL || bufsize < 10) {
        return 0;
    }
    return fread(buf, 1, 10, in); /* fread reads up to 10 bytes safely */
}

size_t read_ten_from_stdin(unsigned char* buf, size_t bufsize) {
    return read_ten_from_stream(buf, bufsize, stdin);
}

static void print_hex(const unsigned char* buf, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        printf("%s%02x", (i ? " " : ""), buf[i]);
    }
}

static void run_test(const char* label, const char* input_data) {
    FILE* f = tmpfile();
    if (!f) {
        fprintf(stderr, "Failed to create tmpfile\n");
        return;
    }
    if (input_data && *input_data) {
        fwrite(input_data, 1, strlen(input_data), f);
    }
    fflush(f);
    fseek(f, 0, SEEK_SET);

    unsigned char* buf = allocate_buffer();
    if (!buf) {
        fprintf(stderr, "Allocation failed\n");
        fclose(f);
        return;
    }

    size_t n = read_ten_from_stream(buf, 1024, f);
    printf("%s: read %zu bytes; hex: ", label, n);
    print_hex(buf, n);
    printf("\n");

    free(buf);
    fclose(f);
}

int main(void) {
    run_test("Test 1", "abcdefghij123");  /* >10 bytes */
    run_test("Test 2", "short");          /* <10 bytes */
    run_test("Test 3", "");               /* empty */
    run_test("Test 4", "0123456789");     /* exactly 10 bytes */
    run_test("Test 5", "line1\nline2\n"); /* multi-line */
    return 0;
}