#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

static regex_t ALLOWED;
static int REGEX_COMPILED = 0;

static const size_t MAX_LEN = 2048;
static const char* SAFE_FALLBACK = "https://www.example.com/";

/* Compile regex once when module is loaded (GCC/Clang). */
__attribute__((constructor))
static void init_regex(void) {
    const char* pattern = "^https://(www\\.)?example\\.com(/[A-Za-z0-9._~\\-/%?#&=]*)?$";
    if (regcomp(&ALLOWED, pattern, REG_EXTENDED | REG_NOSUB) == 0) {
        REGEX_COMPILED = 1;
    } else {
        REGEX_COMPILED = 0;
    }
}

__attribute__((destructor))
static void free_regex(void) {
    if (REGEX_COMPILED) {
        regfree(&ALLOWED);
        REGEX_COMPILED = 0;
    }
}

static int has_ctl(const char* s) {
    for (const unsigned char* p = (const unsigned char*)s; *p; ++p) {
        if (*p < 0x20 || *p == 0x7F) return 1;
    }
    return 0;
}

static char* sdup(const char* s) {
    size_t n = strlen(s);
    char* out = (char*)malloc(n + 1);
    if (!out) return NULL;
    memcpy(out, s, n + 1);
    return out;
}

static char* sanitize_default(const char* def_url) {
    if (def_url == NULL) return sdup(SAFE_FALLBACK);
    size_t n = strlen(def_url);
    if (n == 0 || n > MAX_LEN) return sdup(SAFE_FALLBACK);
    if (has_ctl(def_url)) return sdup(SAFE_FALLBACK);
    if (strncmp(def_url, "https://", 8) != 0) return sdup(SAFE_FALLBACK);
    return sdup(def_url);
}

char* resolve_redirect(const char* target_url, const char* default_url) {
    char* safe_default = sanitize_default(default_url);
    if (safe_default == NULL) return NULL;

    if (!REGEX_COMPILED) {
        return safe_default;
    }

    if (target_url == NULL) {
        return safe_default;
    }
    size_t n = strlen(target_url);
    if (n == 0 || n > MAX_LEN) {
        return safe_default;
    }
    if (has_ctl(target_url)) {
        return safe_default;
    }

    int match = regexec(&ALLOWED, target_url, 0, NULL, 0);
    if (match == 0) {
        char* res = sdup(target_url);
        if (!res) {
            return safe_default;
        }
        free(safe_default);
        return res;
    }
    return safe_default;
}

int main(void) {
    const char* default_url = "https://www.example.com/safe";
    const char* tests[5] = {
        "https://www.example.com/path?x=1",   /* allowed */
        "https://evil.com/",                  /* wrong domain */
        "http://www.example.com/",            /* wrong scheme */
        NULL,                                 /* null */
        "https://www.example.com/\r\nattack"  /* CRLF injection attempt */
    };

    for (int i = 0; i < 5; ++i) {
        char* result = resolve_redirect(tests[i], default_url);
        if (result) {
            printf("Test %d: %s\n", i + 1, result);
            free(result);
        } else {
            printf("Test %d: <allocation failure>\n", i + 1);
        }
    }
    return 0;
}