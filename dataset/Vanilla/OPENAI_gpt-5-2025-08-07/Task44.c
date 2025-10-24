#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static int is_safe_ascii(const char* s) {
    if (!s) return 0;
    for (const unsigned char* p = (const unsigned char*)s; *p; ++p) {
        unsigned char ch = *p;
        if (ch > 0x7F) return 0;
        if (!(isalnum(ch) || ch == ' ' || ch == '_' || ch == '-')) return 0;
    }
    return 1;
}

static void to_upper_inplace(char* s) {
    for (unsigned char* p = (unsigned char*)s; *p; ++p) {
        *p = (unsigned char)toupper(*p);
    }
}

static void trim_copy(const char* src, char* dst, size_t dstsz) {
    if (!src || !dst || dstsz == 0) return;
    const char* start = src;
    const char* end = src + strlen(src);
    while (start < end && isspace((unsigned char)*start)) start++;
    while (end > start && isspace((unsigned char)*(end - 1))) end--;
    size_t len = (size_t)(end - start);
    if (len >= dstsz) len = dstsz - 1;
    memmove(dst, start, len);
    dst[len] = '\0';
}

int process_data(int repeat_count, const char* text, char* out_buf, size_t out_size) {
    if (!out_buf || out_size == 0) return 0;
    if (repeat_count < 1 || repeat_count > 20) {
        snprintf(out_buf, out_size, "ERROR: repeatCount out of range [1,20]");
        return 0;
    }
    if (!text) {
        snprintf(out_buf, out_size, "ERROR: text is null");
        return 0;
    }

    char trimmed[128];
    trim_copy(text, trimmed, sizeof(trimmed));
    size_t tlen = strlen(trimmed);
    if (tlen < 1 || tlen > 100) {
        snprintf(out_buf, out_size, "ERROR: text length must be 1..100 after trimming");
        return 0;
    }
    if (!is_safe_ascii(trimmed)) {
        snprintf(out_buf, out_size, "ERROR: text contains disallowed characters (allowed: A-Z, a-z, 0-9, space, underscore, hyphen)");
        return 0;
    }

    char normalized[128];
    strncpy(normalized, trimmed, sizeof(normalized) - 1);
    normalized[sizeof(normalized) - 1] = '\0';
    to_upper_inplace(normalized);

    int checksum = 0;
    for (const unsigned char* p = (const unsigned char*)normalized; *p; ++p) {
        checksum = (checksum + (*p & 0xFF)) % 1000;
    }

    // Build repeated string safely
    char repeated[4096];
    repeated[0] = '\0';
    size_t rep_len = 0;
    for (int i = 0; i < repeat_count; ++i) {
        if (i > 0) {
            if (rep_len + 1 >= sizeof(repeated)) break;
            repeated[rep_len++] = '-';
            repeated[rep_len] = '\0';
        }
        size_t nlen = strlen(normalized);
        if (rep_len + nlen >= sizeof(repeated)) {
            nlen = sizeof(repeated) - 1 - rep_len;
        }
        strncat(repeated, normalized, nlen);
        rep_len += nlen;
        repeated[rep_len] = '\0';
    }

    // Compose final JSON-like result
    // Ensure we don't overflow out_buf
    int written = snprintf(out_buf, out_size,
                           "{\"original\":\"%s\", \"normalized\":\"%s\", \"repeatCount\":%d, \"repeated\":\"%s\", \"length\":%zu, \"checksum\":%d}",
                           trimmed, normalized, repeat_count, repeated, strlen(normalized), checksum);
    if (written < 0 || (size_t)written >= out_size) {
        // Truncated, but still return something
        return 0;
    }
    return 1;
}

int main(int argc, char* argv[]) {
    char out[8192];

    // 5 test cases
    if (process_data(3, "hello world", out, sizeof(out))) puts(out); else puts(out);
    if (process_data(1, "Safe_Input-123", out, sizeof(out))) puts(out); else puts(out);
    if (process_data(5, "   trim  me   ", out, sizeof(out))) puts(out); else puts(out);
    if (process_data(0, "abc", out, sizeof(out))) puts(out); else puts(out);           // invalid repeatCount
    if (process_data(2, "bad!char", out, sizeof(out))) puts(out); else puts(out);      // invalid character '!'

    // Optional user input via command-line args: first arg is repeatCount, the rest form the text
    if (argc >= 3) {
        char text[512];
        text[0] = '\0';
        for (int i = 2; i < argc; ++i) {
            if (i > 2) strncat(text, " ", sizeof(text) - 1 - strlen(text));
            strncat(text, argv[i], sizeof(text) - 1 - strlen(text));
        }
        char res[8192];
        int rc = atoi(argv[1]);
        if (process_data(rc, text, res, sizeof(res))) {
            puts(res);
        } else {
            puts(res);
        }
    }

    return 0;
}