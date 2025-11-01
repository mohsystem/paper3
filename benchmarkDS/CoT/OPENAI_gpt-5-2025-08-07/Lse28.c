#include <stdio.h>
#include <string.h>
#include <regex.h>

// 1) Problem understanding:
// - Compile regex once when module is loaded.
// - Validate target URL against regex.
// - If match: use target; else: use default.
// - Do not use regex to construct redirect URL.

// 2) Security requirements:
// - Regex is constant and compiled once.
// - Cap input length.
// - Provide safe default if needed.

#define MAX_URL_LENGTH 2048

static regex_t g_allowed_pattern;
static int g_regex_ready = 0;

static const char* FALLBACK_DEFAULT = "https://example.com/";

// Compile once at module load (GCC/Clang)
#if defined(__GNUC__)
__attribute__((constructor))
#endif
static void init_allowed_pattern(void) {
    const char *pat = "^https://(www\\.)?example\\.com(/[A-Za-z0-9._~!$&'()*+,;=:@%/-]*)?(\\?[A-Za-z0-9._~!$&'()*+,;=:@%/?-]*)?(#[A-Za-z0-9._~!$&'()*+,;=:@%/?-]*)?$";
    if (regcomp(&g_allowed_pattern, pat, REG_EXTENDED | REG_ICASE | REG_NOSUB) == 0) {
        g_regex_ready = 1;
    } else {
        g_regex_ready = 0;
    }
}

#if defined(__GNUC__)
__attribute__((destructor))
#endif
static void free_allowed_pattern(void) {
    if (g_regex_ready) {
        regfree(&g_allowed_pattern);
        g_regex_ready = 0;
    }
}

// 3) Secure coding generation:
const char* determine_redirect(const char* target_url, const char* default_url) {
    const char* safe_default = (default_url && default_url[0] != '\0') ? default_url : FALLBACK_DEFAULT;

    if (target_url == NULL) {
        return safe_default;
    }

    size_t len = strnlen(target_url, MAX_URL_LENGTH + 1);
    if (len == 0 || len > MAX_URL_LENGTH) {
        return safe_default;
    }

    if (!g_regex_ready) {
        return safe_default;
    }

    int rc = regexec(&g_allowed_pattern, target_url, 0, NULL, 0);
    if (rc == 0) {
        return target_url;
    }
    return safe_default;
}

// 4) Code review and 5) Secure code output: tests
int main(void) {
    const char* default_url = "https://safe.example.org/home";
    const char* tests[5] = {
        "https://example.com/welcome",
        "https://www.example.com/path?item=42#section",
        "https://evil.com/phish",
        "http://example.com/insecure",
        ""
    };

    for (int i = 0; i < 5; ++i) {
        const char* result = determine_redirect(tests[i], default_url);
        printf("Test %d: %s\n", i + 1, result);
    }
    return 0;
}