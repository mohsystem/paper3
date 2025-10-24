#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

int isValidAdminPassword(const char* pwd) {
    if (pwd == NULL) return 0;

    size_t len = strlen(pwd);
    if (len < 12) return 0;

    int hasUpper = 0, hasLower = 0, hasDigit = 0, hasSpecial = 0;

    char prev1 = 0, prev2 = 0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char ch = (unsigned char)pwd[i];
        if (isspace(ch)) return 0;

        if (isupper(ch)) hasUpper = 1;
        else if (islower(ch)) hasLower = 1;
        else if (isdigit(ch)) hasDigit = 1;
        else hasSpecial = 1;

        if (i >= 2) {
            if (pwd[i] == prev1 && pwd[i] == prev2) return 0; // 3 identical in a row
        }
        prev2 = prev1;
        prev1 = pwd[i];
    }

    if (!(hasUpper && hasLower && hasDigit && hasSpecial)) return 0;

    char* lowerPwd = (char*)malloc(len + 1);
    if (!lowerPwd) return 0;
    for (size_t i = 0; i < len; ++i) {
        lowerPwd[i] = (char)tolower((unsigned char)pwd[i]);
    }
    lowerPwd[len] = '\0';

    const char* badEquals[] = {"password", "admin", "admin123", "123456", "qwerty", "letmein", "welcome", "passw0rd", "iloveyou"};
    size_t beCount = sizeof(badEquals) / sizeof(badEquals[0]);
    for (size_t i = 0; i < beCount; ++i) {
        if (strcmp(lowerPwd, badEquals[i]) == 0) {
            free(lowerPwd);
            return 0;
        }
    }

    const char* badContains[] = {"admin", "password", "qwerty", "12345"};
    size_t bcCount = sizeof(badContains) / sizeof(badContains[0]);
    for (size_t i = 0; i < bcCount; ++i) {
        if (strstr(lowerPwd, badContains[i]) != NULL) {
            free(lowerPwd);
            return 0;
        }
    }

    free(lowerPwd);
    return 1;
}

int main(void) {
    const char* tests[5] = {
        "Admin@123",
        "Str0ng!AdminPwd",
        "CorrectHorse9!",
        "aaaaaaaaaaaa",
        "N0_Spaces-Allowed?"
    };
    for (int i = 0; i < 5; ++i) {
        printf("%s -> %s\n", tests[i], isValidAdminPassword(tests[i]) ? "true" : "false");
    }
    return 0;
}