#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_PAYLOAD_BYTES 8192
#define MAX_NAME_CHARS 256

static int hex_val(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static int is_escaped(const char* s, size_t pos) {
    size_t backslashes = 0;
    if (pos == 0) return 0;
    size_t i = pos;
    while (i > 0 && s[i-1] == '\\') {
        backslashes++;
        i--;
    }
    return (backslashes % 2) == 1;
}

static char* json_error(const char* msg) {
    const char* prefix = "{\"error\":\"";
    const char* suffix = "\"}";
    size_t len = strlen(msg);
    // Worst case every char escaped -> ~6x
    size_t cap = strlen(prefix) + len * 6 + strlen(suffix) + 1;
    char* out = (char*)malloc(cap);
    if (!out) return NULL;
    size_t pos = 0;
    memcpy(out + pos, prefix, strlen(prefix)); pos += strlen(prefix);
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)msg[i];
        switch (c) {
            case '\"': out[pos++]='\\'; out[pos++]='\"'; break;
            case '\\': out[pos++]='\\'; out[pos++]='\\'; break;
            case '\b': out[pos++]='\\'; out[pos++]='b'; break;
            case '\f': out[pos++]='\\'; out[pos++]='f'; break;
            case '\n': out[pos++]='\\'; out[pos++]='n'; break;
            case '\r': out[pos++]='\\'; out[pos++]='r'; break;
            case '\t': out[pos++]='\\'; out[pos++]='t'; break;
            default:
                if (c < 0x20) {
                    int n = snprintf(out + pos, cap - pos, "\\u%04x", (unsigned int)c);
                    if (n < 0) { free(out); return NULL; }
                    pos += (size_t)n;
                } else {
                    out[pos++] = (char)c;
                }
        }
    }
    memcpy(out + pos, suffix, strlen(suffix)); pos += strlen(suffix);
    out[pos] = '\0';
    return out;
}

static char* json_escape(const char* s) {
    size_t len = strlen(s);
    size_t cap = len * 6 + 1;
    char* out = (char*)malloc(cap);
    if (!out) return NULL;
    size_t pos = 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)s[i];
        switch (c) {
            case '\"': out[pos++]='\\'; out[pos++]='\"'; break;
            case '\\': out[pos++]='\\'; out[pos++]='\\'; break;
            case '\b': out[pos++]='\\'; out[pos++]='b'; break;
            case '\f': out[pos++]='\\'; out[pos++]='f'; break;
            case '\n': out[pos++]='\\'; out[pos++]='n'; break;
            case '\r': out[pos++]='\\'; out[pos++]='r'; break;
            case '\t': out[pos++]='\\'; out[pos++]='t'; break;
            default:
                if (c < 0x20) {
                    int n = snprintf(out + pos, cap - pos, "\\u%04x", (unsigned int)c);
                    if (n < 0) { free(out); return NULL; }
                    pos += (size_t)n;
                } else {
                    out[pos++] = (char)c;
                }
        }
    }
    out[pos] = '\0';
    return out;
}

static int is_valid_name(const char* name) {
    if (!name) return 0;
    size_t len = strlen(name);
    if (len == 0 || len > MAX_NAME_CHARS) return 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)name[i];
        if (c < 0x20) return 0;
    }
    return 1;
}

static char* url_decode(const char* s, int* ok) {
    *ok = 0;
    size_t len = strlen(s);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    size_t pos = 0;
    for (size_t i = 0; i < len; i++) {
        char c = s[i];
        if (c == '+') {
            out[pos++] = ' ';
        } else if (c == '%') {
            if (i + 2 >= len) { free(out); return NULL; }
            int hi = hex_val(s[i+1]);
            int lo = hex_val(s[i+2]);
            if (hi < 0 || lo < 0) { free(out); return NULL; }
            out[pos++] = (char)((hi << 4) | lo);
            i += 2;
        } else {
            out[pos++] = c;
        }
        if (pos > MAX_NAME_CHARS * 4) { free(out); return NULL; }
    }
    out[pos] = '\0';
    *ok = 1;
    return out;
}

static char* extract_from_form(const char* body) {
    size_t len = strlen(body);
    size_t i = 0;
    while (i <= len) {
        size_t amp = i;
        while (amp < len && body[amp] != '&') amp++;
        size_t end = amp;
        if (end > i) {
            size_t eq = i;
            while (eq < end && body[eq] != '=') eq++;
            size_t klen = (eq < end) ? (eq - i) : (end - i);
            size_t vlen = (eq < end) ? (end - eq - 1) : 0;
            char* key = (char*)malloc(klen + 1);
            char* val = (char*)malloc(vlen + 1);
            if (!key || !val) { free(key); free(val); return NULL; }
            memcpy(key, body + i, klen); key[klen]='\0';
            if (vlen > 0) memcpy(val, body + eq + 1, vlen);
            val[vlen]='\0';
            int ok1=0, ok2=0;
            char* dkey = url_decode(key, &ok1);
            char* dval = url_decode(val, &ok2);
            free(key); free(val);
            if (!dkey || !ok1) { free(dkey); free(dval); return NULL; }
            if (strcmp(dkey, "name") == 0) {
                if (!dval || !ok2) { free(dkey); free(dval); return NULL; }
                free(dkey);
                return dval;
            }
            free(dkey);
            free(dval);
        }
        if (amp == len) break;
        i = amp + 1;
    }
    return NULL;
}

static char* extract_from_json(const char* s) {
    size_t len = strlen(s);
    const char* key = "name";
    size_t klen = 4;
    size_t pos = 0;
    while (pos < len) {
        // find quote
        char* qptr = strchr(s + pos, '"');
        if (!qptr) return NULL;
        size_t q = (size_t)(qptr - s);
        if (!is_escaped(s, q)) {
            if (q + 1 + klen < len && strncmp(s + q + 1, key, klen) == 0) {
                size_t q2 = q + 1 + klen;
                if (q2 < len && s[q2] == '"' && !is_escaped(s, q2)) {
                    size_t i = q2 + 1;
                    while (i < len && isspace((unsigned char)s[i])) i++;
                    if (i >= len || s[i] != ':') return NULL;
                    i++;
                    while (i < len && isspace((unsigned char)s[i])) i++;
                    if (i >= len || s[i] != '"') return NULL;
                    i++;
                    // decode string value
                    char* val = (char*)malloc(MAX_NAME_CHARS * 4 + 1);
                    if (!val) return NULL;
                    size_t vp = 0;
                    while (i < len) {
                        char c = s[i++];
                        if (c == '\\') {
                            if (i >= len) { free(val); return NULL; }
                            char e = s[i++];
                            switch (e) {
                                case '"': val[vp++] = '"'; break;
                                case '\\': val[vp++] = '\\'; break;
                                case '/': val[vp++] = '/'; break;
                                case 'b': val[vp++] = '\b'; break;
                                case 'f': val[vp++] = '\f'; break;
                                case 'n': val[vp++] = '\n'; break;
                                case 'r': val[vp++] = '\r'; break;
                                case 't': val[vp++] = '\t'; break;
                                case 'u': {
                                    if (i + 4 > len) { free(val); return NULL; }
                                    int code = 0;
                                    for (int k = 0; k < 4; k++) {
                                        int hv = hex_val(s[i + k]);
                                        if (hv < 0) { free(val); return NULL; }
                                        code = (code << 4) | hv;
                                    }
                                    i += 4;
                                    // Simple BMP to UTF-8 (limited)
                                    if (code <= 0x7F) {
                                        val[vp++] = (char)code;
                                    } else if (code <= 0x7FF) {
                                        if (vp + 2 >= MAX_NAME_CHARS * 4) { free(val); return NULL; }
                                        val[vp++] = (char)(0xC0 | ((code >> 6) & 0x1F));
                                        val[vp++] = (char)(0x80 | (code & 0x3F));
                                    } else {
                                        if (vp + 3 >= MAX_NAME_CHARS * 4) { free(val); return NULL; }
                                        val[vp++] = (char)(0xE0 | ((code >> 12) & 0x0F));
                                        val[vp++] = (char)(0x80 | ((code >> 6) & 0x3F));
                                        val[vp++] = (char)(0x80 | (code & 0x3F));
                                    }
                                } break;
                                default: free(val); return NULL;
                            }
                        } else if (c == '"') {
                            val[vp] = '\0';
                            return val;
                        } else {
                            if (vp + 1 >= MAX_NAME_CHARS * 4) { free(val); return NULL; }
                            val[vp++] = c;
                        }
                        if (vp > MAX_NAME_CHARS * 4) { free(val); return NULL; }
                    }
                    free(val);
                    return NULL;
                }
            }
        }
        pos = q + 1;
    }
    return NULL;
}

char* process_payload(const unsigned char* payload, size_t len, const char* content_type) {
    if (!payload) {
        return json_error("invalid input");
    }
    if (len > MAX_PAYLOAD_BYTES) {
        return json_error("payload too large");
    }
    // Copy payload to null-terminated buffer for parsing
    char* body = (char*)malloc(len + 1);
    if (!body) return NULL;
    memcpy(body, payload, len);
    body[len] = '\0';

    char ctbuf[128];
    ctbuf[0] = '\0';
    if (content_type) {
        size_t ctn = strlen(content_type);
        if (ctn >= sizeof(ctbuf)) ctn = sizeof(ctbuf) - 1;
        for (size_t i = 0; i < ctn; i++) {
            char ch = content_type[i];
            ctbuf[i] = (char)tolower((unsigned char)ch);
        }
        ctbuf[ctn] = '\0';
    }

    char* name = NULL;

    int tried = 0;
    // Heuristics
    size_t bi = 0;
    while (bi < len && isspace((unsigned char)body[bi])) bi++;
    if ((content_type && strstr(ctbuf, "application/json")) || (bi < len && body[bi] == '{')) {
        name = extract_from_json(body);
        tried = 1;
    } else if ((content_type && strstr(ctbuf, "application/x-www-form-urlencoded")) || strchr(body, '=')) {
        name = extract_from_form(body);
        tried = 1;
    }
    if (!tried) {
        name = extract_from_json(body);
        if (!name) {
            name = extract_from_form(body);
        }
    }

    free(body);

    if (!name) {
        return json_error("name field missing");
    }

    int valid = is_valid_name(name);
    if (!valid) {
        free(name);
        return json_error("invalid name");
    }

    char* escaped = json_escape(name);
    free(name);
    if (!escaped) return NULL;

    const char* prefix = "{\"name\":\"";
    const char* suffix = "\"}";
    size_t outlen = strlen(prefix) + strlen(escaped) + strlen(suffix);
    char* out = (char*)malloc(outlen + 1);
    if (!out) { free(escaped); return NULL; }
    strcpy(out, prefix);
    strcat(out, escaped);
    strcat(out, suffix);
    free(escaped);
    return out;
}

int main(void) {
    const char* ct1 = "application/json";
    const char* ct2 = "application/x-www-form-urlencoded";
    const char* ct3 = "";

    const char* t1 = "{\"name\":\"Alice\"}";
    const char* t2 = "{\"age\":30}";
    const char* t3 = "name=Bob%20Builder&role=dev";
    char longname[400];
    memset(longname, 'x', sizeof(longname));
    longname[399] = '\0';
    char t4[420];
    snprintf(t4, sizeof(t4), "{\"name\":\"%s\"}", longname);
    const char* t5 = "invalid_payload";

    struct {
        const char* payload;
        size_t len;
        const char* ct;
    } tests[5] = {
        { t1, strlen(t1), ct1 },
        { t2, strlen(t2), ct1 },
        { t3, strlen(t3), ct2 },
        { t4, strlen(t4), ct1 },
        { t5, strlen(t5), ct3 }
    };

    for (int i = 0; i < 5; i++) {
        char* res = process_payload((const unsigned char*)tests[i].payload, tests[i].len, tests[i].ct);
        if (res) {
            printf("Test %d: %s\n", i + 1, res);
            free(res);
        } else {
            printf("Test %d: <null>\n", i + 1);
        }
    }

    return 0;
}