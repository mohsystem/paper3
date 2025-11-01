#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef struct {
    const char* key;
    const char* value;
} KeyValue;

static const char* get_value(const KeyValue* arr, size_t n, const char* key) {
    if (!arr || !key) return NULL;
    for (size_t i = 0; i < n; i++) {
        if (arr[i].key && strcmp(arr[i].key, key) == 0) {
            return arr[i].value;
        }
    }
    return NULL;
}

typedef struct {
    char* buf;
    size_t len;
    size_t cap;
} StrBuf;

static int sb_init(StrBuf* sb, size_t initial) {
    if (!sb) return 0;
    if (initial == 0) initial = 64;
    sb->buf = (char*)malloc(initial);
    if (!sb->buf) return 0;
    sb->len = 0;
    sb->cap = initial;
    sb->buf[0] = '\0';
    return 1;
}

static void sb_free(StrBuf* sb) {
    if (!sb) return;
    free(sb->buf);
    sb->buf = NULL;
    sb->len = sb->cap = 0;
}

static int sb_ensure(StrBuf* sb, size_t add) {
    if (!sb) return 0;
    if (add > SIZE_MAX - sb->len - 1) return 0; // overflow check including null terminator
    size_t need = sb->len + add + 1;
    if (need <= sb->cap) return 1;
    size_t newcap = sb->cap ? sb->cap : 64;
    while (newcap < need) {
        if (newcap > SIZE_MAX / 2) { newcap = need; break; }
        newcap *= 2;
    }
    char* p = (char*)realloc(sb->buf, newcap);
    if (!p) return 0;
    sb->buf = p;
    sb->cap = newcap;
    return 1;
}

static int sb_append_mem(StrBuf* sb, const char* data, size_t n) {
    if (!data) return 1;
    if (!sb_ensure(sb, n)) return 0;
    memcpy(sb->buf + sb->len, data, n);
    sb->len += n;
    sb->buf[sb->len] = '\0';
    return 1;
}

static int sb_append_char(StrBuf* sb, char c) {
    if (!sb_ensure(sb, 1)) return 0;
    sb->buf[sb->len++] = c;
    sb->buf[sb->len] = '\0';
    return 1;
}

static char* escape_html(const char* s) {
    if (!s) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    StrBuf sb;
    if (!sb_init(&sb, strlen(s) * 2 + 16)) return NULL;
    for (const char* p = s; *p; ++p) {
        switch (*p) {
            case '&': if (!sb_append_mem(&sb, "&amp;", 5)) { sb_free(&sb); return NULL; } break;
            case '<': if (!sb_append_mem(&sb, "&lt;", 4)) { sb_free(&sb); return NULL; } break;
            case '>': if (!sb_append_mem(&sb, "&gt;", 4)) { sb_free(&sb); return NULL; } break;
            case '"': if (!sb_append_mem(&sb, "&quot;", 6)) { sb_free(&sb); return NULL; } break;
            case '\'': if (!sb_append_mem(&sb, "&#39;", 5)) { sb_free(&sb); return NULL; } break;
            default: if (!sb_append_char(&sb, *p)) { sb_free(&sb); return NULL; } break;
        }
    }
    return sb.buf; // caller frees
}

char* render_response_from_env(const KeyValue* env, size_t env_len,
                               const KeyValue* request, size_t req_len) {
    const char* name = get_value(request, req_len, "name");
    if (!name) name = "";
    const char* templ = get_value(env, env_len, "template");
    static const char* default_templ = "Hello, {{name}}";
    if (!templ) templ = default_templ;
    size_t tlen = strlen(templ);
    if (tlen > 10000) {
        templ = default_templ;
        tlen = strlen(templ);
    }

    char* escaped = escape_html(name);
    if (!escaped) return NULL;
    const char* token = "{{name}}";
    const size_t token_len = 8;

    StrBuf out;
    if (!sb_init(&out, tlen + 32)) {
        free(escaped);
        return NULL;
    }

    const char* p = templ;
    while (*p) {
        const char* match = strstr(p, token);
        if (!match) {
            if (!sb_append_mem(&out, p, strlen(p))) { sb_free(&out); free(escaped); return NULL; }
            break;
        } else {
            if (!sb_append_mem(&out, p, (size_t)(match - p))) { sb_free(&out); free(escaped); return NULL; }
            if (!sb_append_mem(&out, escaped, strlen(escaped))) { sb_free(&out); free(escaped); return NULL; }
            p = match + token_len;
        }
    }

    free(escaped);
    return out.buf; // caller frees
}

char* home(const KeyValue* env, size_t env_len,
           const KeyValue* request, size_t req_len) {
    return render_response_from_env(env, env_len, request, req_len);
}

int main(void) {
    KeyValue env[] = { {"template", "<h1>Welcome, {{name}}</h1>"} };

    KeyValue req1[] = { {"name", "Alice"} };
    KeyValue req2[] = { {"name", "<script>alert(1)</script>"} };
    KeyValue req3[] = { /* missing name */ };
    KeyValue req4[] = { {"name", "O'Reilly & Co"} };
    char longname[51]; for (int i = 0; i < 50; ++i) longname[i] = 'A'; longname[50] = '\0';
    KeyValue req5[] = { {"name", longname} };

    char* r;

    r = home(env, 1, req1, 1); printf("Test 1: %s\n", r ? r : "(null)"); free(r);
    r = home(env, 1, req2, 1); printf("Test 2: %s\n", r ? r : "(null)"); free(r);
    r = home(env, 1, req3, 0); printf("Test 3: %s\n", r ? r : "(null)"); free(r);
    r = home(env, 1, req4, 1); printf("Test 4: %s\n", r ? r : "(null)"); free(r);
    r = home(env, 1, req5, 1); printf("Test 5: %s\n", r ? r : "(null)"); free(r);

    return 0;
}