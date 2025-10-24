#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

char* longest(const char* s1, const char* s2) {
    bool seen[26];
    for (int i = 0; i < 26; i++) {
        seen[i] = false;
    }

    const char* arr[2] = { s1, s2 };
    for (int j = 0; j < 2; j++) {
        const char* s = arr[j];
        if (s == NULL) continue;
        for (const char* p = s; *p != '\0'; ++p) {
            unsigned char c = (unsigned char)*p;
            if (c >= 'a' && c <= 'z') {
                seen[c - 'a'] = true;
            }
        }
    }

    char* res = (char*)malloc(27);
    if (!res) {
        return NULL;
    }
    int k = 0;
    for (int i = 0; i < 26; i++) {
        if (seen[i]) {
            res[k++] = (char)('a' + i);
        }
    }
    res[k] = '\0';
    return res;
}

int main() {
    const char* A[5] = {
        "xyaabbbccccdefww",
        "abcdefghijklmnopqrstuvwxyz",
        "",
        "abc123",
        "thequickbrownfox"
    };
    const char* B[5] = {
        "xxxxyyyyabklmopq",
        "abcdefghijklmnopqrstuvwxyz",
        "",
        "def!ghi",
        "jumpsoverthelazydog"
    };

    for (int i = 0; i < 5; i++) {
        char* r = longest(A[i], B[i]);
        if (r) {
            printf("%s\n", r);
            free(r);
        } else {
            printf("\n");
        }
    }
    return 0;
}