/* Chain-of-Through process in code generation:
 * 1) Problem understanding: Read a string, process by trimming/collapsing whitespace to single spaces, convert to uppercase.
 * 2) Security requirements: Avoid buffer overflow using fgets with a bounded buffer; safe memory handling; careful ctype usage.
 * 3) Secure coding generation: Implement processing with allocated output sized to input length+1; cast to unsigned char for ctype.
 * 4) Code review: No unsafe functions (e.g., gets/scanf without bounds); check allocations; free allocated memory.
 * 5) Secure code output: Final code applies bounds and safe APIs.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char* process_input(const char* input) {
    if (input == NULL) {
        char* out = (char*)malloc(1);
        if (out) out[0] = '\0';
        return out;
    }
    size_t len = strlen(input);
    char* out = (char*)malloc(len + 1); // output will be <= input length after collapsing/trim
    if (!out) {
        return NULL;
    }

    size_t j = 0;
    int last_was_space = 1; // treat leading whitespace as suppressed

    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)input[i];
        if (isspace(c)) {
            if (j > 0 && !last_was_space) {
                out[j++] = ' ';
                last_was_space = 1;
            }
        } else {
            out[j++] = (char)toupper(c);
            last_was_space = 0;
        }
    }
    if (j > 0 && out[j - 1] == ' ') {
        j--;
    }
    out[j] = '\0';
    return out;
}

int main(void) {
    enum { BUF_SIZE = 4096 };
    char buffer[BUF_SIZE];

    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        buffer[0] = '\0';
    } else {
        size_t l = strlen(buffer);
        if (l > 0 && buffer[l - 1] == '\n') {
            buffer[l - 1] = '\0';
        }
    }

    char* processed = process_input(buffer);
    if (processed) {
        printf("%s\n", processed);
        free(processed);
    } else {
        // Allocation failed; print nothing to remain safe.
    }

    // 5 test cases
    const char* tests[5] = {
        "  Hello,   world!  ",
        "",
        "Tabs\tand\nnewlines",
        "1234 abc DEF",
        "   multiple    spaces   here   "
    };
    for (int i = 0; i < 5; ++i) {
        char* res = process_input(tests[i]);
        if (res) {
            printf("%s\n", res);
            free(res);
        }
    }

    return 0;
}