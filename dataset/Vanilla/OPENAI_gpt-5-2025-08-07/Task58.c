#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>

bool XO(const char* s) {
    int x = 0, o = 0;
    for (const char* p = s; *p; ++p) {
        char c = (char)tolower((unsigned char)*p);
        if (c == 'x') x++;
        else if (c == 'o') o++;
    }
    return x == o;
}

int main() {
    const char* tests[] = {"ooxx", "xooxx", "ooxXm", "zpzpzpp", "zzoo"};
    for (int i = 0; i < 5; ++i) {
        bool res = XO(tests[i]);
        printf("XO(\"%s\") => %s\n", tests[i], res ? "true" : "false");
    }
    return 0;
}