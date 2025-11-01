#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>

#define MAX_TARGET_LENGTH 2048

static void trim(char *s) {
    if (s == NULL) return;
    size_t len = strlen(s);
    size_t start = 0;
    while (start < len && (unsigned char)s[start] <= ' ') start++;
    size_t end = len;
    while (end > start && (unsigned char)s[end - 1] <= ' ') end--;
    if (start > 0 || end < len) {
        memmove(s, s + start, end - start);
        s[end - start] = '\0';
    }
}

char* get_redirect_url(const char* target_param) {
    const char* fallback = "/";
    if (target_param == NULL) {
        return strdup(fallback);
    }

    size_t in_len = strnlen(target_param, MAX_TARGET_LENGTH + 1);
    if (in_len == 0 || in_len > MAX_TARGET_LENGTH) {
        return strdup(fallback);
    }

    char *buf = (char*)malloc(in_len + 1);
    if (!buf) {
        return strdup(fallback);
    }
    memcpy(buf, target_param, in_len);
    buf[in_len] = '\0';

    trim(buf);

    if (buf[0] == '\0') {
        free(buf);
        return strdup(fallback);
    }

    if (strchr(buf, '\r') != NULL || strchr(buf, '\n') != NULL) {
        free(buf);
        return strdup(fallback);
    }

    regex_t regex;
    int rc = regcomp(&regex, "^[A-Za-z0-9-]+\\.", REG_EXTENDED);
    if (rc != 0) {
        free(buf);
        return strdup(fallback);
    }

    int match = regexec(&regex, buf, 0, NULL, 0) == 0;
    regfree(&regex);

    if (match) {
        // return buf as is
        return buf;
    } else {
        free(buf);
        return strdup(fallback);
    }
}

int main(void) {
    const char* tests[5] = {
        "example.com/home",
        "sub-domain.example",
        "/internal/path",
        "http://evil.com",
        "bad..com"
    };

    for (int i = 0; i < 5; i++) {
        char* result = get_redirect_url(tests[i]);
        printf("Input: %s => Redirect: %s\n", tests[i], result);
        free(result);
    }
    return 0;
}