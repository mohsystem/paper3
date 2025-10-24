#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <regex.h>
#include <ctype.h>

static bool within_length(const char* s, size_t min_len, size_t max_len) {
    if (s == NULL) return false;
    size_t len = 0;
    for (const char* p = s; *p != '\0'; ++p) {
        ++len;
        if (len > max_len) return false;
    }
    return len >= min_len && len <= max_len;
}

static bool regex_fullmatch(const char* s, const char* pattern) {
    regex_t re;
    int rc = regcomp(&re, pattern, REG_EXTENDED | REG_NOSUB);
    if (rc != 0) {
        return false;
    }
    rc = regexec(&re, s, 0, NULL, 0);
    regfree(&re);
    return rc == 0;
}

bool isValidEmail(const char* s) {
    if (!within_length(s, 3, 254)) return false;
    // Simple email regex
    const char* pat = "^[A-Za-z0-9._%+-]{1,64}@[A-Za-z0-9.-]{1,253}\\.[A-Za-z]{2,63}$";
    return regex_fullmatch(s, pat);
}

bool isValidUsername(const char* s) {
    if (!within_length(s, 3, 20)) return false;
    const char* pat = "^[A-Za-z][A-Za-z0-9_]{2,19}$";
    return regex_fullmatch(s, pat);
}

bool isValidPhone(const char* s) {
    if (!within_length(s, 7, 20)) return false;
    const char* allowed = "^[+0-9() .-]{7,20}$";
    if (!regex_fullmatch(s, allowed)) return false;
    size_t digits = 0;
    for (const char* p = s; *p != '\0'; ++p) {
        if (*p >= '0' && *p <= '9') ++digits;
    }
    return digits >= 7 && digits <= 15;
}

bool isValidISODate(const char* s) {
    if (!within_length(s, 10, 10)) return false;
    const char* pat = "^[0-9]{4}-(0[1-9]|1[0-2])-(0[1-9]|[12][0-9]|3[01])$";
    return regex_fullmatch(s, pat);
}

bool isValidPassword(const char* s) {
    if (!within_length(s, 8, 64)) return false;
    // Allowed characters: non-whitespace printable ASCII (no spaces)
    const char* allowed = "^[!-~]{8,64}$";
    if (!regex_fullmatch(s, allowed)) return false;

    // Require at least one of each class
    const char* lower = "[a-z]";
    const char* upper = "[A-Z]";
    const char* digit = "[0-9]";
    const char* special = "[^A-Za-z0-9]";

    if (!regex_fullmatch(s, ".*")) return false; // sanity no-op
    regex_t re;
    if (regcomp(&re, lower, REG_EXTENDED)) return false;
    int ok_lower = (regexec(&re, s, 0, NULL, 0) == 0);
    regfree(&re);

    if (regcomp(&re, upper, REG_EXTENDED)) return false;
    int ok_upper = (regexec(&re, s, 0, NULL, 0) == 0);
    regfree(&re);

    if (regcomp(&re, digit, REG_EXTENDED)) return false;
    int ok_digit = (regexec(&re, s, 0, NULL, 0) == 0);
    regfree(&re);

    if (regcomp(&re, special, REG_EXTENDED)) return false;
    int ok_special = (regexec(&re, s, 0, NULL, 0) == 0);
    regfree(&re);

    return ok_lower && ok_upper && ok_digit && ok_special;
}

int main(void) {
    const char* emails[5] = {
        "alice@example.com",
        "alice@@example.com",
        "user.name+tag@sub.domain.co",
        "no-at-symbol.domain",
        "this-is-long_but.valid-123@domain-info.org"
    };
    const char* usernames[5] = {
        "Alice_01",
        "1bad",
        "ab",
        "Valid_User_20",
        "UserNameWithMaxLen20"
    };
    const char* phones[5] = {
        "+1-202-555-0123",
        "12345",
        "(020) 7946 0958",
        "+44 20 7946 0958 12345",
        "202.555.0180"
    };
    const char* dates[5] = {
        "2025-10-14",
        "2025-13-01",
        "2025-02-00",
        "1999-12-31",
        "abcd-ef-gh"
    };
    const char* passwords[5] = {
        "Str0ng!Pwd",
        "weak",
        "NoSpecial123",
        "alllower!!1",
        "GoodPass#2025"
    };

    for (int i = 0; i < 5; ++i) {
        printf("Test case #%d\n", i + 1);
        printf("  Email: %s -> %s\n", emails[i], isValidEmail(emails[i]) ? "true" : "false");
        printf("  Username: %s -> %s\n", usernames[i], isValidUsername(usernames[i]) ? "true" : "false");
        printf("  Phone: %s -> %s\n", phones[i], isValidPhone(phones[i]) ? "true" : "false");
        printf("  Date: %s -> %s\n", dates[i], isValidISODate(dates[i]) ? "true" : "false");
        // Do not print password values
        printf("  Password valid? -> %s\n", isValidPassword(passwords[i]) ? "true" : "false");
    }

    return 0;
}