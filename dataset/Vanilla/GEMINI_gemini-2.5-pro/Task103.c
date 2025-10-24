#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

bool checkPasswordStrength(const char* password) {
    if (strlen(password) < 8) {
        return false;
    }

    bool hasUpper = false;
    bool hasLower = false;
    bool hasDigit = false;

    for (int i = 0; password[i] != '\0'; i++) {
        if (isupper(password[i])) {
            hasUpper = true;
        } else if (islower(password[i])) {
            hasLower = true;
        } else if (isdigit(password[i])) {
            hasDigit = true;
        }
    }

    return hasUpper && hasLower && hasDigit;
}

int main() {
    const char* testPasswords[] = {
        "StrongPassword123",
        "weak",
        "NoDigitsHere",
        "nouppercase123",
        "NOLOWERCASE123"
    };
    int numTests = sizeof(testPasswords) / sizeof(testPasswords[0]);

    for (int i = 0; i < numTests; i++) {
        bool isStrong = checkPasswordStrength(testPasswords[i]);
        printf("Password: \"%s\" -> %s\n", testPasswords[i], isStrong ? "Strong" : "Weak");
    }

    return 0;
}