#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char* my_strdup(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, s, len + 1);
    return out;
}

char* rtrim_copy(const char* s) {
    if (!s) return my_strdup("");
    size_t len = strlen(s);
    if (len == 0) return my_strdup("");
    ssize_t i = (ssize_t)len - 1;
    while (i >= 0 && isspace((unsigned char)s[i])) {
        i--;
    }
    size_t newlen = (i < 0) ? 0 : (size_t)(i + 1);
    char* out = (char*)malloc(newlen + 1);
    if (!out) return NULL;
    if (newlen > 0) memcpy(out, s, newlen);
    out[newlen] = '\0';
    return out;
}

typedef struct {
    char* original;
    char* trimmed;
} ResultC;

// Function: accepts input as parameter and returns outputs
ResultC process_input(const char* input) {
    ResultC res;
    res.original = my_strdup(input ? input : "");
    res.trimmed = rtrim_copy(input ? input : "");
    return res;
}

// Command-line behavior as described
void run_with_args(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Error: exactly one argument required.\n");
        exit(1);
    }
    char* input = argv[1];
    printf("%s\n", input);
    char* trimmed = rtrim_copy(input);
    printf("%s\n", trimmed ? trimmed : "");
    free(trimmed);
    exit(0);
}

int main(int argc, char* argv[]) {
    // 5 test cases
    const char* tests[5] = {
        "hello world   ",
        "   leading and trailing\t\t",
        "no-trailing",
        "tabs-and-newline\t\t\n",
        ""
    };
    for (int i = 0; i < 5; ++i) {
        ResultC r = process_input(tests[i]);
        printf("TEST%d original: [%s]\n", i + 1, r.original ? r.original : "");
        printf("TEST%d trimmed: [%s]\n", i + 1, r.trimmed ? r.trimmed : "");
        free(r.original);
        free(r.trimmed);
    }
    // If any CLI args are provided, run the specified behavior
    if (argc > 1) {
        run_with_args(argc, argv);
    }
    return 0;
}