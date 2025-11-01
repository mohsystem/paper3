#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
   Process arguments according to the description:
   - Ensure exactly one argument.
   - Check the argument is a string (non-NULL).
   - For loop trims trailing whitespace (spaces and tabs).
   - Second for loop trims the newlines (removes all '\n' and '\r').
   - Last line null terminates the string.
   Returns a newly allocated string on success, or NULL on invalid input.
*/
char* process(int argc, const char* argv[]) {
    // First if: ensure exactly one argument
    if (argc != 1) {
        return NULL;
    }
    // Check the argument is a string
    if (argv == NULL || argv[0] == NULL) {
        return NULL;
    }

    // Duplicate input to a mutable buffer
    size_t len = strlen(argv[0]);
    char* buf = (char*)malloc(len + 1);
    if (!buf) return NULL;
    memcpy(buf, argv[0], len + 1);

    // For loop trims trailing whitespace (spaces and tabs)
    size_t end = len;
    for (; end > 0 && (buf[end - 1] == ' ' || buf[end - 1] == '\t'); --end) {
        // trimming
    }
    buf[end] = '\0'; // null terminate after trimming trailing spaces/tabs

    // Second for loop trims the newlines (removes all '\n' and '\r')
    size_t i, j;
    for (i = 0, j = 0; buf[i] != '\0'; ++i) {
        if (buf[i] != '\n' && buf[i] != '\r') {
            buf[j++] = buf[i];
        }
    }
    buf[j] = '\0'; // Last line null terminates the string

    return buf;
}

int main(void) {
    const char* t1[] = {"Hello World   "};
    const char* t2[] = {"Line with newline\n\n"};
    const char* t3[] = {" Mix \t \nLine\r\nBreaks\t\t"};
    // Invalid cases
    const char** t4 = NULL; // argc = 0
    const char* t5[] = {"A", "B"};

    char* r1 = process(1, t1);
    char* r2 = process(1, t2);
    char* r3 = process(1, t3);
    char* r4 = process(0, t4);
    char* r5 = process(2, t5);

    printf("Test 1: %s\n", r1 ? r1 : "null");
    printf("Test 2: %s\n", r2 ? r2 : "null");
    printf("Test 3: %s\n", r3 ? r3 : "null");
    printf("Test 4: %s\n", r4 ? r4 : "null");
    printf("Test 5: %s\n", r5 ? r5 : "null");

    free(r1);
    free(r2);
    free(r3);
    free(r4);
    free(r5);
    return 0;
}