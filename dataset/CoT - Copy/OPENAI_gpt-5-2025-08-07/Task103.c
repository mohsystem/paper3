#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

bool isStrongPassword(const char* password) {
    if (password == NULL) return false;
    size_t len = strlen(password);
    if (len < 8) return false;

    bool hasLower = false;
    bool hasUpper = false;
    bool hasDigit = false;

    for (size_t i = 0; i < len; ++i) {
        unsigned char ch = (unsigned char)password[i];
        if (!hasLower && islower(ch)) hasLower = true;
        else if (!hasUpper && isupper(ch)) hasUpper = true;
        else if (!hasDigit && isdigit(ch)) hasDigit = true;

        if (hasLower && hasUpper && hasDigit) break;
    }

    return hasLower && hasUpper && hasDigit;
}

int main(void) {
    const char* tests[5] = {
        "Password1",
        "password",
        "PASSWORD1",
        "Pass1",
        "StrongPass123"
    };

    for (int i = 0; i < 5; ++i) {
        bool result = isStrongPassword(tests[i]);
        printf("Test case %d: %s\n", i + 1, result ? "1" : "0");
    }

    return 0;
}