#include <stdio.h>
#include <string.h>

char* expandedForm(long long num) {
    static char buf[2048];
    buf[0] = '\0';
    char s[32];
    sprintf(s, "%lld", num);
    int n = (int)strlen(s);
    int first = 1;
    for (int i = 0; i < n; i++) {
        int d = s[i] - '0';
        if (d == 0) continue;
        long long place = 1;
        for (int j = i + 1; j < n; j++) place *= 10;
        long long val = d * place;
        char temp[64];
        if (first) {
            sprintf(temp, "%lld", val);
            first = 0;
        } else {
            sprintf(temp, " + %lld", val);
        }
        strcat(buf, temp);
    }
    return buf;
}

int main() {
    long long tests[5] = {12, 42, 70304, 9000000, 105};
    for (int i = 0; i < 5; i++) {
        printf("%lld -> %s\n", tests[i], expandedForm(tests[i]));
    }
    return 0;
}