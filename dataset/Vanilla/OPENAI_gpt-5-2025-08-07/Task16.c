#include <stdio.h>
#include <string.h>
#include <stdbool.h>

bool solution(const char* str, const char* ending) {
    size_t ls = strlen(str);
    size_t le = strlen(ending);
    if (le > ls) return false;
    return strncmp(str + (ls - le), ending, le) == 0;
}

int main(void) {
    const char* tests[][2] = {
        {"abc", "bc"},
        {"abc", "d"},
        {"hello", ""},
        {"", ""},
        {"JavaTask16", "Task16"}
    };
    for (int i = 0; i < 5; ++i) {
        bool res = solution(tests[i][0], tests[i][1]);
        printf("solution(\"%s\", \"%s\") -> %s\n", tests[i][0], tests[i][1], res ? "true" : "false");
    }
    return 0;
}