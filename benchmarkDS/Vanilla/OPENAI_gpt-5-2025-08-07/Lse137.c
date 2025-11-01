#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int auth_mod(const char* username, const char* password, const char* session) {
    if (!username || !password || !session) return 0;

    const char* expectedPass = NULL;
    if (strcmp(username, "alice") == 0) expectedPass = "alice123";
    else if (strcmp(username, "bob") == 0) expectedPass = "bob123";
    else if (strcmp(username, "carol") == 0) expectedPass = "carol123";
    if (!expectedPass) return 0;
    if (strcmp(password, expectedPass) != 0) return 0;
    if (strcmp(session, "sessOK") != 0) return 0;
    return (strcmp(username, "alice") == 0 || strcmp(username, "bob") == 0) ? 1 : 0;
}

static char* get_param_value(const char* url, const char* key) {
    if (!url || !key) return NULL;
    const char* q = strchr(url, '?');
    if (!q || *(q + 1) == '\0') return NULL;

    char* qs = strdup(q + 1);
    if (!qs) return NULL;

    size_t keylen = strlen(key);
    char* saveptr = NULL;
    char* pair = strtok_r(qs, "&", &saveptr);
    while (pair) {
        if (strncmp(pair, key, keylen) == 0 && pair[keylen] == '=') {
            const char* val = pair + keylen + 1;
            char* out = (char*)malloc(strlen(val) + 1);
            if (out) strcpy(out, val);
            free(qs);
            return out;
        }
        pair = strtok_r(NULL, "&", &saveptr);
    }
    free(qs);
    return NULL;
}

int check_mod(const char* url) {
    char* username = get_param_value(url, "username");
    char* password = get_param_value(url, "password");
    char* session = get_param_value(url, "session");

    int result = auth_mod(username, password, session);

    if (username) free(username);
    if (password) free(password);
    if (session) free(session);

    return result;
}

int main(void) {
    const char* tests[5] = {
        "/check_mod?username=alice&password=alice123&session=sessOK",
        "/check_mod?username=bob&password=bob123&session=sessOK",
        "/check_mod?username=carol&password=carol123&session=sessOK",
        "/check_mod?username=alice&password=wrong&session=sessOK",
        "/check_mod?username=alice&password=alice123&session=bad"
    };
    for (int i = 0; i < 5; ++i) {
        printf("%s\n", check_mod(tests[i]) ? "true" : "false");
    }
    return 0;
}