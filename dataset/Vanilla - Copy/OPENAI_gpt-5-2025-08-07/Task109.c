#include <stdio.h>
#include <regex.h>
#include <string.h>

int match_regex(const char* pattern, const char* text) {
    regex_t regex;
    int ret = regcomp(&regex, pattern, REG_EXTENDED | REG_NOSUB);
    if (ret != 0) {
        regfree(&regex);
        return 0;
    }
    ret = regexec(&regex, text, 0, NULL, 0);
    regfree(&regex);
    return ret == 0;
}

int is_valid_email(const char* s) {
    if (s == NULL) return 0;
    const char* pattern = "^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$";
    return match_regex(pattern, s);
}

int is_valid_phone(const char* s) {
    if (s == NULL) return 0;
    const char* pattern = "^\\+?[0-9]{1,3}?[-. ]?(\\([0-9]{3}\\)|[0-9]{3})[-. ]?[0-9]{3}[-. ]?[0-9]{4}$";
    return match_regex(pattern, s);
}

int is_valid_username(const char* s) {
    if (s == NULL) return 0;
    const char* pattern = "^[A-Za-z][A-Za-z0-9_]{2,15}$";
    return match_regex(pattern, s);
}

int main(void) {
    const char* emails[5] = {
        "user@example.com",
        "foo.bar+tag@sub.domain.co",
        "bad@domain",
        "noatsymbol.com",
        "user@domain.c"
    };
    const char* phones[5] = {
        "+1 (555) 123-4567",
        "555-123-4567",
        "5551234567",
        "123-45-6789",
        "++1 555 123 4567"
    };
    const char* usernames[5] = {
        "Alice_123",
        "a",
        "1start",
        "Good_Name",
        "ThisUsernameIsWayTooLong123"
    };

    printf("Email validations:\n");
    for (int i = 0; i < 5; ++i) {
        printf("%s -> %s\n", emails[i], is_valid_email(emails[i]) ? "true" : "false");
    }

    printf("\nPhone validations:\n");
    for (int i = 0; i < 5; ++i) {
        printf("%s -> %s\n", phones[i], is_valid_phone(phones[i]) ? "true" : "false");
    }

    printf("\nUsername validations:\n");
    for (int i = 0; i < 5; ++i) {
        printf("%s -> %s\n", usernames[i], is_valid_username(usernames[i]) ? "true" : "false");
    }

    return 0;
}