#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char* name;
} Request;

typedef struct {
    const char* template_str;
    Request request;
} Env;

static char* replace_all(const char* str, const char* from, const char* to) {
    if (!str) return strdup("");
    if (!from || from[0] == '\0') return strdup(str);
    if (!to) to = "";

    size_t str_len = strlen(str);
    size_t from_len = strlen(from);
    size_t to_len = strlen(to);

    size_t count = 0;
    const char* p = str;
    while ((p = strstr(p, from)) != NULL) {
        count++;
        p += from_len;
    }

    size_t new_len = str_len + count * (to_len - from_len);
    char* result = (char*)malloc(new_len + 1);
    if (!result) return NULL;

    const char* src = str;
    char* dst = result;
    const char* pos;
    while ((pos = strstr(src, from)) != NULL) {
        size_t bytes = (size_t)(pos - src);
        memcpy(dst, src, bytes);
        dst += bytes;
        memcpy(dst, to, to_len);
        dst += to_len;
        src = pos + from_len;
    }
    strcpy(dst, src);
    return result;
}

char* render_response_from_env(const Env* env) {
    const char* template_str = (env && env->template_str) ? env->template_str : "";
    const char* name = (env && env->request.name) ? env->request.name : "";
    return replace_all(template_str, "{name}", name);
}

char* home(const Env* env) {
    return render_response_from_env(env);
}

int main(void) {
    Env e1 = { "Hello, {name}!", {"Alice"} };
    Env e2 = { "Welcome {name} to the system.", {"Bob"} };
    Env e3 = { "Goodbye, {name}.", {"Charlie"} };
    Env e4 = { "{name}, your order is ready.", {""} };
    Env e5 = { "Hi {name}, hi again {name}!", {"Dana"} };

    char* r1 = home(&e1);
    char* r2 = home(&e2);
    char* r3 = home(&e3);
    char* r4 = home(&e4);
    char* r5 = home(&e5);

    if (r1) { printf("%s\n", r1); free(r1); }
    if (r2) { printf("%s\n", r2); free(r2); }
    if (r3) { printf("%s\n", r3); free(r3); }
    if (r4) { printf("%s\n", r4); free(r4); }
    if (r5) { printf("%s\n", r5); free(r5); }

    return 0;
}