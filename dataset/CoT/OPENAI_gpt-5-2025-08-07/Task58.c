#include <stdio.h>
#include <stdbool.h>

bool XO(const char* s) {
    if (s == NULL) {
        return true;
    }
    int x = 0, o = 0;
    for (const char* p = s; *p != '\0'; ++p) {
        if (*p == 'x' || *p == 'X') {
            x++;
        } else if (*p == 'o' || *p == 'O') {
            o++;
        }
    }
    return x == o;
}

int main(void) {
    const char* tests[5] = {"ooxx", "xooxx", "ooxXm", "zpzpzpp", "zzoo"};
    for (int i = 0; i < 5; ++i) {
        bool res = XO(tests[i]);
        printf("%s\n", res ? "true" : "false");
    }
    return 0;
}