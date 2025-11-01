#include <stdio.h>
#include <string.h>

static const char* DATABASE[] = {
    "alice@example.com",
    "bob@fail.com",
    "carol@example.com",
    "dave@example.com",
    "eve@example.org"
};
static const int DATABASE_SIZE = sizeof(DATABASE) / sizeof(DATABASE[0]);

static int unsubscribe_script(const char* email) {
    if (email == NULL) return 1;
    const char* at = strrchr(email, '@');
    const char* domain = (at != NULL) ? (at + 1) : "";
    if (domain[0] == '\0') return 1;
    // Compare case-insensitively to "fail.com"
    const char* fail = "fail.com";
    size_t i = 0;
    while (domain[i] && fail[i]) {
        char a = domain[i];
        char b = fail[i];
        if (a >= 'A' && a <= 'Z') a = (char)(a - 'A' + 'a');
        if (b >= 'A' && b <= 'Z') b = (char)(b - 'A' + 'a');
        if (a != b) break;
        i++;
    }
    if (domain[i] == '\0' && fail[i] == '\0') {
        return 2; // simulate failure for fail.com
    }
    return 0; // success otherwise
}

int check_and_unsubscribe(const char* email) {
    if (email == NULL) return 0;
    int found = 0;
    for (int i = 0; i < DATABASE_SIZE; ++i) {
        if (strcmp(DATABASE[i], email) == 0) {
            found = 1;
            break;
        }
    }
    if (!found) return 0;
    int rc = unsubscribe_script(email);
    return (rc == 0) ? 1 : 0;
}

int main(void) {
    const char* tests[5] = {
        "alice@example.com",    // exists, script success -> 1
        "bob@fail.com",         // exists, script failure -> 0
        "notfound@example.com", // not in DB -> 0
        "carol@example.com",    // exists, script success -> 1
        "unknown@fail.com"      // not in DB -> 0
    };
    for (int i = 0; i < 5; ++i) {
        int result = check_and_unsubscribe(tests[i]);
        printf("%s -> %d\n", tests[i], result);
    }
    return 0;
}