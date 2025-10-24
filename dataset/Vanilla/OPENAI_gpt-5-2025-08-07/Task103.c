#include <stdio.h>
#include <string.h>
#include <ctype.h>

int isStrongPassword(const char* password) {
    if (password == NULL) return 0;
    size_t len = strlen(password);
    if (len < 8) return 0;
    int hasUpper = 0, hasLower = 0, hasDigit = 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)password[i];
        if (isupper(c)) hasUpper = 1;
        else if (islower(c)) hasLower = 1;
        else if (isdigit(c)) hasDigit = 1;
    }
    return hasUpper && hasLower && hasDigit;
}

int main() {
    const char* tests[] = {
        "Password1",
        "password",
        "PASSWORD1",
        "Passw1",
        "StrongPass123"
    };
    size_t n = sizeof(tests) / sizeof(tests[0]);
    for (size_t i = 0; i < n; i++) {
        printf("%s -> %s\n", tests[i], isStrongPassword(tests[i]) ? "true" : "false");
    }
    return 0;
}