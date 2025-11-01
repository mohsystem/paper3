#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *name; // only "name" parameter is needed
} Request;

typedef struct {
    Request request;
    char *templateStr;
} Env;

typedef struct {
    int status;
    char *body;
} Response;

static char* strdup_safe(const char* s) {
    if (!s) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    size_t len = strlen(s);
    char* out = (char*)malloc(len + 1);
    if (out) memcpy(out, s, len + 1);
    return out;
}

static size_t count_substr(const char* str, const char* sub) {
    if (!str || !sub || !*sub) return 0;
    size_t count = 0;
    const char* p = str;
    size_t sublen = strlen(sub);
    while ((p = strstr(p, sub)) != NULL) {
        count++;
        p += sublen;
    }
    return count;
}

static char* replace_all(const char* str, const char* target, const char* repl) {
    if (!str || !target) return strdup_safe(str);
    if (!*target) return strdup_safe(str);
    if (!repl) repl = "";

    size_t occurrences = count_substr(str, target);
    if (occurrences == 0) {
        return strdup_safe(str);
    }

    size_t str_len = strlen(str);
    size_t target_len = strlen(target);
    size_t repl_len = strlen(repl);
    size_t result_len = str_len + occurrences * (repl_len - target_len);

    char* result = (char*)malloc(result_len + 1);
    if (!result) return NULL;

    const char* src = str;
    char* dst = result;
    const char* pos;
    while ((pos = strstr(src, target)) != NULL) {
        size_t chunk = (size_t)(pos - src);
        memcpy(dst, src, chunk);
        dst += chunk;
        memcpy(dst, repl, repl_len);
        dst += repl_len;
        src = pos + target_len;
    }
    // copy the remainder
    size_t tail = strlen(src);
    memcpy(dst, src, tail);
    dst += tail;
    *dst = '\0';

    return result;
}

Response render_response_from_env(const Env* env) {
    const char* name = (env && env->request.name && env->request.name[0] != '\0')
                       ? env->request.name
                       : "World";
    const char* tpl = (env && env->templateStr) ? env->templateStr : "";
    char* rendered = replace_all(tpl, "{{name}}", name);
    Response res;
    res.status = 200;
    res.body = rendered ? rendered : strdup_safe("");
    return res;
}

Response home(const Env* env) {
    return render_response_from_env(env);
}

static void free_response(Response* res) {
    if (res && res->body) {
        free(res->body);
        res->body = NULL;
    }
}

int main() {
    // Test case 1
    Env e1;
    e1.request.name = "Alice";
    e1.templateStr = "Hello, {{name}}!";
    Response r1 = home(&e1);
    printf("%s\n", r1.body);
    free_response(&r1);

    // Test case 2
    Env e2;
    e2.request.name = "Bob";
    e2.templateStr = "Welcome, {{name}}.";
    Response r2 = home(&e2);
    printf("%s\n", r2.body);
    free_response(&r2);

    // Test case 3 (empty name -> default)
    Env e3;
    e3.request.name = "";
    e3.templateStr = "Hi, {{name}}";
    Response r3 = home(&e3);
    printf("%s\n", r3.body);
    free_response(&r3);

    // Test case 4 (multiple occurrences)
    Env e4;
    e4.request.name = "Eve";
    e4.templateStr = "User: {{name}} | Echo: {{name}}";
    Response r4 = home(&e4);
    printf("%s\n", r4.body);
    free_response(&r4);

    // Test case 5 (no name provided -> default)
    Env e5;
    e5.request.name = NULL;
    e5.templateStr = "Greetings, {{name}}!";
    Response r5 = home(&e5);
    printf("%s\n", r5.body);
    free_response(&r5);

    return 0;
}