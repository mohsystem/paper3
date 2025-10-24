#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

typedef struct {
    int ok;        // 1 on success, 0 on failure
    char* data;    // deserialized (validated) UTF-8 text
    char* error;   // error message on failure
} ProcessResult;

static void free_process_result(ProcessResult* r) {
    if (!r) return;
    if (r->data) free(r->data);
    if (r->error) free(r->error);
    r->data = NULL;
    r->error = NULL;
}

static int is_valid_utf8_bytes(const unsigned char* s, size_t len) {
    size_t i = 0;
    while (i < len) {
        unsigned char c = s[i];
        if ((c & 0x80u) == 0) {
            i++;
        } else if ((c & 0xE0u) == 0xC0u) {
            if (i + 1 >= len) return 0;
            unsigned char c1 = s[i+1];
            if ((c & 0xFEu) == 0xC0u) return 0;
            if ((c1 & 0xC0u) != 0x80u) return 0;
            i += 2;
        } else if ((c & 0xF0u) == 0xE0u) {
            if (i + 2 >= len) return 0;
            unsigned char c1 = s[i+1];
            unsigned char c2 = s[i+2];
            if ((c1 & 0xC0u) != 0x80u || (c2 & 0xC0u) != 0x80u) return 0;
            if (c == 0xE0u && (c1 & 0xE0u) == 0x80u) return 0;
            if (c == 0xEDu && (c1 & 0xE0u) == 0xA0u) return 0;
            i += 3;
        } else if ((c & 0xF8u) == 0xF0u) {
            if (i + 3 >= len) return 0;
            unsigned char c1 = s[i+1];
            unsigned char c2 = s[i+2];
            unsigned char c3 = s[i+3];
            if ((c1 & 0xC0u) != 0x80u || (c2 & 0xC0u) != 0x80u || (c3 & 0xC0u) != 0x80u) return 0;
            if (c == 0xF0u && (c1 & 0xF0u) == 0x80u) return 0;
            if (c > 0xF4u || (c == 0xF4u && c1 > 0x8Fu)) return 0;
            i += 4;
        } else {
            return 0;
        }
    }
    return 1;
}

static char* str_trim_new(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    size_t start = 0;
    while (start < len && isspace((unsigned char)s[start])) start++;
    size_t end = len;
    while (end > start && isspace((unsigned char)s[end - 1])) end--;
    size_t n = end - start;
    char* out = (char*)malloc(n + 1);
    if (!out) return NULL;
    if (n > 0) memcpy(out, s + start, n);
    out[n] = '\0';
    return out;
}

static int iequals_prefix(const char* s, size_t pos, const char* pat) {
    size_t sl = strlen(s);
    size_t pl = strlen(pat);
    if (pos + pl > sl) return 0;
    for (size_t i = 0; i < pl; ++i) {
        char a = s[pos + i];
        char b = pat[i];
        if (tolower((unsigned char)a) != tolower((unsigned char)b)) return 0;
    }
    return 1;
}

static int contains_no_doctype(const char* s) {
    size_t n = strlen(s);
    for (size_t i = 0; i + 8 <= n; ++i) {
        if (iequals_prefix(s, i, "<!doctype")) return 0;
    }
    return 1;
}

static int looks_like_json(const char* s) {
    char* t = str_trim_new(s);
    if (!t) return 0;
    size_t n = strlen(t);
    int ok = 0;
    if (n >= 2) {
        if ((t[0] == '{' && t[n-1] == '}') || (t[0] == '[' && t[n-1] == ']')) ok = 1;
    }
    free(t);
    return ok;
}

static int looks_like_xml(const char* s) {
    char* t = str_trim_new(s);
    if (!t) return 0;
    if (!contains_no_doctype(t)) {
        free(t);
        return 0;
    }
    size_t pos = 0;
    size_t n = strlen(t);
    if (n >= 5 && iequals_prefix(t, 0, "<?xml")) {
        char* decl = strstr(t, "?>");
        if (!decl) { free(t); return 0; }
        pos = (size_t)((decl - t) + 2);
        while (pos < n && isspace((unsigned char)t[pos])) pos++;
    }
    int ok = 0;
    if (pos < n && t[pos] == '<' && n > 0 && t[n-1] == '>') ok = 1;
    free(t);
    return ok;
}

static int base64_decode(const char* in, unsigned char** out, size_t* out_len, char** error) {
    if (!in || !out || !out_len) return 0;
    static int8_t D[256];
    static int initialized = 0;
    if (!initialized) {
        for (int i = 0; i < 256; ++i) D[i] = -1;
        const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        for (int i = 0; i < 64; ++i) D[(unsigned char)chars[i]] = (int8_t)i;
        D[(unsigned char)'='] = -2; // padding
        initialized = 1;
    }

    size_t in_len = strlen(in);
    if (in_len == 0) {
        *out = NULL;
        *out_len = 0;
        return 0;
    }

    // Build sanitized string (remove whitespace)
    char* s = (char*)malloc(in_len + 1);
    if (!s) return 0;
    size_t si = 0;
    for (size_t i = 0; i < in_len; ++i) {
        unsigned char c = (unsigned char)in[i];
        if (c > 0x7F) { if (error) { *error = strdup("Input not ASCII/UTF-8."); } free(s); return 0; }
        if (isspace(c)) continue;
        if (D[c] == -1 && c != '=') { if (error) { *error = strdup("Invalid Base64 character."); } free(s); return 0; }
        s[si++] = (char)c;
    }
    s[si] = '\0';

    if (si == 0 || (si % 4) != 0) {
        if (error) { *error = strdup("Base64 length is invalid."); }
        free(s);
        return 0;
    }

    size_t pad = 0;
    if (si >= 1 && s[si-1] == '=') pad++;
    if (si >= 2 && s[si-2] == '=') pad++;
    size_t groups = si / 4;
    size_t out_size = groups * 3;
    if (pad > 2) { if (error) { *error = strdup("Invalid Base64 padding."); } free(s); return 0; }
    out_size -= pad;
    if (out_size > (1u << 26)) { if (error) { *error = strdup("Decoded data too large."); } free(s); return 0; }

    unsigned char* buf = (unsigned char*)malloc(out_size ? out_size : 1);
    if (!buf) { free(s); return 0; }
    size_t bi = 0;
    for (size_t i = 0; i < si; i += 4) {
        int8_t a = D[(unsigned char)s[i]];
        int8_t b = D[(unsigned char)s[i+1]];
        int8_t c = D[(unsigned char)s[i+2]];
        int8_t d = D[(unsigned char)s[i+3]];
        if (a < 0 || b < 0) {
            if (error) { *error = strdup("Invalid Base64 quartet."); }
            free(s); free(buf); return 0;
        }
        uint32_t triple = ((uint32_t)a << 18) | ((uint32_t)b << 12);
        if (c >= 0) triple |= ((uint32_t)c << 6);
        if (d >= 0) triple |= (uint32_t)d;
        if (bi < out_size) buf[bi++] = (unsigned char)((triple >> 16) & 0xFFu);
        if (c >= 0 && bi < out_size) buf[bi++] = (unsigned char)((triple >> 8) & 0xFFu);
        if (d >= 0 && bi < out_size) buf[bi++] = (unsigned char)(triple & 0xFFu);
    }
    free(s);
    *out = buf;
    *out_len = out_size;
    return 1;
}

// Base64 encoder for tests
static char* base64_encode(const unsigned char* data, size_t len) {
    static const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t out_len = ((len + 2) / 3) * 4;
    char* out = (char*)malloc(out_len + 1);
    if (!out) return NULL;
    size_t oi = 0;
    for (size_t i = 0; i < len; i += 3) {
        uint32_t triple = (uint32_t)data[i] << 16;
        if (i + 1 < len) triple |= ((uint32_t)data[i+1] << 8);
        if (i + 2 < len) triple |= (uint32_t)data[i+2];
        out[oi++] = chars[(triple >> 18) & 0x3F];
        out[oi++] = chars[(triple >> 12) & 0x3F];
        out[oi++] = (i + 1 < len) ? chars[(triple >> 6) & 0x3F] : '=';
        out[oi++] = (i + 2 < len) ? chars[triple & 0x3F] : '=';
    }
    out[oi] = '\0';
    return out;
}

ProcessResult process_request(const char* raw_b64, const char* format) {
    ProcessResult res;
    res.ok = 0;
    res.data = NULL;
    res.error = NULL;

    const size_t MAX_RAW = (1u << 20);
    if (!raw_b64 || !format) {
        res.error = strdup("Null parameter.");
        return res;
    }
    size_t raw_len = strlen(raw_b64);
    if (raw_len == 0 || raw_len > MAX_RAW) {
        res.error = strdup("Invalid input size.");
        return res;
    }
    if (!is_valid_utf8_bytes((const unsigned char*)raw_b64, raw_len)) {
        res.error = strdup("raw_data is not valid UTF-8.");
        return res;
    }

    unsigned char* decoded = NULL;
    size_t decoded_len = 0;
    char* dec_err = NULL;
    if (!base64_decode(raw_b64, &decoded, &decoded_len, &dec_err)) {
        if (dec_err) {
            res.error = dec_err;
        } else {
            res.error = strdup("Base64 decode failed.");
        }
        return res;
    }

    if (!is_valid_utf8_bytes(decoded, decoded_len)) {
        free(decoded);
        res.error = strdup("Decoded data is not valid UTF-8.");
        return res;
    }

    // Copy decoded to NUL-terminated string
    char* decoded_str = (char*)malloc(decoded_len + 1);
    if (!decoded_str) {
        free(decoded);
        res.error = strdup("Memory allocation failed.");
        return res;
    }
    if (decoded_len > 0) memcpy(decoded_str, decoded, decoded_len);
    decoded_str[decoded_len] = '\0';
    free(decoded);

    // Validate format appearance
    // lowercase format
    size_t fl = strlen(format);
    char* f = (char*)malloc(fl + 1);
    if (!f) {
        free(decoded_str);
        res.error = strdup("Memory allocation failed.");
        return res;
    }
    for (size_t i = 0; i < fl; ++i) f[i] = (char)tolower((unsigned char)format[i]);
    f[fl] = '\0';

    int ok = 0;
    if (strcmp(f, "json") == 0) {
        ok = looks_like_json(decoded_str);
        if (!ok) {
            res.error = strdup("Decoded text does not look like valid JSON.");
            free(f);
            free(decoded_str);
            return res;
        }
    } else if (strcmp(f, "xml") == 0) {
        ok = looks_like_xml(decoded_str);
        if (!ok) {
            res.error = strdup("Decoded text does not look like safe XML.");
            free(f);
            free(decoded_str);
            return res;
        }
    } else {
        res.error = strdup("Unsupported format. Use 'json' or 'xml'.");
        free(f);
        free(decoded_str);
        return res;
    }

    free(f);
    res.ok = 1;
    res.data = decoded_str;
    return res;
}

int main(void) {
    // Prepare 5 test cases
    const char* json1 = "{\"name\":\"Alice\",\"age\":30}";
    char* b64_json1 = base64_encode((const unsigned char*)json1, strlen(json1));

    const char* xml1 = "<root><a>1</a><b>2</b></root>";
    char* b64_xml1 = base64_encode((const unsigned char*)xml1, strlen(xml1));

    const char* invalid_b64 = "!!!@@@";

    const char* json2 = "[\"x\", 1, true, null]";
    char* b64_json2 = base64_encode((const unsigned char*)json2, strlen(json2));

    const char* xml_doctype = "<?xml version=\"1.0\"?><!DOCTYPE foo [ <!ENTITY xxe SYSTEM \"file:///etc/passwd\"> ]><root>&xxe;</root>";
    char* b64_xml_doctype = base64_encode((const unsigned char*)xml_doctype, strlen(xml_doctype));

    struct {
        const char* b64;
        const char* fmt;
        int needs_free;
    } tests[5] = {
        { b64_json1, "json", 1 },
        { b64_xml1, "xml", 1 },
        { invalid_b64, "json", 0 },
        { b64_json2, "json", 1 },
        { b64_xml_doctype, "xml", 1 }
    };

    for (int i = 0; i < 5; ++i) {
        ProcessResult r = process_request(tests[i].b64, tests[i].fmt);
        printf("Test %d (%s): ", i + 1, tests[i].fmt);
        if (r.ok) {
            printf("OK | Data: %s\n", r.data ? r.data : "");
        } else {
            printf("ERROR | %s\n", r.error ? r.error : "Unknown error");
        }
        free_process_result(&r);
        if (tests[i].needs_free && tests[i].b64) free((void*)tests[i].b64);
    }

    return 0;
}