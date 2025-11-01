#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#define MAX_NAME_LEN 100
#define MAX_TEMPLATE_LEN 8000
#define MAX_RESPONSE_LEN 8192

typedef struct {
    const char* name_param; /* Raw name parameter from request */
} Request;

typedef struct {
    Request request;
    const char* template_str; /* Template from environment */
} Env;

typedef struct {
    int status;
    char* body;
} Response;

/* --------- Utility: safe memory functions --------- */
static void* xmalloc(size_t sz) {
    void* p = malloc(sz);
    if (!p) {
        fprintf(stderr, "Allocation failed\n");
        exit(1);
    }
    return p;
}

static void* xrealloc(void* ptr, size_t sz) {
    void* p = realloc(ptr, sz);
    if (!p) {
        fprintf(stderr, "Reallocation failed\n");
        free(ptr);
        exit(1);
    }
    return p;
}

/* --------- Sanitize name: allow [A-Za-z0-9 _-], max 100, trim --------- */
static char* sanitize_name(const char* raw) {
    const char* in = raw ? raw : "";
    /* Trim leading and trailing spaces */
    while (*in && isspace((unsigned char)*in)) in++;
    size_t len = strlen(in);
    while (len > 0 && isspace((unsigned char)in[len - 1])) len--;
    /* Filter allowed characters */
    char* out = (char*)xmalloc(MAX_NAME_LEN + 1);
    size_t out_len = 0;
    for (size_t i = 0; i < len && out_len < MAX_NAME_LEN; i++) {
        char c = in[i];
        if ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            c == ' ' || c == '-' || c == '_') {
            out[out_len++] = c;
        }
    }
    /* Trim again */
    while (out_len > 0 && isspace((unsigned char)out[out_len - 1])) out_len--;
    out[out_len] = '\0';
    if (out_len == 0) {
        /* Fallback to Guest */
        free(out);
        char* g = (char*)xmalloc(6);
        memcpy(g, "Guest", 6);
        return g;
    }
    return out;
}

/* --------- HTML escape (defensive; sanitized input should be safe) --------- */
static char* html_escape(const char* s) {
    if (!s) {
        char* empty = (char*)xmalloc(1);
        empty[0] = '\0';
        return empty;
    }
    size_t cap = (strlen(s) * 2) + 16;
    if (cap > MAX_RESPONSE_LEN) cap = MAX_RESPONSE_LEN;
    char* out = (char*)xmalloc(cap);
    size_t out_len = 0;
    for (const char* p = s; *p; p++) {
        const char* rep = NULL;
        switch (*p) {
            case '&': rep = "&amp;"; break;
            case '<': rep = "&lt;"; break;
            case '>': rep = "&gt;"; break;
            case '"': rep = "&quot;"; break;
            case '\'': rep = "&#x27;"; break;
            case '/': rep = "&#x2F;"; break;
            default: break;
        }
        if (rep) {
            size_t rlen = strlen(rep);
            if (out_len + rlen + 1 >= cap) {
                size_t new_cap = cap * 2;
                if (new_cap > MAX_RESPONSE_LEN) new_cap = MAX_RESPONSE_LEN;
                if (new_cap <= cap) { /* cannot grow */
                    break;
                }
                out = (char*)xrealloc(out, new_cap);
                cap = new_cap;
            }
            memcpy(out + out_len, rep, rlen);
            out_len += rlen;
        } else {
            if (out_len + 2 >= cap) {
                size_t new_cap = cap * 2;
                if (new_cap > MAX_RESPONSE_LEN) new_cap = MAX_RESPONSE_LEN;
                if (new_cap <= cap) {
                    break;
                }
                out = (char*)xrealloc(out, new_cap);
                cap = new_cap;
            }
            out[out_len++] = *p;
        }
        if (out_len >= MAX_RESPONSE_LEN - 1) break;
    }
    out[out_len] = '\0';
    return out;
}

/* --------- Render template: replace all occurrences of {{name}} --------- */
static char* render_template(const char* tmpl, const char* name_val) {
    const char* token = "{{name}}";
    size_t token_len = strlen(token);
    const char* safe_tmpl = tmpl;
    const char* default_tmpl = "Hello, {{name}}!";
    if (!safe_tmpl || strlen(safe_tmpl) > MAX_TEMPLATE_LEN) {
        safe_tmpl = default_tmpl;
    }

    size_t cap = strlen(safe_tmpl) + strlen(name_val) + 16;
    if (cap > MAX_RESPONSE_LEN) cap = MAX_RESPONSE_LEN;
    char* out = (char*)xmalloc(cap);
    size_t out_len = 0;

    const char* p = safe_tmpl;
    while (*p) {
        if (strncmp(p, token, token_len) == 0) {
            size_t need = strlen(name_val);
            if (out_len + need + 1 >= cap) {
                size_t new_cap = cap;
                while (out_len + need + 1 >= new_cap && new_cap < MAX_RESPONSE_LEN) {
                    new_cap *= 2;
                    if (new_cap > MAX_RESPONSE_LEN) new_cap = MAX_RESPONSE_LEN;
                }
                if (new_cap <= cap) { /* cannot grow further */
                    /* Truncate safely */
                    break;
                }
                out = (char*)xrealloc(out, new_cap);
                cap = new_cap;
            }
            memcpy(out + out_len, name_val, need);
            out_len += need;
            p += token_len;
        } else {
            if (out_len + 2 >= cap) {
                size_t new_cap = cap * 2;
                if (new_cap > MAX_RESPONSE_LEN) new_cap = MAX_RESPONSE_LEN;
                if (new_cap <= cap) {
                    break;
                }
                out = (char*)xrealloc(out, new_cap);
                cap = new_cap;
            }
            out[out_len++] = *p++;
        }
        if (out_len >= MAX_RESPONSE_LEN - 1) break;
    }
    out[out_len] = '\0';
    if (*p != '\0') {
        /* Truncation occurred */
        const char* msg = "Error: content too large";
        size_t msg_len = strlen(msg);
        size_t copy = (msg_len < (MAX_RESPONSE_LEN - 1)) ? msg_len : (MAX_RESPONSE_LEN - 1);
        memcpy(out, msg, copy);
        out[copy] = '\0';
    }
    return out;
}

Response render_response_from_env(const Env* env) {
    if (env == NULL) {
        Response r = {400, NULL};
        r.body = (char*)xmalloc(12);
        memcpy(r.body, "Bad Request", 12);
        return r;
    }
    const char* raw = (env->request.name_param != NULL) ? env->request.name_param : "";
    char* sanitized = sanitize_name(raw);
    char* escaped = html_escape(sanitized);
    char* body = render_template(env->template_str, escaped);

    free(sanitized);
    free(escaped);

    Response r = {200, body};
    return r;
}

Response home(const Env* env) {
    return render_response_from_env(env);
}

static void free_response(Response* r) {
    if (!r) return;
    if (r->body) free(r->body);
    r->body = NULL;
}

int main(void) {
    /* Test 1: normal */
    Env e1;
    e1.request.name_param = "Alice";
    e1.template_str = "Hello, {{name}}! Welcome.";
    Response r1 = home(&e1);
    printf("%s\n", r1.body);
    free_response(&r1);

    /* Test 2: contains script tags -> sanitized */
    Env e2;
    e2.request.name_param = "Bob <script>alert(1)</script>";
    e2.template_str = "Hi {{name}}, your input is safe.";
    Response r2 = home(&e2);
    printf("%s\n", r2.body);
    free_response(&r2);

    /* Test 3: empty -> Guest */
    Env e3;
    e3.request.name_param = "   ";
    e3.template_str = "Hello, {{name}}!";
    Response r3 = home(&e3);
    printf("%s\n", r3.body);
    free_response(&r3);

    /* Test 4: very long name */
    char longbuf[151];
    memset(longbuf, 'X', 150);
    longbuf[150] = '\0';
    Env e4;
    e4.request.name_param = longbuf;
    e4.template_str = "Long name test: {{name}}.";
    Response r4 = home(&e4);
    printf("%s\n", r4.body);
    free_response(&r4);

    /* Test 5: null template -> default */
    Env e5;
    e5.request.name_param = "Eve-User_42";
    e5.template_str = NULL;
    Response r5 = home(&e5);
    printf("%s\n", r5.body);
    free_response(&r5);

    return 0;
}