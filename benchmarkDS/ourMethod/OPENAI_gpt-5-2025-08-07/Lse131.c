#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_RESPONSE_LEN 4096
#define MAX_TEMPLATE_LEN 2000
#define MAX_NAME_LEN 100

typedef struct {
    const char* template_str; /* may be NULL */
} Environment;

typedef struct {
    const char* name; /* may be NULL */
} Request;

static int is_allowed_char(unsigned char c) {
    return (isalnum(c) || c == ' ' || c == '_' || c == '-' || c == '.');
}

static char* strndup_safe(const char* s, size_t n) {
    if (!s) return NULL;
    char* out = (char*)malloc(n + 1);
    if (!out) return NULL;
    memcpy(out, s, n);
    out[n] = '\0';
    return out;
}

static char* sanitize_name(const char* name) {
    const char* guest = "Guest";
    if (name == NULL) {
        char* out = (char*)malloc(strlen(guest) + 1);
        if (out) strcpy(out, guest);
        return out;
    }
    /* Trim */
    const char* start = name;
    while (*start && isspace((unsigned char)*start)) start++;
    const char* end = name + strlen(name);
    while (end > start && isspace((unsigned char)*(end - 1))) end--;
    size_t len = (size_t)(end - start);
    if (len == 0 || len > MAX_NAME_LEN) {
        char* out = (char*)malloc(strlen(guest) + 1);
        if (out) strcpy(out, guest);
        return out;
    }
    for (size_t i = 0; i < len; i++) {
        if (!is_allowed_char((unsigned char)start[i])) {
            char* out = (char*)malloc(strlen(guest) + 1);
            if (out) strcpy(out, guest);
            return out;
        }
    }
    return strndup_safe(start, len);
}

static char* html_escape(const char* s) {
    if (!s) {
        char* out = (char*)malloc(1);
        if (out) out[0] = '\0';
        return out;
    }
    size_t len = strlen(s);
    /* Worst-case expansion ~6x, but cap result to MAX_RESPONSE_LEN */
    size_t est = len * 6 + 1;
    if (est > (size_t)MAX_RESPONSE_LEN) est = (size_t)MAX_RESPONSE_LEN;
    char* out = (char*)malloc(est);
    if (!out) return NULL;

    size_t o = 0;
    for (size_t i = 0; i < len; i++) {
        const char* rep = NULL;
        switch (s[i]) {
            case '&': rep = "&amp;"; break;
            case '<': rep = "&lt;"; break;
            case '>': rep = "&gt;"; break;
            case '"': rep = "&quot;"; break;
            case '\'': rep = "&#x27;"; break;
            default: break;
        }
        if (rep) {
            size_t rlen = strlen(rep);
            if (o + rlen >= est) { /* avoid overflow */
                break;
            }
            memcpy(out + o, rep, rlen);
            o += rlen;
        } else {
            if (o + 1 >= est) break;
            out[o++] = s[i];
        }
        if (o >= (size_t)(MAX_RESPONSE_LEN - 1)) break;
    }
    out[o] = '\0';
    return out;
}

static const char* get_template_safe(const Environment* env) {
    static const char* def = "Hello, {{name}}!";
    if (!env || !env->template_str || env->template_str[0] == '\0') return def;
    size_t len = strlen(env->template_str);
    if (len > MAX_TEMPLATE_LEN) return def;
    return env->template_str;
}

/* Replace all occurrences of token with replacement, bounded by MAX_RESPONSE_LEN */
static char* replace_token_bounded(const char* template_str, const char* token, const char* replacement) {
    size_t tpl_len = strlen(template_str);
    size_t tok_len = strlen(token);
    size_t rep_len = strlen(replacement);

    /* First pass: count occurrences (bounded to avoid pathological cases) */
    size_t count = 0;
    for (size_t i = 0; i + tok_len <= tpl_len && count < 64; ) {
        if (memcmp(template_str + i, token, tok_len) == 0) {
            count++;
            i += tok_len;
        } else {
            i++;
        }
    }

    /* Estimate final size */
    size_t final_est = tpl_len + count * (rep_len - tok_len);
    if (final_est > (size_t)(MAX_RESPONSE_LEN - 1)) final_est = (size_t)(MAX_RESPONSE_LEN - 1);

    char* out = (char*)malloc(final_est + 1);
    if (!out) return NULL;

    size_t o = 0;
    for (size_t i = 0; i < tpl_len;) {
        if (i + tok_len <= tpl_len && memcmp(template_str + i, token, tok_len) == 0) {
            /* Append replacement */
            size_t to_copy = rep_len;
            if (o + to_copy > (size_t)(MAX_RESPONSE_LEN - 1)) {
                to_copy = (size_t)(MAX_RESPONSE_LEN - 1) - o;
            }
            if (to_copy > 0) {
                memcpy(out + o, replacement, to_copy);
                o += to_copy;
            }
            i += tok_len;
        } else {
            if (o + 1 > (size_t)(MAX_RESPONSE_LEN - 1)) break;
            out[o++] = template_str[i++];
        }
    }
    out[o] = '\0';
    return out;
}

char* render_response_from_env(const Environment* env, const Request* req) {
    const char* tpl = get_template_safe(env);
    char* sanitized = sanitize_name(req ? req->name : NULL);
    if (!sanitized) return NULL;

    char* escaped = html_escape(sanitized);
    free(sanitized);
    if (!escaped) return NULL;

    const char* token = "{{name}}";
    char* rendered = replace_token_bounded(tpl, token, escaped);
    free(escaped);
    return rendered;
}

char* home(const Environment* env, const Request* req) {
    return render_response_from_env(env, req);
}

int main(void) {
    char* out = NULL;

    /* Test 1: Normal */
    Environment env1 = { "Welcome, {{name}}." };
    Request req1 = { "Alice" };
    out = home(&env1, &req1);
    printf("Test 1: %s\n", out ? out : "(null)");
    free(out);

    /* Test 2: Escaping */
    Environment env2 = { "<h1>Hello, {{name}}</h1>" };
    Request req2 = { "<script>alert(1)</script>" };
    out = home(&env2, &req2);
    printf("Test 2: %s\n", out ? out : "(null)");
    free(out);

    /* Test 3: Too long name -> Guest */
    char longName[151];
    for (int i = 0; i < 150; i++) longName[i] = 'A';
    longName[150] = '\0';
    Environment env3 = { "Hi {{name}}!" };
    Request req3 = { longName };
    out = home(&env3, &req3);
    printf("Test 3: %s\n", out ? out : "(null)");
    free(out);

    /* Test 4: Invalid chars -> Guest */
    Environment env4 = { "User: {{name}}" };
    Request req4 = { "Bob!!@@##" };
    out = home(&env4, &req4);
    printf("Test 4: %s\n", out ? out : "(null)");
    free(out);

    /* Test 5: Missing template -> default */
    Environment env5 = { NULL };
    Request req5 = { "Charlie" };
    out = home(&env5, &req5);
    printf("Test 5: %s\n", out ? out : "(null)");
    free(out);

    return 0;
}