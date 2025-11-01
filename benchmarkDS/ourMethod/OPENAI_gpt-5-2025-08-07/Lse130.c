#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_NAME_LEN 100
#define MAX_TEMPLATE_LEN 4096

typedef struct {
    char *template_str;
    char *request_name;
} Env;

typedef struct {
    int status;
    char *body;
} Response;

static size_t safe_strnlen(const char *s, size_t maxlen) {
    if (!s) return 0;
    size_t i = 0;
    for (; i < maxlen && s[i] != '\0'; ++i) { /* loop */ }
    return i;
}

static char *sanitize_name(const char *name) {
    char tmp[MAX_NAME_LEN + 1];
    size_t out_i = 0;

    if (name != NULL) {
        size_t nlen = safe_strnlen(name, MAX_NAME_LEN);
        for (size_t i = 0; i < nlen && out_i < MAX_NAME_LEN; ++i) {
            unsigned char c = (unsigned char)name[i];
            if (isalnum(c) || c == ' ' || c == '-' || c == '_' || c == '.' || c == ',') {
                tmp[out_i++] = (char)c;
            }
        }
        // trim spaces from both ends
        size_t start = 0;
        while (start < out_i && isspace((unsigned char)tmp[start])) start++;
        size_t end = out_i;
        while (end > start && isspace((unsigned char)tmp[end - 1])) end--;
        out_i = (end > start) ? (end - start) : 0;
        if (start > 0 && out_i > 0) {
            memmove(tmp, tmp + start, out_i);
        }
    }

    if (out_i == 0) {
        const char *guest = "Guest";
        size_t glen = strlen(guest);
        char *res = (char *)malloc(glen + 1);
        if (!res) return NULL;
        memcpy(res, guest, glen + 1);
        return res;
    } else {
        tmp[out_i] = '\0';
        char *res = (char *)malloc(out_i + 1);
        if (!res) return NULL;
        memcpy(res, tmp, out_i + 1);
        return res;
    }
}

static char *escape_html(const char *in) {
    if (!in) {
        char *empty = (char *)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    // Calculate required size
    size_t inlen = safe_strnlen(in, MAX_NAME_LEN);
    size_t outlen = 0;
    for (size_t i = 0; i < inlen; ++i) {
        switch (in[i]) {
            case '&': outlen += 5; break;      // &amp;
            case '<': outlen += 4; break;      // &lt;
            case '>': outlen += 4; break;      // &gt;
            case '"': outlen += 6; break;      // &quot;
            case '\'': outlen += 5; break;     // &#39;
            default: outlen += 1; break;
        }
    }
    // Safety cap (worst case 6x for quotes), ensure not too large
    if (outlen > (MAX_NAME_LEN * 6 + 1)) {
        return NULL;
    }
    char *out = (char *)malloc(outlen + 1);
    if (!out) return NULL;

    size_t j = 0;
    for (size_t i = 0; i < inlen; ++i) {
        switch (in[i]) {
            case '&': memcpy(out + j, "&amp;", 5); j += 5; break;
            case '<': memcpy(out + j, "&lt;", 4); j += 4; break;
            case '>': memcpy(out + j, "&gt;", 4); j += 4; break;
            case '"': memcpy(out + j, "&quot;", 6); j += 6; break;
            case '\'': memcpy(out + j, "&#39;", 5); j += 5; break;
            default: out[j++] = in[i]; break;
        }
    }
    out[j] = '\0';
    return out;
}

static char *render_template(const char *template_str, const char *escaped_name) {
    if (!template_str || !escaped_name) {
        char *msg = (char *)malloc(1);
        if (msg) msg[0] = '\0';
        return msg;
    }
    size_t tlen = safe_strnlen(template_str, MAX_TEMPLATE_LEN + 1);
    if (tlen > MAX_TEMPLATE_LEN) {
        const char *invalid = "Invalid template";
        size_t ilen = strlen(invalid);
        char *res = (char *)malloc(ilen + 1);
        if (!res) return NULL;
        memcpy(res, invalid, ilen + 1);
        return res;
    }
    const char *ph = "{name}";
    const size_t phlen = 6; // includes braces
    size_t nlen = strlen(escaped_name);

    // Count occurrences
    size_t count = 0;
    for (size_t i = 0; i + phlen <= tlen; ) {
        if (memcmp(template_str + i, ph, phlen) == 0) {
            count++;
            i += phlen;
        } else {
            i++;
        }
    }

    // Compute final length with overflow checks
    size_t extra_per = (nlen >= phlen) ? (nlen - phlen) : 0;
    if (count > 0) {
        // Check overflow: tlen + count * extra_per
        if (extra_per > 0 && count > ((SIZE_MAX - tlen) / extra_per)) {
            return NULL;
        }
    }
    size_t final_len = tlen + count * extra_per;

    char *out = (char *)malloc(final_len + 1);
    if (!out) return NULL;

    size_t oi = 0;
    size_t i = 0;
    while (i < tlen) {
        if (i + phlen <= tlen && memcmp(template_str + i, ph, phlen) == 0) {
            memcpy(out + oi, escaped_name, nlen);
            oi += nlen;
            i += phlen;
        } else {
            out[oi++] = template_str[i++];
        }
    }
    out[oi] = '\0';
    return out;
}

Response render_response_from_env(const Env *env) {
    Response r;
    r.status = 200;
    r.body = NULL;

    if (!env) {
        r.status = 400;
        r.body = (char *)malloc(19);
        if (r.body) memcpy(r.body, "Invalid environment", 19);
        return r;
    }

    char *san = sanitize_name(env->request_name);
    if (!san) {
        r.status = 500;
        r.body = (char *)malloc(16);
        if (r.body) memcpy(r.body, "Internal Failure", 16);
        return r;
    }

    char *esc = escape_html(san);
    free(san);
    if (!esc) {
        r.status = 500;
        r.body = (char *)malloc(16);
        if (r.body) memcpy(r.body, "Internal Failure", 16);
        return r;
    }

    char *rendered = render_template(env->template_str, esc);
    free(esc);
    if (!rendered) {
        r.status = 500;
        r.body = (char *)malloc(16);
        if (r.body) memcpy(r.body, "Internal Failure", 16);
        return r;
    }

    r.body = rendered;
    return r;
}

Response home(const Env *env) {
    return render_response_from_env(env);
}

static void print_response(const char *label, const Response *r) {
    if (!r) return;
    printf("%s -> Status: %d, Body: %s\n", label, r->status, r->body ? r->body : "");
}

int main(void) {
    Env t1 = { "Hello, {name}!", "Alice" };
    Env t2 = { "<div>User: {name}</div>", "Bob <script>alert(1)</script>" };
    char longname[501];
    for (int i = 0; i < 500; ++i) longname[i] = 'a';
    longname[500] = '\0';
    Env t3 = { "Len test: {name}", longname };
    Env t4 = { "Safe SQL-ish: {name}", "Eve; DROP TABLE users;--" };
    Env t5 = { "Welcome!", NULL };

    Response r1 = home(&t1);
    Response r2 = home(&t2);
    Response r3 = home(&t3);
    Response r4 = home(&t4);
    Response r5 = home(&t5);

    print_response("Test 1", &r1);
    print_response("Test 2", &r2);
    print_response("Test 3", &r3);
    print_response("Test 4", &r4);
    print_response("Test 5", &r5);

    free(r1.body);
    free(r2.body);
    free(r3.body);
    free(r4.body);
    free(r5.body);

    return 0;
}