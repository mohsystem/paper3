#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

struct BufferResult {
    int success;
    char *content;   // dynamically allocated; caller must free via free_buffer_result
    int truncated;
    char message[64];
};

static char* sanitize(const char* input) {
    if (input == NULL) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    size_t len = strlen(input);
    char* out = (char*)malloc(len + 1);
    if (!out) {
        return NULL;
    }
    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char ch = (unsigned char)input[i];
        if (ch == '\n' || ch == '\t' || (ch >= 32 && ch <= 126)) {
            out[j++] = (char)ch;
        }
    }
    out[j] = '\0';
    return out;
}

struct BufferResult handle_input_to_fixed_buffer(const char* input, size_t buffer_size) {
    struct BufferResult res;
    res.success = 0;
    res.content = NULL;
    res.truncated = 0;
    snprintf(res.message, sizeof(res.message), "%s", "INIT");

    if (buffer_size < 1 || buffer_size > 4096) {
        snprintf(res.message, sizeof(res.message), "%s", "Invalid buffer size");
        return res;
    }

    char* sanitized = sanitize(input);
    if (!sanitized) {
        snprintf(res.message, sizeof(res.message), "%s", "Allocation failure");
        return res;
    }

    size_t sanitized_len = strlen(sanitized);
    size_t copy_len = sanitized_len < buffer_size ? sanitized_len : buffer_size;

    char* buffer = (char*)malloc(copy_len + 1);
    if (!buffer) {
        free(sanitized);
        snprintf(res.message, sizeof(res.message), "%s", "Allocation failure");
        return res;
    }

    if (copy_len > 0) {
        memcpy(buffer, sanitized, copy_len);
    }
    buffer[copy_len] = '\0';

    res.success = 1;
    res.content = buffer;
    res.truncated = sanitized_len > copy_len ? 1 : 0;
    snprintf(res.message, sizeof(res.message), "%s", "OK");

    free(sanitized);
    return res;
}

void free_buffer_result(struct BufferResult* r) {
    if (r && r->content) {
        free(r->content);
        r->content = NULL;
    }
}

int main(void) {
    size_t buf_size = 16;
    const char* tests[5];
    tests[0] = "Hello World";                         // fits
    tests[1] = "";                                    // empty
    tests[2] = "This is a very long input string!!!"; // long
    char ctrl[16];
    ctrl[0] = 'H'; ctrl[1] = 'i'; ctrl[2] = 1; ctrl[3] = 2; ctrl[4] = ' '; ctrl[5] = 't'; ctrl[6] = 'h'; ctrl[7] = 'e'; ctrl[8] = 'r'; ctrl[9] = 'e'; ctrl[10] = '\0';
    tests[3] = ctrl;                                  // control chars
    tests[4] = "1234567890ABCDEF";                    // boundary

    for (int i = 0; i < 5; i++) {
        struct BufferResult res = handle_input_to_fixed_buffer(tests[i], buf_size);
        printf("Test %d: BufferResult{success=%s, content=\"%s\", truncated=%s, message=\"%s\"}\n",
               i + 1,
               res.success ? "true" : "false",
               res.content ? res.content : "",
               res.truncated ? "true" : "false",
               res.message);
        free_buffer_result(&res);
    }

    return 0;
}