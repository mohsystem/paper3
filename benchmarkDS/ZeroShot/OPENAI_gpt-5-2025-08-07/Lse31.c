#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static int contains_ctl(const char* s) {
    if (!s) return 1;
    for (const unsigned char* p = (const unsigned char*)s; *p; ++p) {
        if (*p < 0x20 || *p == 0x7F) return 1;
    }
    return 0;
}

static char* str_trim_dup(const char* s) {
    if (!s) return NULL;
    const char* start = s;
    while (*start && isspace((unsigned char)*start)) start++;
    if (*start == '\0') {
        char* out = (char*)malloc(1);
        if (out) out[0] = '\0';
        return out;
    }
    const char* end = s + strlen(s) - 1;
    while (end > start && isspace((unsigned char)*end)) end--;
    size_t len = (size_t)(end - start + 1);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, start, len);
    out[len] = '\0';
    return out;
}

static void to_lower_inplace(char* s) {
    if (!s) return;
    for (; *s; ++s) *s = (char)tolower((unsigned char)*s);
}

static int is_http_or_https(const char* s, size_t len) {
    if (len == 4 && strncasecmp(s, "http", 4) == 0) return 1;
    if (len == 5 && strncasecmp(s, "https", 5) == 0) return 1;
    return 0;
}

char* redirect_url(const char* url) {
    const char* not_found = "404 Not Found";
    const char* redirect_prefix = "Redirected to: ";
    if (!url) {
        char* r = (char*)malloc(strlen(not_found) + 1);
        if (r) strcpy(r, not_found);
        return r;
    }

    char* trimmed = str_trim_dup(url);
    if (!trimmed || trimmed[0] == '\0' || contains_ctl(trimmed)) {
        free(trimmed);
        char* r = (char*)malloc(strlen(not_found) + 1);
        if (r) strcpy(r, not_found);
        return r;
    }

    int needs_scheme = strstr(trimmed, "://") == NULL;
    size_t new_len = strlen(trimmed) + (needs_scheme ? 7 : 0); // "http://"
    char* u = (char*)malloc(new_len + 1);
    if (!u) {
        free(trimmed);
        char* r = (char*)malloc(strlen(not_found) + 1);
        if (r) strcpy(r, not_found);
        return r;
    }
    if (needs_scheme) {
        strcpy(u, "http://");
        strcat(u, trimmed);
    } else {
        strcpy(u, trimmed);
    }
    free(trimmed);

    char* scheme_pos = strstr(u, "://");
    if (!scheme_pos) {
        free(u);
        char* r = (char*)malloc(strlen(not_found) + 1);
        if (r) strcpy(r, not_found);
        return r;
    }
    size_t scheme_len = (size_t)(scheme_pos - u);
    if (!is_http_or_https(u, scheme_len)) {
        free(u);
        char* r = (char*)malloc(strlen(not_found) + 1);
        if (r) strcpy(r, not_found);
        return r;
    }

    char* auth_start = scheme_pos + 3;
    if (*auth_start == '\0') {
        free(u);
        char* r = (char*)malloc(strlen(not_found) + 1);
        if (r) strcpy(r, not_found);
        return r;
    }
    char* auth_end = strpbrk(auth_start, "/?#");
    if (!auth_end) auth_end = u + strlen(u);

    size_t auth_len = (size_t)(auth_end - auth_start);
    char* authority = (char*)malloc(auth_len + 1);
    if (!authority) {
        free(u);
        char* r = (char*)malloc(strlen(not_found) + 1);
        if (r) strcpy(r, not_found);
        return r;
    }
    memcpy(authority, auth_start, auth_len);
    authority[auth_len] = '\0';

    // Remove userinfo (last '@')
    char* at = strrchr(authority, '@');
    char* hostport = authority;
    if (at) hostport = at + 1;

    char* host = NULL;
    if (hostport[0] == '[') {
        char* rbr = strchr(hostport, ']');
        if (!rbr) {
            free(authority);
            free(u);
            char* r = (char*)malloc(strlen(not_found) + 1);
            if (r) strcpy(r, not_found);
            return r;
        }
        size_t hlen = (size_t)(rbr - hostport - 1);
        host = (char*)malloc(hlen + 1);
        if (!host) {
            free(authority);
            free(u);
            char* r = (char*)malloc(strlen(not_found) + 1);
            if (r) strcpy(r, not_found);
            return r;
        }
        memcpy(host, hostport + 1, hlen);
        host[hlen] = '\0';
    } else {
        char* colon = strchr(hostport, ':');
        size_t hlen = colon ? (size_t)(colon - hostport) : strlen(hostport);
        host = (char*)malloc(hlen + 1);
        if (!host) {
            free(authority);
            free(u);
            char* r = (char*)malloc(strlen(not_found) + 1);
            if (r) strcpy(r, not_found);
            return r;
        }
        memcpy(host, hostport, hlen);
        host[hlen] = '\0';
    }

    to_lower_inplace(host);

    char* result;
    if (strcmp(host, "example.com") == 0) {
        size_t out_len = strlen(redirect_prefix) + strlen(u);
        result = (char*)malloc(out_len + 1);
        if (result) {
            strcpy(result, redirect_prefix);
            strcat(result, u);
        }
    } else {
        result = (char*)malloc(strlen(not_found) + 1);
        if (result) strcpy(result, not_found);
    }

    free(host);
    free(authority);
    free(u);
    return result;
}

int main(void) {
    const char* tests[5] = {
        "http://example.com/path",
        "https://EXAMPLE.com",
        "http://sub.example.com",
        "http://example.com@evil.com",
        "example.com"
    };
    for (int i = 0; i < 5; ++i) {
        char* res = redirect_url(tests[i]);
        if (res) {
            printf("%s\n", res);
            free(res);
        }
    }
    return 0;
}