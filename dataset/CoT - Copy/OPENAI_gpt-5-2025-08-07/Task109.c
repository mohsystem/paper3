/* Chain-of-Through process in code generation:
 * 1) Problem understanding: validate user inputs (email, phone, username, password, URL) using POSIX regex where applicable.
 * 2) Security requirements: anchor patterns, bound lengths, avoid dangerous constructs.
 * 3) Secure coding generation: length checks, POSIX ERE regexes, supplemental checks without backreferences.
 * 4) Code review: anchored patterns, REG_NOSUB, freed regex, safe printing.
 * 5) Secure code output: final code includes 5 test cases in main.
 */
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <stdbool.h>

#define MAX_EMAIL_LEN 320
#define MAX_PHONE_LEN 16
#define MIN_USERNAME_LEN 2
#define MAX_USERNAME_LEN 30
#define MIN_PASSWORD_LEN 8
#define MAX_PASSWORD_LEN 64
#define MAX_URL_LEN 2083

static int regex_match_full(const char *pattern, const char *input) {
    regex_t re;
    int rc = regcomp(&re, pattern, REG_EXTENDED | REG_NOSUB);
    if (rc != 0) {
        return 0;
    }
    rc = regexec(&re, input, 0, NULL, 0);
    regfree(&re);
    return rc == 0;
}

static int regex_search_any(const char *pattern, const char *input) {
    regex_t re;
    int rc = regcomp(&re, pattern, REG_EXTENDED);
    if (rc != 0) {
        return 0;
    }
    rc = regexec(&re, input, 0, NULL, 0);
    regfree(&re);
    return rc == 0;
}

static bool within_len(const char *s, size_t minLen, size_t maxLen) {
    if (s == NULL) return false;
    size_t n = strlen(s);
    return n >= minLen && n <= maxLen;
}

bool validateEmail(const char *input) {
    if (!within_len(input, 3, MAX_EMAIL_LEN)) return false;
    // Email: local part (no leading/trailing dot), domain with labels (no leading/trailing hyphen), TLD 2..63
    const char *EMAIL_RX = "^[A-Za-z0-9]([A-Za-z0-9._%+-]{0,62}[A-Za-z0-9])?@([A-Za-z0-9]([A-Za-z0-9-]{0,61}[A-Za-z0-9])?\\.)+[A-Za-z]{2,63}$";
    if (!regex_match_full(EMAIL_RX, input)) return false;

    // Additional check: no consecutive dots in local part or domain
    const char *at = strchr(input, '@');
    if (at == NULL || at == input || *(at + 1) == '\0') return false;
    // local
    for (const char *p = input; p < at - 1; ++p) {
        if (*p == '.' && *(p + 1) == '.') return false;
    }
    // domain
    for (const char *p = at + 1; *p && *(p + 1); ++p) {
        if (*p == '.' && *(p + 1) == '.') return false;
    }
    return true;
}

bool validatePhoneE164(const char *input) {
    if (!within_len(input, 2, MAX_PHONE_LEN)) return false;
    const char *PHONE_RX = "^\\+?[1-9][0-9]{1,14}$";
    return regex_match_full(PHONE_RX, input);
}

bool validateUsername(const char *input) {
    if (!within_len(input, MIN_USERNAME_LEN, MAX_USERNAME_LEN)) return false;
    const char *USERNAME_RX = "^[A-Za-z0-9]([A-Za-z0-9._-]{0,28}[A-Za-z0-9])?$";
    if (!regex_match_full(USERNAME_RX, input)) return false;
    // Disallow consecutive special characters among . _ -
    for (size_t i = 1; input[i] != '\0'; ++i) {
        char a = input[i - 1], b = input[i];
        int aSpec = (a == '.' || a == '_' || a == '-');
        int bSpec = (b == '.' || b == '_' || b == '-');
        if (aSpec && bSpec) return false;
    }
    return true;
}

bool validateStrongPassword(const char *input) {
    if (!within_len(input, MIN_PASSWORD_LEN, MAX_PASSWORD_LEN)) return false;
    // Reject any whitespace
    if (regex_search_any("[[:space:]]", input)) return false;
    // Require at least one of each class
    if (!regex_search_any("[a-z]", input)) return false;
    if (!regex_search_any("[A-Z]", input)) return false;
    if (!regex_search_any("[0-9]", input)) return false;
    if (!regex_search_any("[^[:alnum:][:space:]]", input)) return false;
    return true;
}

bool validateURL(const char *input) {
    if (!within_len(input, 8, MAX_URL_LEN)) return false; // minimal http://
    // Scheme http/https, domain labels with constraints, optional port, optional path with safe chars
    const char *URL_RX = "^https?://([A-Za-z0-9]([A-Za-z0-9-]{0,61}[A-Za-z0-9])?\\.)+[A-Za-z]{2,63}(:[0-9]{2,5})?(/[A-Za-z0-9._~!$&'()*+,;=:@%/\\-?]*)?$";
    return regex_match_full(URL_RX, input);
}

int main(void) {
    const char *emails[5] = {
        "user@example.com",
        "first.last+alias@sub.domain.co",
        ".user@domain.com",
        "user..name@domain.com",
        "user@-domain.com"
    };
    const char *phones[5] = {
        "+14155552671",
        "4155552671",
        "+001234",
        "++123",
        "+1234567890123456"
    };
    const char *usernames[5] = {
        "alice_01",
        "a",
        "user--name",
        "_user",
        "valid.user-name"
    };
    const char *passwords[5] = {
        "Abcdef1!",
        "short1!",
        "NoDigits!",
        "noupper1!",
        "With Space1!"
    };
    const char *urls[5] = {
        "https://example.com",
        "http://sub.domain.co:8080/path/to/resource?x=1",
        "ftp://example.com",
        "http://-bad.com",
        "http://example"
    };

    for (int i = 0; i < 5; ++i) {
        printf("Email: %s -> %s\n", emails[i], validateEmail(emails[i]) ? "1" : "0");
    }
    for (int i = 0; i < 5; ++i) {
        printf("Phone: %s -> %s\n", phones[i], validatePhoneE164(phones[i]) ? "1" : "0");
    }
    for (int i = 0; i < 5; ++i) {
        printf("Username: %s -> %s\n", usernames[i], validateUsername(usernames[i]) ? "1" : "0");
    }
    for (int i = 0; i < 5; ++i) {
        printf("Password: %s -> %s\n", passwords[i], validateStrongPassword(passwords[i]) ? "1" : "0");
    }
    for (int i = 0; i < 5; ++i) {
        printf("URL: %s -> %s\n", urls[i], validateURL(urls[i]) ? "1" : "0");
    }

    return 0;
}