#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

static const int DATA[] = {100, 200, 300, 400, 500};
#define DATA_SIZE (sizeof(DATA) / sizeof(DATA[0]))

/**
 * @brief Parses an index from args, validates it, and prints the element from DATA.
 * @param argc Argument count.
 * @param argv Argument vector.
 */
void printElementAtIndex(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: ./a.out <index>\n");
        return;
    }

    char* pEnd;
    errno = 0; // Reset errno before calling strtol
    long index_long = strtol(argv[1], &pEnd, 10);

    // Check for conversion errors from strtol
    if (pEnd == argv[1] || *pEnd != '\0') {
        fprintf(stderr, "Error: Argument '%s' is not a valid integer.\n", argv[1]);
        return;
    }
    if (errno == ERANGE) {
        fprintf(stderr, "Error: Index '%s' is out of range for type long.\n", argv[1]);
        return;
    }

    // After successful conversion, check if the index is within array bounds.
    if (index_long < 0 || index_long >= DATA_SIZE) {
        fprintf(stderr, "Error: Index %ld is out of bounds. Valid range is 0 to %zu.\n", index_long, DATA_SIZE - 1);
        return;
    }
    
    // All checks passed, safe to access the array.
    int index = (int)index_long;
    printf("Element at index %d: %d\n", index, DATA[index]);
}

int main(int argc, char* argv[]) {
    // This block allows the program to be run with actual command-line arguments.
    if (argc > 1) {
        printf("--- Running with actual command-line arguments ---\n");
        printElementAtIndex(argc, argv);
        printf("-----------------------------------------------\n\n");
    }

    // Running 5 test cases as requested.
    printf("--- Running 5 built-in test cases ---\n");
    
    char arg1[] = "2";
    char* argv1[] = { (char*)"./a.out", arg1, NULL };
    printf("\n[Test 1] Valid index '2':\n");
    printElementAtIndex(2, argv1);

    char arg2[] = "10";
    char* argv2[] = { (char*)"./a.out", arg2, NULL };
    printf("\n[Test 2] Out of bounds index '10':\n");
    printElementAtIndex(2, argv2);

    char arg3[] = "-1";
    char* argv3[] = { (char*)"./a.out", arg3, NULL };
    printf("\n[Test 3] Negative index '-1':\n");
    printElementAtIndex(2, argv3);

    char arg4[] = "abc";
    char* argv4[] = { (char*)"./a.out", arg4, NULL };
    printf("\n[Test 4] Non-numeric input 'abc':\n");
    printElementAtIndex(2, argv4);

    char* argv5[] = { (char*)"./a.out", NULL };
    printf("\n[Test 5] No arguments:\n");
    printElementAtIndex(1, argv5);

    printf("\n--- Test cases finished ---\n");

    return 0;
}