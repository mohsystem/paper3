#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

bool isStrongPassword(const char *password) {
    if (password == NULL) return false;

    size_t len = strlen(password);
    if (len < 8) return false;

    bool hasUpper = false;
    bool hasLower = false;
    bool hasDigit = false;

    for (size_t i = 0; i < len; i++) {
        unsigned char ch = (unsigned char)password[i];
        if (isupper(ch)) {
            hasUpper = true;
        } else if (islower(ch)) {
            hasLower = true;
        } else if (isdigit(ch)) {
            hasDigit = true;
        }
        if (hasUpper && hasLower && hasDigit) {
            return true;
        }
    }
    return hasUpper && hasLower && hasDigit;
}

int main(void) {
    const char *tests[] = {"Password1", "password1", "PASSWORD1", "Passw1", "Secure123"};
    size_t num = sizeof(tests) / sizeof(tests[0]);
    for (size_t i = 0; i < num; i++) {
        printf("%s -> %s\n", tests[i], isStrongPassword(tests[i]) ? "true" : "false");
    }
    return 0;
}