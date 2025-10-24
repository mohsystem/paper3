#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct User {
    char name[128];
    int age;
    char email[128];
};

static void str_trim(char *s) {
    if (!s) return;
    size_t len = strlen(s);
    size_t start = 0;
    while (start < len && isspace((unsigned char)s[start])) start++;
    size_t end = len;
    while (end > start && isspace((unsigned char)s[end - 1])) end--;
    if (start > 0 || end < len) {
        memmove(s, s + start, end - start);
        s[end - start] = '\0';
    }
}

static char *str_to_lower(char *s) {
    for (char *p = s; *p; ++p) *p = (char)tolower((unsigned char)*p);
    return s;
}

int deserialize_user(const char *input, struct User *out_user) {
    if (!out_user) return 0;
    out_user->name[0] = '\0';
    out_user->email[0] = '\0';
    out_user->age = -1;

    if (!input) return 1;

    size_t in_len = strlen(input);
    char *buf = (char *)malloc(in_len + 1);
    if (!buf) return 0;
    memcpy(buf, input, in_len + 1);

    char *token = strtok(buf, ";");
    while (token) {
        char *eq = strchr(token, '=');
        if (eq) {
            size_t key_len = (size_t)(eq - token);
            char *key = (char *)malloc(key_len + 1);
            if (!key) { free(buf); return 0; }
            memcpy(key, token, key_len);
            key[key_len] = '\0';

            char *value = strdup(eq + 1);
            if (!value) { free(key); free(buf); return 0; }

            str_trim(key);
            str_trim(value);
            str_to_lower(key);

            if (strcmp(key, "name") == 0) {
                strncpy(out_user->name, value, sizeof(out_user->name) - 1);
                out_user->name[sizeof(out_user->name) - 1] = '\0';
            } else if (strcmp(key, "age") == 0) {
                char *endptr = NULL;
                long v = strtol(value, &endptr, 10);
                if (endptr != value && *endptr == '\0') {
                    out_user->age = (int)v;
                } else {
                    out_user->age = -1;
                }
            } else if (strcmp(key, "email") == 0) {
                strncpy(out_user->email, value, sizeof(out_user->email) - 1);
                out_user->email[sizeof(out_user->email) - 1] = '\0';
            } else {
                // ignore unknown
            }

            free(key);
            free(value);
        }
        token = strtok(NULL, ";");
    }

    free(buf);
    return 1;
}

int main(void) {
    const char *tests[5] = {
        "name=Alice; age=30; email=alice@example.com",
        "age=25;name=Bob;email=bob@domain.org",
        "name=Charlie;email=charlie@mail.net",
        "name = Dana ; age = notanumber ; email = dana@x.y",
        "email=eve@ex.com; name=Eve; extra=ignored; age= 42 ;"
    };

    for (int i = 0; i < 5; ++i) {
        struct User u;
        if (deserialize_user(tests[i], &u)) {
            printf("Input: %s\n", tests[i]);
            printf("Deserialized: User{name='%s', age=%d, email='%s'}\n", u.name, u.age, u.email);
            printf("---\n");
        } else {
            printf("Failed to deserialize: %s\n", tests[i]);
        }
    }
    return 0;
}