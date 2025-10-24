#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>

bool isIsogram(const char *s) {
    int mask = 0;
    if (!s) return true;
    for (const unsigned char *p = (const unsigned char *)s; *p; ++p) {
        char c = tolower(*p);
        if (c < 'a' || c > 'z') continue;
        int bit = c - 'a';
        if (mask & (1 << bit)) return false;
        mask |= (1 << bit);
    }
    return true;
}

int main() {
    const char *tests[5] = {"Dermatoglyphics", "aba", "moOse", "", "isogram"};
    for (int i = 0; i < 5; ++i) {
        printf("%s -> %s\n", tests[i], isIsogram(tests[i]) ? "true" : "false");
    }
    return 0;
}