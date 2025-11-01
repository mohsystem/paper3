#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static int hexVal(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static void url_decode(const char* src, char* dest) {
    size_t i = 0, j = 0;
    size_t n = strlen(src);
    while (i < n) {
        char c = src[i];
        if (c == '+') {
            dest[j++] = ' ';
        } else if (c == '%' && i + 2 < n) {
            int hi = hexVal(src[i+1]);
            int lo = hexVal(src[i+2]);
            if (hi >= 0 && lo >= 0) {
                dest[j++] = (char)((hi << 4) | lo);
                i += 2;
            } else {
                dest[j++] = c;
            }
        } else {
            dest[j++] = c;
        }
        i++;
    }
    dest[j] = '\0';
}

const char* check_mod(const char* url) {
    const char* CORRECT_USER = "admin";
    const char* CORRECT_PASS = "secret";

    const char* path_start = NULL;
    const char* scheme = strstr(url, "://");
    if (scheme) {
        const char* slash = strchr(scheme + 3, '/');
        path_start = slash ? slash : "/";
    } else {
        path_start = url;
    }

    const char* qmark = strchr(path_start, '?');
    size_t path_len = qmark ? (size_t)(qmark - path_start) : strlen(path_start);

    char* path = (char*)malloc(path_len + 1);
    if (!path) return "false";
    memcpy(path, path_start, path_len);
    path[path_len] = '\0';

    const char* query_start = qmark ? (qmark + 1) : NULL;

    const char* result = "false";
    if (strcmp(path, "/check_mod") != 0) {
        free(path);
        return result;
    }

    char* username = NULL;
    char* password = NULL;

    if (query_start) {
        size_t qlen = strlen(query_start);
        char* qbuf = (char*)malloc(qlen + 1);
        if (!qbuf) {
            free(path);
            return "false";
        }
        memcpy(qbuf, query_start, qlen + 1);

        char* saveptr = NULL;
        char* pair = strtok_r(qbuf, "&", &saveptr);
        while (pair) {
            char* eq = strchr(pair, '=');
            char* key = NULL;
            char* val = NULL;
            if (eq) {
                *eq = '\0';
                key = pair;
                val = eq + 1;
            } else {
                key = pair;
                val = "";
            }
            // Decode
            char* dkey = (char*)malloc(strlen(key) + 1);
            char* dval = (char*)malloc(strlen(val) + 1);
            if (dkey && dval) {
                url_decode(key, dkey);
                url_decode(val, dval);
                if (strcmp(dkey, "username") == 0) {
                    free(username);
                    username = strdup(dval);
                } else if (strcmp(dkey, "password") == 0) {
                    free(password);
                    password = strdup(dval);
                }
            }
            free(dkey);
            free(dval);
            pair = strtok_r(NULL, "&", &saveptr);
        }
        free(qbuf);
    }

    if (username && password && strcmp(username, CORRECT_USER) == 0 && strcmp(password, CORRECT_PASS) == 0) {
        result = "true";
    } else {
        result = "false";
    }

    free(username);
    free(password);
    free(path);
    return result;
}

int main(void) {
    const char* tests[] = {
        "/check_mod?username=admin&password=secret",
        "/check_mod?username=admin&password=wrong",
        "/other?username=admin&password=secret",
        "http://example.com/check_mod?username=admin&password=sec%72et",
        "/check_mod?password=secret&username=admin"
    };
    for (int i = 0; i < 5; ++i) {
        printf("%s\n", check_mod(tests[i]));
    }
    return 0;
}