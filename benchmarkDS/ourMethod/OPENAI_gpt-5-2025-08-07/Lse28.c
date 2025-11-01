#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#define MAX_URL_LEN 2048

static const char SAFE_DEFAULT[] = "https://www.example.com/";
// POSIX ERE (no non-capturing groups). Case-insensitive via REG_ICASE.
static const char ALLOW_REGEX[] = "^https://([A-Za-z0-9-]+\\.)*example\\.com(:[0-9]{1,5})?(/.*)?$";

static regex_t g_allow_re;
static int g_regex_initialized = 0;

static void compile_once(void) {
    if (!g_regex_initialized) {
        if (regcomp(&g_allow_re, ALLOW_REGEX, REG_EXTENDED | REG_NOSUB | REG_ICASE) != 0) {
            // If compilation fails, keep it uninitialized; all checks will fail closed.
            g_regex_initialized = -1;
        } else {
            g_regex_initialized = 1;
        }
    }
}

static int is_allowed(const char *url) {
    compile_once();
    if (g_regex_initialized != 1) return 0;
    if (url == NULL) return 0;
    size_t len = strnlen(url, MAX_URL_LEN + 1);
    if (len == 0 || len > MAX_URL_LEN) return 0;
    for (size_t i = 0; i < len; i++) {
        if (url[i] == '\r' || url[i] == '\n') return 0;
    }
    int rc = regexec(&g_allow_re, url, 0, NULL, 0);
    return rc == 0 ? 1 : 0;
}

// Returns a newly allocated string: either targetUrl if allowed, else a safe default.
// Caller must free the returned string with free().
char* resolve_redirect(const char *targetUrl, const char *defaultUrl) {
    const char *fallback = is_allowed(defaultUrl) ? defaultUrl : SAFE_DEFAULT;
    const char *chosen = is_allowed(targetUrl) ? targetUrl : fallback;

    size_t len = strnlen(chosen, MAX_URL_LEN + 1);
    if (len > MAX_URL_LEN) {
        chosen = SAFE_DEFAULT;
        len = strlen(SAFE_DEFAULT);
    }
    char *out = (char*)malloc(len + 1);
    if (!out) {
        // Allocation failure: return a duplicate of SAFE_DEFAULT minimalistically
        size_t dlen = strlen(SAFE_DEFAULT);
        char *d = (char*)malloc(dlen + 1);
        if (!d) return NULL;
        memcpy(d, SAFE_DEFAULT, dlen + 1);
        return d;
    }
    memcpy(out, chosen, len);
    out[len] = '\0';
    return out;
}

int main(void) {
    const char *tests[5][2] = {
        {"https://example.com/path", "https://www.example.com/home"},
        {"https://sub.example.com/", "https://www.example.com/fallback"},
        {"https://evil.com/", "https://www.example.com/fallback"},
        {"http://example.com/", "https://www.example.com/fallback"},
        {NULL, "http://insecure.example.com/"} // NULL target; invalid default
    };

    for (int i = 0; i < 5; i++) {
        const char *t = tests[i][0];
        const char *d = tests[i][1];
        char *res = resolve_redirect(t, d);
        printf("Test %d:\n", i + 1);
        printf("  target = %s\n", t ? (strlen(t) <= 80 ? t : "(too long)") : "(null)");
        printf("  default= %s\n", d ? d : "(null)");
        printf("  result = %s\n", res ? res : "(null)");
        free(res);
    }

    if (g_regex_initialized == 1) {
        regfree(&g_allow_re);
    }
    return 0;
}