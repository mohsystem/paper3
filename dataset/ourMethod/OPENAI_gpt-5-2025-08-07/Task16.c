#include <stdio.h>
#include <string.h>

int solution(const char* str, const char* ending) {
    if (str == NULL || ending == NULL) {
        return 0; // Fail closed on invalid input
    }
    size_t len_str = strlen(str);
    size_t len_end = strlen(ending);
    if (len_end > len_str) {
        return 0;
    }
    if (len_end == 0) {
        return 1;
    }
    return memcmp(str + (len_str - len_end), ending, len_end) == 0 ? 1 : 0;
}

int main(void) {
    struct TestCase {
        const char* s;
        const char* e;
        int expected;
    } tests[5] = {
        {"abc", "bc", 1},
        {"abc", "d", 0},
        {"hello", "", 1},
        {"testing", "ing", 1},
        {"short", "longer", 0}
    };

    for (int i = 0; i < 5; i++) {
        int result = solution(tests[i].s, tests[i].e);
        printf("solution(\"%s\", \"%s\") = %s | expected: %s\n",
               tests[i].s, tests[i].e,
               result ? "true" : "false",
               tests[i].expected ? "true" : "false");
    }
    return 0;
}