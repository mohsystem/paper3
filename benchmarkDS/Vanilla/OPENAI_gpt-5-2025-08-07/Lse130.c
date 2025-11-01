#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Request {
    const char* name;
};

struct Environment {
    struct Request request;
    const char* template_str;
};

struct Response {
    char* body;
};

static char* render_template(const char* tmpl, const char* name) {
    if (!tmpl) tmpl = "";
    const char* ph = "{name}";
    size_t ph_len = 6;
    if (!name || name[0] == '\0') name = "World";

    // Count occurrences
    size_t count = 0;
    const char* p = tmpl;
    while ((p = strstr(p, ph)) != NULL) {
        count++;
        p += ph_len;
    }

    size_t tmpl_len = strlen(tmpl);
    size_t name_len = strlen(name);
    size_t out_len = tmpl_len + count * (name_len - ph_len);

    char* out = (char*)malloc(out_len + 1);
    if (!out) return NULL;

    const char* src = tmpl;
    char* dst = out;
    while (1) {
        const char* found = strstr(src, ph);
        if (!found) {
            size_t rem = strlen(src);
            memcpy(dst, src, rem);
            dst += rem;
            break;
        }
        size_t prefix_len = (size_t)(found - src);
        memcpy(dst, src, prefix_len);
        dst += prefix_len;
        memcpy(dst, name, name_len);
        dst += name_len;
        src = found + ph_len;
    }
    *dst = '\0';
    return out;
}

struct Response render_response_from_env(struct Environment env) {
    const char* tmpl = env.template_str ? env.template_str : "";
    const char* nm = env.request.name;
    char* rendered = render_template(tmpl, nm);
    struct Response resp;
    resp.body = rendered ? rendered : strdup("");
    return resp;
}

struct Response home(struct Environment env) {
    return render_response_from_env(env);
}

int main(void) {
    struct Environment tests[5] = {
        { .request = { "Alice" }, .template_str = "Hello, {name}!" },
        { .request = { "Bob" }, .template_str = "Welcome, {name}." },
        { .request = { "" }, .template_str = "Hi {name}" },
        { .request = { "Charlie" }, .template_str = "{name} logged in" },
        { .request = { "Dana" }, .template_str = "Bye, {name}! See you." }
    };

    for (int i = 0; i < 5; ++i) {
        struct Response r = home(tests[i]);
        printf("%s\n", r.body);
        free(r.body);
    }
    return 0;
}