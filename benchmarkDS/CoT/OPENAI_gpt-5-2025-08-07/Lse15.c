#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// Chain-of-Through process:
// 1) Problem understanding: Prompt user for an index, return value at that index from id_sequence.
// 2) Security requirements: Validate input and bounds; handle EOF/invalid input safely.
// 3) Secure coding generation: Use fgets + strtol; avoid unsafe scanf; validate before array access.
// 4) Code review: Ensure no unchecked access; robustly handle parsing errors.
// 5) Secure code output: Final code applies all validations.

static const int ID_SEQUENCE[] = {101, 202, 303, 404, 505, 606, 707, 808, 909, 1001};
static const size_t ID_SEQUENCE_LEN = sizeof(ID_SEQUENCE) / sizeof(ID_SEQUENCE[0]);

int getValueAtIndex(int indexParam) {
    char buf[128];
    for (;;) {
        printf("Enter index (0..%zu): ", ID_SEQUENCE_LEN - 1);
        fflush(stdout);
        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            return -1; // EOF or error
        }
        // Trim leading/trailing whitespace
        size_t len = strlen(buf);
        while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r' || buf[len - 1] == ' ' || buf[len - 1] == '\t')) {
            buf[--len] = '\0';
        }
        char *start = buf;
        while (*start == ' ' || *start == '\t' || *start == '\r' || *start == '\n') {
            start++;
        }
        if (*start == '\0') {
            puts("Invalid input: empty. Please enter an integer index.");
            continue;
        }

        errno = 0;
        char *endptr = NULL;
        long val = strtol(start, &endptr, 10);
        if (errno != 0 || endptr == start || *endptr != '\0') {
            puts("Invalid input: not a valid integer. Try again.");
            continue;
        }
        if (val < 0 || (unsigned long)val >= ID_SEQUENCE_LEN) {
            printf("Index out of bounds. Valid range is 0..%zu. Try again.\n", ID_SEQUENCE_LEN - 1);
            continue;
        }
        return ID_SEQUENCE[(size_t)val];
    }
}

int main(void) {
    // 5 test cases; function will prompt for index each time
    for (int t = 1; t <= 5; ++t) {
        int value = getValueAtIndex(-1);
        printf("Test %d -> Value: %d\n", t, value);
    }
    return 0;
}