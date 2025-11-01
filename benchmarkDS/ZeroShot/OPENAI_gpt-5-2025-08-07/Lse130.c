#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

struct Request {
    const char* name;
};

struct Env {
    struct Request request;
    const char* templateStr;
};

static char* html_escape(const char* input) {
    if (input == NULL) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    size_t len = 0;
    for (const char* p = input; *p; ++p) {
        switch (*p) {
            case '&': len += 5; break;      // &amp;
            case '<': len += 4; break;      // &lt;
            case '>': len += 4; break;      // &gt;
            case '"': len += 6; break;      // &quot;
            case '\'': len += 6; break;     // &#x27;
            default: len += 1; break;
        }
    }
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    char* w = out;
    for (const char* p = input; *p; ++p) {
        switch (*p) {
            case '&': memcpy(w, "&amp;", 5); w += 5; break;
            case '<': memcpy(w, "&lt;", 4); w += 4; break;
            case '>': memcpy(w, "&gt;", 4); w += 4; break;
            case '"': memcpy(w, "&quot;", 6); w += 6; break;
            case '\'': memcpy(w, "&#x27;", 6); w += 6; break;
            default: *w++ = *p; break;
        }
    }
    *w = '\0';
    return out;
}

static char* replace_all(const char* text, const char* from, const char* to) {
    if (!text || !from || !to) return NULL;
    size_t from_len = strlen(from);
    if (from_len == 0) {
        char* dup = (char*)malloc(strlen(text) + 1);
        if (dup) strcpy(dup, text);
        return dup;
    }
    size_t to_len = strlen(to);
    // Count occurrences
    size_t count = 0;
    const char* p = text;
    while ((p = strstr(p, from)) != NULL) {
        count++;
        p += from_len;
    }
    size_t text_len = strlen(text);
    // Check for overflow
    if (count > SIZE_MAX / (to_len > from_len ? (to_len - from_len) : 1)) {
        return NULL;
    }
    size_t new_len = text_len + count * (to_len - from_len);
    char* out = (char*)malloc(new_len + 1);
    if (!out) return NULL;

    const char* src = text;
    char* dst = out;
    const char* pos;
    while ((pos = strstr(src, from)) != NULL) {
        size_t chunk = (size_t)(pos - src);
        memcpy(dst, src, chunk);
        dst += chunk;
        memcpy(dst, to, to_len);
        dst += to_len;
        src = pos + from_len;
    }
    // Copy remainder
    size_t rem = strlen(src);
    memcpy(dst, src, rem);
    dst += rem;
    *dst = '\0';
    return out;
}

char* render_response_from_env(const struct Env* env) {
    if (!env) {
        char* msg = (char*)malloc(18);
        if (msg) strcpy(msg, "Invalid environment");
        return msg;
    }
    const char* name = env->request.name ? env->request.name : "World";
    const char* tmpl = env->templateStr ? env->templateStr : "Hello, {{name}}!";
    const size_t MAX_TEMPLATE_LEN = 1000000;
    if (strlen(tmpl) > MAX_TEMPLATE_LEN) {
        char* msg = (char*)malloc(18);
        if (msg) strcpy(msg, "Template too large");
        return msg;
    }
    char* safe_name = html_escape(name);
    if (!safe_name) return NULL;
    char* rendered = replace_all(tmpl, "{{name}}", safe_name);
    free(safe_name);
    if (!rendered) {
        return NULL;
    }
    return rendered;
}

char* home(const struct Env* env) {
    return render_response_from_env(env);
}

int main(void) {
    // Test case 1: Normal name with default template
    struct Env env1;
    env1.request.name = "Alice";
    env1.templateStr = "Hello, {{name}}!";
    char* out1 = home(&env1);
    if (out1) { printf("%s\n", out1); free(out1); }

    // Test case 2: Missing name parameter -> default "World"
    struct Env env2;
    env2.request.name = NULL;
    env2.templateStr = "Welcome, {{name}}.";
    char* out2 = home(&env2);
    if (out2) { printf("%s\n", out2); free(out2); }

    // Test case 3: Name with HTML that should be escaped
    struct Env env3;
    env3.request.name = "<script>alert(1)</script>";
    env3.templateStr = "<div>User: {{name}}</div>";
    char* out3 = home(&env3);
    if (out3) { printf("%s\n", out3); free(out3); }

    // Test case 4: Missing template -> default template used
    struct Env env4;
    env4.request.name = "Bob";
    env4.templateStr = NULL;
    char* out4 = home(&env4);
    if (out4) { printf("%s\n", out4); free(out4); }

    // Test case 5: Multiple placeholders
    struct Env env5;
    env5.request.name = "Charlie";
    env5.templateStr = "Hi {{name}}, bye {{name}}!";
    char* out5 = home(&env5);
    if (out5) { printf("%s\n", out5); free(out5); }

    return 0;
}