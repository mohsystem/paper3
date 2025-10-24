#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static size_t safe_strnlen_(const char *s, size_t max) {
    size_t i = 0;
    if (s == NULL) return 0;
    while (i < max && s[i] != '\0') i++;
    return i;
}

static char* make_copy(const char* s) {
    if (s == NULL) return NULL;
    size_t n = strlen(s);
    char *out = (char*)malloc(n + 1);
    if (!out) return NULL;
    memcpy(out, s, n + 1);
    return out;
}

static void to_lower_ascii_inplace(char *s) {
    if (!s) return;
    for (; *s; ++s) {
        *s = (char)tolower((unsigned char)*s);
    }
}

static int is_safe_username(const char *u) {
    if (u == NULL) return 0;
    size_t len = strlen(u);
    if (len < 1 || len > 30) return 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)u[i];
        if (!(isalnum(c) || c == '_')) return 0;
    }
    return 1;
}

char* resolve_redirect(const char *input) {
    if (input == NULL) {
        return make_copy("/error?code=bad-request");
    }

    size_t len = safe_strnlen_(input, 1024);
    if (len >= 1024) {
        return make_copy("/error?code=too-long");
    }

    // Trim leading/trailing whitespace
    size_t start = 0;
    size_t end = len;
    while (start < end && isspace((unsigned char)input[start])) start++;
    while (end > start && isspace((unsigned char)input[end - 1])) end--;

    if (start == end) {
        return make_copy("/error?code=bad-request");
    }

    size_t tlen = end - start;
    if (tlen > 100) {
        return make_copy("/error?code=too-long");
    }

    char trimmed[101];
    memcpy(trimmed, input + start, tlen);
    trimmed[tlen] = '\0';

    // Find colon
    size_t colon_pos = (size_t)-1;
    for (size_t i = 0; i < tlen; i++) {
        if (trimmed[i] == ':') {
            colon_pos = i;
            break;
        }
    }

    if (colon_pos != (size_t)-1) {
        // Split into head and tail
        size_t head_len = colon_pos;
        if (head_len > 50) { // arbitrary sane bound for command part
            return make_copy("/error?code=unknown-page");
        }
        char head[51];
        memcpy(head, trimmed, head_len);
        head[head_len] = '\0';
        to_lower_ascii_inplace(head);

        const char *tail_raw = trimmed + colon_pos + 1;

        // Trim spaces around tail
        size_t tail_len_full = strlen(tail_raw);
        size_t ts = 0, te = tail_len_full;
        while (ts < te && isspace((unsigned char)tail_raw[ts])) ts++;
        while (te > ts && isspace((unsigned char)tail_raw[te - 1])) te--;
        size_t tail_len = te - ts;

        if (strcmp(head, "profile") == 0) {
            if (tail_len == 0 || tail_len > 30) {
                return make_copy("/error?code=invalid-username");
            }
            char username[31];
            memcpy(username, tail_raw + ts, tail_len);
            username[tail_len] = '\0';
            if (!is_safe_username(username)) {
                return make_copy("/error?code=invalid-username");
            }
            const char *prefix = "/user/profile/";
            size_t out_len = strlen(prefix) + strlen(username);
            char *out = (char*)malloc(out_len + 1);
            if (!out) return make_copy("/error?code=server-error");
            // Safe formatting with constant format string
            snprintf(out, out_len + 1, "%s%s", prefix, username);
            return out;
        } else {
            return make_copy("/error?code=unknown-page");
        }
    } else {
        // No colon: match known routes (case-insensitive)
        char lower[101];
        memcpy(lower, trimmed, tlen + 1);
        to_lower_ascii_inplace(lower);

        if (strcmp(lower, "home") == 0) return make_copy("/home");
        if (strcmp(lower, "about") == 0) return make_copy("/about");
        if (strcmp(lower, "contact") == 0) return make_copy("/contact");
        if (strcmp(lower, "help") == 0) return make_copy("/help");
        if (strcmp(lower, "dashboard") == 0) return make_copy("/user/dashboard");

        return make_copy("/error?code=unknown-page");
    }
}

int main(void) {
    const char *tests[5] = {
        "home",
        "Profile:Alice_123",
        "profile:bad/../hacker",
        "unknown",
        "dashboard"
    };
    for (int i = 0; i < 5; i++) {
        char *res = resolve_redirect(tests[i]);
        if (res) {
            printf("Input: %s -> Redirect: %s\n", tests[i], res);
            free(res);
        } else {
            printf("Input: %s -> Redirect: %s\n", tests[i], "(null)");
        }
    }
    return 0;
}