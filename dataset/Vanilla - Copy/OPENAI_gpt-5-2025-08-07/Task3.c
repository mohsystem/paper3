#include <stdio.h>

int isPangram(const char* s) {
    int seen[26] = {0};
    int count = 0;
    if (!s) return 0;
    while (*s) {
        unsigned char ch = (unsigned char)*s++;
        if (ch >= 'A' && ch <= 'Z') ch = (unsigned char)(ch - 'A' + 'a');
        if (ch >= 'a' && ch <= 'z') {
            int idx = ch - 'a';
            if (!seen[idx]) {
                seen[idx] = 1;
                count++;
                if (count == 26) return 1;
            }
        }
    }
    return count == 26;
}

int main() {
    const char* tests[5] = {
        "The quick brown fox jumps over the lazy dog",
        "Sphinx of black quartz, judge my vow",
        "Hello, World!",
        "",
        "Pack my box with five dozen liquor jugs."
    };
    for (int i = 0; i < 5; i++) {
        printf("%s\n", isPangram(tests[i]) ? "true" : "false");
    }
    return 0;
}