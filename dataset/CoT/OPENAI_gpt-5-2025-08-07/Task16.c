/* Chain-of-Through process:
   1) Problem: Check if first C-string ends with the second.
   2) Security: Validate NULL pointers, avoid out-of-bounds.
   3) Secure coding: Use strlen and memcmp safely with bounds checks.
   4) Review: Handle empty ending and equal lengths.
   5) Output: Final function with 5 tests.
*/
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

bool solution(const char* s, const char* ending) {
    if (s == NULL || ending == NULL) {
        return false;
    }
    size_t ls = strlen(s);
    size_t le = strlen(ending);
    if (le > ls) {
        return false;
    }
    return memcmp(s + (ls - le), ending, le) == 0;
}

int main(void) {
    const char* tests[5][2] = {
        {"abc", "bc"},
        {"abc", "d"},
        {"hello", ""},
        {"", ""},
        {"abc", "abc"}
    };
    for (int i = 0; i < 5; ++i) {
        bool res = solution(tests[i][0], tests[i][1]);
        printf(res ? "true\n" : "false\n");
    }
    return 0;
}