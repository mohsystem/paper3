#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MIN_LENGTH 12
#define MAX_LENGTH 128

int isValidAdminPassword(const char* password);

static int is_common_password(const char* lower) {
    const char* common[] = {
        "password","123456","qwerty","letmein","welcome","admin","administrator","passw0rd",
        "iloveyou","monkey","dragon","football","abc123","123456789","12345678","111111",
        "123123","login","princess","admin123","adminadmin","root","root123"
    };
    size_t count = sizeof(common) / sizeof(common[0]);
    for (size_t i = 0; i < count; ++i) {
        if (strcmp(lower, common[i]) == 0) return 1;
    }
    return 0;
}

static int has_triple_repeat(const char* s) {
    int cnt = 1;
    for (size_t i = 1; s[i] != '\0'; ++i) {
        if (s[i] == s[i-1]) {
            cnt++;
            if (cnt >= 3) return 1;
        } else cnt = 1;
    }
    return 0;
}

static int has_sequential_pattern(const char* s) {
    size_t n = strlen(s);
    if (n < 4) return 0;
    // Create lowercase copy
    char buf[MAX_LENGTH + 1];
    if (n > MAX_LENGTH) n = MAX_LENGTH;
    for (size_t i = 0; i < n; ++i) buf[i] = (char)tolower((unsigned char)s[i]);
    buf[n] = '\0';

    for (size_t i = 0; i + 3 < n; ++i) {
        char a = buf[i], b = buf[i+1], c = buf[i+2], d = buf[i+3];
        // letters
        if (a >= 'a' && a <= 'z' && b >= 'a' && b <= 'z' && c >= 'a' && c <= 'z' && d >= 'a' && d <= 'z') {
            int asc = (b == a + 1) && (c == b + 1) && (d == c + 1);
            int desc = (b == a - 1) && (c == b - 1) && (d == c - 1);
            if (asc || desc) return 1;
        }
        // digits
        if (isdigit((unsigned char)a) && isdigit((unsigned char)b) &&
            isdigit((unsigned char)c) && isdigit((unsigned char)d)) {
            int asc = (b == a + 1) && (c == b + 1) && (d == c + 1);
            int desc = (b == a - 1) && (c == b - 1) && (d == c - 1);
            if (asc || desc) return 1;
        }
    }
    return 0;
}

int isValidAdminPassword(const char* password) {
    if (password == NULL) return 0;
    size_t n = strnlen(password, MAX_LENGTH + 1);
    if (n < MIN_LENGTH || n > MAX_LENGTH) return 0;

    // lowercase copy
    char lower[MAX_LENGTH + 1];
    for (size_t i = 0; i < n; ++i) lower[i] = (char)tolower((unsigned char)password[i]);
    lower[n] = '\0';

    if (is_common_password(lower)) return 0;
    if (strstr(lower, "admin") != NULL || strstr(lower, "administrator") != NULL || strstr(lower, "root") != NULL)
        return 0;

    int hasUpper = 0, hasLower = 0, hasDigit = 0, hasSpecial = 0;
    for (size_t i = 0; i < n; ++i) {
        unsigned char ch = (unsigned char)password[i];
        if (isspace(ch)) return 0;
        if (isupper(ch)) hasUpper = 1;
        else if (islower(ch)) hasLower = 1;
        else if (isdigit(ch)) hasDigit = 1;
        else hasSpecial = 1;
    }
    if (!(hasUpper && hasLower && hasDigit && hasSpecial)) return 0;

    if (has_triple_repeat(password)) return 0;
    if (has_sequential_pattern(password)) return 0;

    return 1;
}

int main(void) {
    const char* tests[5] = {
        "password",          // false
        "Admin123!",         // false
        "StrongPassw0rd!",   // true
        "Adm!nStrong#2024",  // true
        "Abcd1234!xyZ"       // false
    };
    for (int i = 0; i < 5; ++i) {
        printf("%s\n", isValidAdminPassword(tests[i]) ? "true" : "false");
    }
    return 0;
}