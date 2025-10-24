#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

// Step 1-5 combined into secure implementation

static bool contains_substring_case_insensitive(const char* hay, const char* needle) {
    if (!hay || !needle) return false;
    size_t hlen = strlen(hay);
    size_t nlen = strlen(needle);
    if (nlen == 0 || nlen > hlen) return false;

    for (size_t i = 0; i + nlen <= hlen; ++i) {
        size_t j = 0;
        for (; j < nlen; ++j) {
            unsigned char hc = (unsigned char)hay[i + j];
            unsigned char nc = (unsigned char)needle[j];
            if ((char)tolower(hc) != (char)tolower(nc)) break;
        }
        if (j == nlen) return true;
    }
    return false;
}

static bool has_consecutive_sequence_digits(const char* s) {
    int run = 1;
    int prev = -1;
    for (size_t i = 0; s[i] != '\0'; ++i) {
        unsigned char ch = (unsigned char)s[i];
        if (!isdigit(ch)) {
            run = 1;
            prev = -1;
            continue;
        }
        int cval = ch;
        if (prev != -1) {
            if (cval == prev + 1 || cval == prev - 1) {
                run++;
                if (run >= 4) return true;
            } else {
                run = 1;
            }
        }
        prev = cval;
    }
    return false;
}

static bool has_consecutive_sequence_letters(const char* s) {
    int run = 1;
    int prev = -1;
    for (size_t i = 0; s[i] != '\0'; ++i) {
        unsigned char raw = (unsigned char)s[i];
        if (!isalpha(raw)) {
            run = 1;
            prev = -1;
            continue;
        }
        int ch = tolower(raw);
        if (prev != -1) {
            if (ch == prev + 1 || ch == prev - 1) {
                run++;
                if (run >= 4) return true;
            } else {
                run = 1;
            }
        }
        prev = ch;
    }
    return false;
}

bool isValidAdminPassword(const char* pwd) {
    if (pwd == NULL) return false;

    // length check with early stop if >128
    size_t len = 0;
    while (pwd[len] != '\0') {
        len++;
        if (len > 128) return false;
    }
    if (len < 12) return false;

    bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;

    int repeatCount = 1;
    for (size_t i = 0; i < len; ++i) {
        unsigned char ch = (unsigned char)pwd[i];

        if (isspace(ch)) return false;

        if (isupper(ch)) hasUpper = true;
        else if (islower(ch)) hasLower = true;
        else if (isdigit(ch)) hasDigit = true;

        if (!isalnum(ch) && !isspace(ch)) hasSpecial = true;

        if (i > 0) {
            if (pwd[i] == pwd[i - 1]) {
                repeatCount++;
                if (repeatCount >= 3) return false;
            } else {
                repeatCount = 1;
            }
        }
    }

    if (!(hasUpper && hasLower && hasDigit && hasSpecial)) return false;

    const char* banned[] = { "password", "admin", "qwerty", "letmein", "123456" };
    for (size_t i = 0; i < sizeof(banned)/sizeof(banned[0]); ++i) {
        if (contains_substring_case_insensitive(pwd, banned[i])) return false;
    }

    if (has_consecutive_sequence_digits(pwd) || has_consecutive_sequence_letters(pwd)) return false;

    return true;
}

int main(void) {
    const char* tests[] = {
        "password123!",
        "Admin123!",
        "ValidStrong#2024Xx",
        "NoSpecialChar1234",
        "AAAaaa111!!!"
    };
    for (int i = 0; i < 5; ++i) {
        printf("Test %d: %d\n", i + 1, isValidAdminPassword(tests[i]) ? 1 : 0);
    }
    return 0;
}