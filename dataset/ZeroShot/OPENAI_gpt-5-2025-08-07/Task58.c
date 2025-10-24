#include <stdio.h>

int XO(const char* s) {
    if (s == NULL) return 1;
    int x = 0, o = 0;
    for (const char* p = s; *p != '\0'; ++p) {
        char c = *p;
        if (c == 'x' || c == 'X') x++;
        else if (c == 'o' || c == 'O') o++;
    }
    return x == o ? 1 : 0;
}

int main(void) {
    const char* tests[] = {"ooxx", "xooxx", "ooxXm", "zpzpzpp", "zzoo"};
    for (int i = 0; i < 5; ++i) {
        printf("%s\n", XO(tests[i]) ? "true" : "false");
    }
    return 0;
}