#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char* name;
} Request;

typedef struct {
    Request request;
    const char* templ;
} Env;

typedef struct {
    char* body;
} Response;

static char* replace_all(const char* src, const char* from, const char* to) {
    if (!src || !from || !to || from[0] == '\0') {
        char* dup = (char*)malloc((src ? strlen(src) : 0) + 1);
        if (dup) strcpy(dup, src ? src : "");
        return dup;
    }
    size_t src_len = strlen(src);
    size_t from_len = strlen(from);
    size_t to_len = strlen(to);

    // Count occurrences
    size_t count = 0;
    const char* p = src;
    while ((p = strstr(p, from)) != NULL) {
        count++;
        p += from_len;
    }

    size_t new_len = src_len + count * (to_len - from_len);
    char* out = (char*)malloc(new_len + 1);
    if (!out) return NULL;

    const char* s = src;
    char* o = out;
    while ((p = strstr(s, from)) != NULL) {
        size_t seg_len = (size_t)(p - s);
        memcpy(o, s, seg_len);
        o += seg_len;
        memcpy(o, to, to_len);
        o += to_len;
        s = p + from_len;
    }
    // Copy remainder
    size_t rem = strlen(s);
    memcpy(o, s, rem);
    o += rem;
    *o = '\0';
    return out;
}

static char* render_template(const char* templ, const char* name) {
    const char* safe_name = name ? name : "";
    char* step1 = replace_all(templ ? templ : "", "{{name}}", safe_name);
    char* final = replace_all(step1 ? step1 : "", "{name}", safe_name);
    if (step1) free(step1);
    return final;
}

Response render_response_from_env(const Env* env) {
    const char* name = (env && env->request.name) ? env->request.name : "";
    const char* templ = env ? env->templ : "";
    char* rendered = render_template(templ, name);
    Response r;
    r.body = rendered ? rendered : strdup("");
    return r;
}

Response home(const Env* env) {
    return render_response_from_env(env);
}

int main(void) {
    Env tests[5];
    tests[0].request.name = "Alice";   tests[0].templ = "Hello, {{name}}!";
    tests[1].request.name = "Bob";     tests[1].templ = "Welcome, {name}.";
    tests[2].request.name = "Charlie"; tests[2].templ = "Hi {name}, this is a test.";
    tests[3].request.name = "Dana";    tests[3].templ = "Dear {name}, your order is ready.";
    tests[4].request.name = "Eve";     tests[4].templ = "Goodbye, {{name}}.";

    for (int i = 0; i < 5; ++i) {
        Response r = home(&tests[i]);
        printf("Test %d: %s\n", i + 1, r.body);
        free(r.body);
    }
    return 0;
}