#include <stdio.h>
#include <string.h>

int solution(const char* s, const char* ending) {
    if (s == NULL || ending == NULL) return 0;
    size_t ls = strlen(s);
    size_t le = strlen(ending);
    if (le > ls) return 0;
    return memcmp(s + (ls - le), ending, le) == 0 ? 1 : 0;
}

int main(void) {
    const char* tests[][2] = {
        {"abc", "bc"},
        {"abc", "d"},
        {"", ""},
        {"abc", ""},
        {"a", "ab"}
    };
    for (size_t i = 0; i < 5; i++) {
        printf("%s\n", solution(tests[i][0], tests[i][1]) ? "true" : "false");
    }
    return 0;
}