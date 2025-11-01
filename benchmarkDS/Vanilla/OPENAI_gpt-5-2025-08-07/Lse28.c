#include <stdio.h>
#include <regex.h>

// Compile the regular expression once when the module is loaded (GCC/Clang)
static regex_t ALLOWED_URL;
static int ALLOWED_URL_COMPILED = 0;

__attribute__((constructor))
static void compile_regex(void) {
    if (!ALLOWED_URL_COMPILED) {
        // ^https?://(www\.)?example\.com(/.*)?$
        int rc = regcomp(&ALLOWED_URL, "^(https?)://(www\\.)?example\\.com(/.*)?$", REG_EXTENDED | REG_ICASE);
        if (rc == 0) {
            ALLOWED_URL_COMPILED = 1;
        }
    }
}

__attribute__((destructor))
static void free_regex(void) {
    if (ALLOWED_URL_COMPILED) {
        regfree(&ALLOWED_URL);
        ALLOWED_URL_COMPILED = 0;
    }
}

const char* redirect(const char* targetUrl, const char* defaultUrl) {
    if (!targetUrl || !defaultUrl) return defaultUrl;
    if (ALLOWED_URL_COMPILED) {
        int rc = regexec(&ALLOWED_URL, targetUrl, 0, NULL, 0);
        if (rc == 0) {
            return targetUrl;
        }
    }
    return defaultUrl;
}

int main(void) {
    const char* defaultUrl = "https://example.com/default";
    const char* tests[5] = {
        "https://example.com/path?a=1",
        "http://www.example.com",
        "https://evil.com/phish",
        "ftp://example.com/file",
        "https://sub.example.com/page"
    };

    for (int i = 0; i < 5; ++i) {
        const char* res = redirect(tests[i], defaultUrl);
        printf("%s -> %s\n", tests[i], res);
    }
    return 0;
}