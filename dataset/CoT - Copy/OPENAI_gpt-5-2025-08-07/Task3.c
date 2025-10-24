/* Chain-of-Through Step 1: Problem understanding - pangram detection, ignore non-letters, case-insensitive.
   Chain-of-Through Step 2: Security requirements - safe character handling, avoid undefined behavior with tolower.
   Chain-of-Through Step 3: Secure coding generation - strict 'a'..'z' checks, early exit.
   Chain-of-Through Step 4: Code review - ensure no buffer overflows and safe iteration.
   Chain-of-Through Step 5: Secure code output - final implementation. */

#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>

bool is_pangram(const char *s) {
    if (s == NULL) return false;
    bool seen[26] = { false };
    int count = 0;
    for (const unsigned char *p = (const unsigned char *)s; *p != '\0'; ++p) {
        char c = (char)tolower(*p);
        if (c >= 'a' && c <= 'z') {
            int idx = c - 'a';
            if (!seen[idx]) {
                seen[idx] = true;
                count++;
                if (count == 26) return true;
            }
        }
    }
    return count == 26;
}

int main(void) {
    const char *tests[5] = {
        "The quick brown fox jumps over the lazy dog",
        "Hello, world!",
        "",
        "Sphinx of black quartz, judge my vow 123!!!",
        "abcdefghijklmno pqrstuvwxy"
    };
    for (int i = 0; i < 5; ++i) {
        bool res = is_pangram(tests[i]);
        printf("%s\n", res ? "True" : "False");
    }
    return 0;
}