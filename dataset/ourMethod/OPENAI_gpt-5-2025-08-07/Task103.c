#include <stdio.h>
#include <string.h>
#include <ctype.h>

int is_strong_password(const char *pwd) {
    if (pwd == NULL) {
        return 0;
    }
    size_t len = strlen(pwd);
    if (len < 8U) {
        return 0;
    }

    int hasUpper = 0;
    int hasLower = 0;
    int hasDigit = 0;

    for (size_t i = 0; i < len; i++) {
        unsigned char uc = (unsigned char)pwd[i];
        if (isupper(uc)) {
            hasUpper = 1;
        } else if (islower(uc)) {
            hasLower = 1;
        } else if (isdigit(uc)) {
            hasDigit = 1;
        }
        if (hasUpper && hasLower && hasDigit) {
            break;
        }
    }
    return (hasUpper && hasLower && hasDigit) ? 1 : 0;
}

int main(void) {
    const char *tests[5] = {
        "Short7",
        "alllowercase1",
        "ALLUPPERCASE1",
        "NoDigitsHere",
        "Str0ngPass"
    };
    int expected[5] = {0, 0, 0, 0, 1};

    for (int i = 0; i < 5; i++) {
        int result = is_strong_password(tests[i]);
        printf("Test %d: expected=%s, got=%s, verdict=%s\n",
               i + 1,
               expected[i] ? "true" : "false",
               result ? "true" : "false",
               (result == expected[i]) ? "PASS" : "FAIL");
    }
    return 0;
}