#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

static const int id_sequence[] = {101, 203, 307, 409, 512, 678, 789, 890, 901, 1024};
static const size_t ID_LEN = sizeof(id_sequence) / sizeof(id_sequence[0]);

int get_id_at_index(int index) {
    if (index < 0 || (size_t)index >= ID_LEN) {
        errno = EINVAL;
        return 0; // Sentinel; caller should check errno for validity
    }
    errno = 0;
    return id_sequence[index];
}

int main(void) {
    char buf[128];

    printf("Enter index (0-%zu): ", ID_LEN - 1);
    if (fgets(buf, sizeof(buf), stdin) != NULL) {
        // Remove trailing newline if present
        buf[strcspn(buf, "\r\n")] = '\0';

        char *endptr = NULL;
        errno = 0;
        long val = strtol(buf, &endptr, 10);

        if (errno == ERANGE || endptr == buf || *endptr != '\0') {
            puts("Invalid input: not a valid integer.");
        } else if (val < INT_MIN || val > INT_MAX) {
            puts("Invalid input: integer out of range.");
        } else {
            int idx = (int)val;
            int result = get_id_at_index(idx);
            if (errno == EINVAL) {
                printf("Invalid index: must be between 0 and %zu.\n", ID_LEN - 1);
            } else {
                printf("Value at index %d is: %d\n", idx, result);
            }
        }
    } else {
        puts("No input received.");
    }

    // Five test cases
    puts("Running 5 test cases:");
    int tests[5] = {0, 3, 9, -1, 100};
    for (size_t i = 0; i < 5; ++i) {
        errno = 0;
        int value = get_id_at_index(tests[i]);
        if (errno == EINVAL) {
            printf("Index %d -> error: Index out of bounds\n", tests[i]);
        } else {
            printf("Index %d -> %d\n", tests[i], value);
        }
    }

    return 0;
}