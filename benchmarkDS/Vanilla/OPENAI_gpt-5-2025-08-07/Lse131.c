#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Env {
    char* tpl;
};

struct Request {
    const char* name;
};

struct Response {
    char* body;
};

struct Env env;
struct Request* current_request = NULL;

static char* render_template(const char* tpl, const char* name) {
    const char* key = "{{name}}";
    if (!tpl) tpl = "";
    if (!name) name = "";
    size_t keylen = strlen(key);
    size_t namelen = strlen(name);

    // Count occurrences
    size_t count = 0;
    const char* p = tpl;
    while ((p = strstr(p, key)) != NULL) {
        count++;
        p += keylen;
    }

    size_t tpllen = strlen(tpl);
    size_t outlen = tpllen + count * (namelen - keylen);
    char* out = (char*)malloc(outlen + 1);
    if (!out) return NULL;

    const char* src = tpl;
    char* dst = out;
    const char* pos;
    while ((pos = strstr(src, key)) != NULL) {
        size_t segment = (size_t)(pos - src);
        memcpy(dst, src, segment);
        dst += segment;
        if (namelen > 0) {
            memcpy(dst, name, namelen);
            dst += namelen;
        }
        src = pos + keylen;
    }
    // Copy remainder
    size_t remainder = strlen(src);
    memcpy(dst, src, remainder);
    dst += remainder;
    *dst = '\0';
    return out;
}

static const char* env_get_template(struct Env* environment) {
    return environment ? environment->tpl : "";
}

struct Response render_response_from_env(struct Env* environment) {
    const char* name = (current_request && current_request->name) ? current_request->name : "";
    const char* tpl = env_get_template(environment);
    char* rendered = render_template(tpl, name);
    struct Response r;
    r.body = rendered ? rendered : strdup("");
    return r;
}

struct Response home() {
    return render_response_from_env(&env);
}

int main(void) {
    env.tpl = strdup("Hello, {{name}}!");
    const char* names[5] = {"Alice", "Bob", "Charlie", "Dana", "Eve"};
    for (int i = 0; i < 5; ++i) {
        struct Request req;
        req.name = names[i];
        current_request = &req;
        struct Response resp = home();
        printf("%s\n", resp.body);
        free(resp.body);
        current_request = NULL;
    }
    free(env.tpl);
    return 0;
}