#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAX_INPUT_BYTES (10 * 1024 * 1024) /* 10MB */

static int is_valid_utf8(const unsigned char *data, size_t len) {
    size_t i = 0;
    while (i < len) {
        unsigned char c = data[i];
        if (c <= 0x7F) {
            i++;
        } else if ((c >> 5) == 0x6) {
            if (i + 1 >= len) return 0;
            unsigned char c1 = data[i+1];
            if ((c1 & 0xC0) != 0x80) return 0;
            unsigned int code = ((c & 0x1F) << 6) | (c1 & 0x3F);
            if (code < 0x80) return 0; /* overlong */
            i += 2;
        } else if ((c >> 4) == 0xE) {
            if (i + 2 >= len) return 0;
            unsigned char c1 = data[i+1], c2 = data[i+2];
            if ((c1 & 0xC0) != 0x80 || (c2 & 0xC0) != 0x80) return 0;
            unsigned int code = ((c & 0x0F) << 12) | ((c1 & 0x3F) << 6) | (c2 & 0x3F);
            if (code < 0x800) return 0; /* overlong */
            if (code >= 0xD800 && code <= 0xDFFF) return 0; /* surrogate */
            i += 3;
        } else if ((c >> 3) == 0x1E) {
            if (i + 3 >= len) return 0;
            unsigned char c1 = data[i+1], c2 = data[i+2], c3 = data[i+3];
            if ((c1 & 0xC0) != 0x80 || (c2 & 0xC0) != 0x80 || (c3 & 0xC0) != 0x80) return 0;
            unsigned int code = ((c & 0x07) << 18) | ((c1 & 0x3F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
            if (code < 0x10000 || code > 0x10FFFF) return 0; /* overlong or out of range */
            i += 4;
        } else {
            return 0;
        }
    }
    return 1;
}

static char* base64_encode(const unsigned char* data, size_t len) {
    static const char tbl[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t out_len = ((len + 2) / 3) * 4;
    char* out = (char*)malloc(out_len + 1);
    if (!out) return NULL;
    size_t i = 0, j = 0;
    while (i + 2 < len) {
        unsigned int n = ((unsigned int)data[i] << 16) | ((unsigned int)data[i+1] << 8) | (unsigned int)data[i+2];
        out[j++] = tbl[(n >> 18) & 63];
        out[j++] = tbl[(n >> 12) & 63];
        out[j++] = tbl[(n >> 6) & 63];
        out[j++] = tbl[n & 63];
        i += 3;
    }
    if (i + 1 < len) {
        unsigned int n = ((unsigned int)data[i] << 16) | ((unsigned int)data[i+1] << 8);
        out[j++] = tbl[(n >> 18) & 63];
        out[j++] = tbl[(n >> 12) & 63];
        out[j++] = tbl[(n >> 6) & 63];
        out[j++] = '=';
    } else if (i < len) {
        unsigned int n = ((unsigned int)data[i] << 16);
        out[j++] = tbl[(n >> 18) & 63];
        out[j++] = tbl[(n >> 12) & 63];
        out[j++] = '=';
        out[j++] = '=';
    }
    out[j] = '\0';
    return out;
}

static int base64_value(int c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    if (c == '=') return -2;
    return -1;
}

static unsigned char* base64_decode(const char* in, size_t in_len, size_t* out_len) {
    unsigned char* out = (unsigned char*)malloc((in_len / 4) * 3 + 3);
    if (!out) return NULL;
    size_t o = 0;

    int q[4];
    int qn = 0;
    for (size_t i = 0; i < in_len; i++) {
        unsigned char c = (unsigned char)in[i];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') continue; /* skip whitespace */
        int v = base64_value(c);
        if (v == -1) { free(out); return NULL; }
        q[qn++] = v;
        if (qn == 4) {
            if (q[2] == -2 && q[3] != -2) { free(out); return NULL; }
            if (q[3] == -2) {
                if (q[2] == -2) {
                    unsigned int n = ((unsigned int)q[0] << 18) | ((unsigned int)q[1] << 12);
                    out[o++] = (unsigned char)((n >> 16) & 0xFF);
                } else {
                    unsigned int n = ((unsigned int)q[0] << 18) | ((unsigned int)q[1] << 12) | ((unsigned int)q[2] << 6);
                    out[o++] = (unsigned char)((n >> 16) & 0xFF);
                    out[o++] = (unsigned char)((n >> 8) & 0xFF);
                }
            } else {
                unsigned int n = ((unsigned int)q[0] << 18) | ((unsigned int)q[1] << 12) | ((unsigned int)q[2] << 6) | (unsigned int)q[3];
                out[o++] = (unsigned char)((n >> 16) & 0xFF);
                out[o++] = (unsigned char)((n >> 8) & 0xFF);
                out[o++] = (unsigned char)(n & 0xFF);
            }
            qn = 0;
        }
    }
    if (qn != 0) { free(out); return NULL; }
    *out_len = o;
    return out;
}

char* process_request(const unsigned char* raw_data, size_t raw_len) {
    if (!raw_data) return NULL;
    if (raw_len > MAX_INPUT_BYTES) return NULL;

    if (!is_valid_utf8(raw_data, raw_len)) return NULL;

    /* Copy raw_data to C string for base64 decoder */
    char* b64 = (char*)malloc(raw_len + 1);
    if (!b64) return NULL;
    memcpy(b64, raw_data, raw_len);
    b64[raw_len] = '\0';

    size_t decoded_len = 0;
    unsigned char* decoded = base64_decode(b64, raw_len, &decoded_len);
    free(b64);
    if (!decoded) return NULL;

    if (!is_valid_utf8(decoded, decoded_len)) {
        free(decoded);
        return NULL;
    }

    char* stored = (char*)malloc(decoded_len + 1);
    if (!stored) {
        free(decoded);
        return NULL;
    }
    memcpy(stored, decoded, decoded_len);
    stored[decoded_len] = '\0';
    free(decoded);
    return stored;
}

int main(void) {
    const char* originals[5] = {
        "{\"name\":\"Alice\",\"age\":30}",
        "<person><name>Alice</name><age>30</age></person>",
        "[\"x\",1,true,null]",
        "<root/>",
        "Hello, 世界"
    };

    for (int i = 0; i < 5; i++) {
        const char* orig = originals[i];
        char* b64 = base64_encode((const unsigned char*)orig, strlen(orig));
        if (!b64) {
            printf("Test %d Error: encode failed\n", i+1);
            continue;
        }
        char* result = process_request((const unsigned char*)b64, strlen(b64));
        if (result) {
            printf("Test %d OK: %s\n", i+1, result);
            free(result);
        } else {
            printf("Test %d Error: processing failed\n", i+1);
        }
        free(b64);
    }
    return 0;
}