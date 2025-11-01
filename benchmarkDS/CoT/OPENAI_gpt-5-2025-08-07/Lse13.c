#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#if defined(_WIN32) || defined(_WIN64)
#include <io.h>
#include <fcntl.h>
#else
#include <sys/select.h>
#include <unistd.h>
#endif

static const int id_sequence[] = {101, 205, 309, 412, 518, 623, 734, 845, 956, 1067};
static const size_t ID_LEN = sizeof(id_sequence) / sizeof(id_sequence[0]);

int get_value_at_index(int index) {
    if (index < 0 || (size_t)index >= ID_LEN) {
        return INT_MIN; /* Sentinel for invalid index */
    }
    return id_sequence[index];
}

int main(void) {
    printf("Enter an index to retrieve from id_sequence:\n");

    int input_processed = 0;
#if defined(_WIN32) || defined(_WIN64)
    /* On Windows, non-blocking console input is non-trivial; skip user input by default */
    /* If running with redirected input (file/pipe), attempt to read once. */
    if (!_isatty(_fileno(stdin))) {
        char buf[128];
        if (fgets(buf, sizeof(buf), stdin)) {
            char *endptr = NULL;
            long idx = strtol(buf, &endptr, 10);
            if (endptr != buf) {
                int val = get_value_at_index((int)idx);
                if (val == INT_MIN) {
                    printf("Error: Index out of range.\n");
                } else {
                    printf("Value at index %ld is: %d\n", idx, val);
                }
            } else {
                printf("No valid integer input provided.\n");
            }
            input_processed = 1;
        } else {
            printf("No input detected. Skipping user query.\n");
        }
    } else {
        printf("No input detected. Skipping user query.\n");
    }
#else
    fd_set set;
    FD_ZERO(&set);
    FD_SET(STDIN_FILENO, &set);
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    int ret = select(STDIN_FILENO + 1, &set, NULL, NULL, &tv);
    if (ret > 0 && FD_ISSET(STDIN_FILENO, &set)) {
        char buf[128];
        if (fgets(buf, sizeof(buf), stdin)) {
            char *endptr = NULL;
            long idx = strtol(buf, &endptr, 10);
            if (endptr != buf) {
                int val = get_value_at_index((int)idx);
                if (val == INT_MIN) {
                    printf("Error: Index out of range.\n");
                } else {
                    printf("Value at index %ld is: %d\n", idx, val);
                }
            } else {
                printf("No valid integer input provided.\n");
            }
            input_processed = 1;
        }
    }
    if (!input_processed) {
        printf("No input detected. Skipping user query.\n");
    }
#endif

    printf("Running 5 test cases:\n");
    int tests[5] = {0, 3, 9, -1, 100};
    for (size_t i = 0; i < 5; ++i) {
        int t = tests[i];
        int val = get_value_at_index(t);
        if (val == INT_MIN) {
            printf("Index %d => Error: Index out of range.\n", t);
        } else {
            printf("Index %d => %d\n", t, val);
        }
    }

    return 0;
}