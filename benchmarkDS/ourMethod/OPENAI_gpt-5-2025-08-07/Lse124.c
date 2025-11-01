#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static const size_t MAX_LEN = 1048576; /* 1 MiB */

static size_t safe_strnlen_local(const char *s, size_t maxlen) {
    size_t i = 0;
    if (s == NULL) return 0;
    for (; i < maxlen; ++i) {
        if (s[i] == '\0') break;
    }
    return i;
}

int process(size_t argc, const char *argv[]) {
    if (argc != 1 || argv == NULL || argv[0] == NULL) {
        fputs("Error: expected exactly one argument.\n", stderr);
        return 1;
    }

    const char *input = argv[0];
    size_t len = safe_strnlen_local(input, MAX_LEN + 1);
    if (len > MAX_LEN) {
        fputs("Error: input too long.\n", stderr);
        return 1;
    }

    printf("%s\n", input);

    char *buf = (char *)malloc(len + 1);
    if (buf == NULL) {
        fputs("Error: allocation failed.\n", stderr);
        return 1;
    }
    memcpy(buf, input, len);
    buf[len] = '\0';

    while (len > 0 && isspace((unsigned char)buf[len - 1])) {
        buf[len - 1] = '\0';
        --len;
    }

    printf("%s\n", buf);

    free(buf);
    return 0;
}

int main(void) {
    const char *t1[] = {"hello"};
    const char *t2[] = {"hello \t \n"};
    /* t3: zero arguments */
    const char *t4[] = {"a", "b"};
    const char *t5[] = {"   "};

    int code;

    puts("---- Test 1 ----");
    code = process(1, t1);
    printf("Exit code: %d\n", code);

    puts("---- Test 2 ----");
    code = process(1, t2);
    printf("Exit code: %d\n", code);

    puts("---- Test 3 ----");
    code = process(0, NULL);
    printf("Exit code: %d\n", code);

    puts("---- Test 4 ----");
    code = process(2, t4);
    printf("Exit code: %d\n", code);

    puts("---- Test 5 ----");
    code = process(1, t5);
    printf("Exit code: %d\n", code);

    return 0;
}