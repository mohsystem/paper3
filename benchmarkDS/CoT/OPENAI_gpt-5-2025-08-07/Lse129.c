#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char* name;
} Request;

typedef struct {
    const char* templ;
    Request request;
} Environment;

typedef struct {
    char* content;
} Response;

static const char* env_get_template(const Environment* env) {
    return env->templ;
}

static char* replace_all(const char* haystack, const char* needle, const char* replacement) {
    if (!haystack || !needle || !replacement) {
        return NULL;
    }
    size_t hlen = strlen(haystack);
    size_t nlen = strlen(needle);
    size_t rlen = strlen(replacement);

    if (nlen == 0) {
        char* out = (char*)malloc(hlen + 1);
        if (out) strcpy(out, haystack);
        return out;
    }

    // Count occurrences
    size_t count = 0;
    const char* p = haystack;
    while ((p = strstr(p, needle)) != NULL) {
        count++;
        p += nlen;
    }

    size_t newlen = hlen + count * (rlen - nlen);
    char* out = (char*)malloc(newlen + 1);
    if (!out) return NULL;

    const char* src = haystack;
    char* dst = out;
    while ((p = strstr(src, needle)) != NULL) {
        size_t seglen = (size_t)(p - src);
        memcpy(dst, src, seglen);
        dst += seglen;
        memcpy(dst, replacement, rlen);
        dst += rlen;
        src = p + nlen;
    }
    // copy remainder
    size_t remainder = strlen(src);
    memcpy(dst, src, remainder);
    dst += remainder;
    *dst = '\0';
    return out;
}

static char* render_template(const char* templ, const char* name) {
    if (!templ) {
        char* out = (char*)malloc(1);
        if (out) out[0] = '\0';
        return out;
    }
    const char* safe_name = name ? name : "";
    char* step1 = replace_all(templ, "{{ name }}", safe_name);
    if (!step1) return NULL;
    char* step2 = replace_all(step1, "{{name}}", safe_name);
    free(step1);
    return step2;
}

Response render_response_from_env(const Environment* env) {
    const char* name = env && env->request.name ? env->request.name : "";
    const char* templ = env_get_template(env);
    char* rendered = render_template(templ, name);
    Response res;
    res.content = rendered ? rendered : strdup("");
    return res;
}

Response home(const Environment* env) {
    return render_response_from_env(env);
}

int main(void) {
    // Test case 1
    {
        Environment env = { "Hello, {{ name }}!", { "Alice" } };
        Response res = home(&env);
        printf("%s\n", res.content);
        free(res.content);
    }
    // Test case 2
    {
        Environment env = { "Welcome, {{name}}.", { "Bob" } };
        Response res = home(&env);
        printf("%s\n", res.content);
        free(res.content);
    }
    // Test case 3
    {
        Environment env = { "Hi {{ name }}, you're number {{ name }}", { "Eve" } };
        Response res = home(&env);
        printf("%s\n", res.content);
        free(res.content);
    }
    // Test case 4
    {
        Environment env = { "No placeholder here.", { "Carol" } };
        Response res = home(&env);
        printf("%s\n", res.content);
        free(res.content);
    }
    // Test case 5
    {
        Environment env = { "{{ name }} only", { NULL } };
        Response res = home(&env);
        printf("%s\n", res.content);
        free(res.content);
    }
    return 0;
}