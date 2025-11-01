#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    char **keys;
    char **values;
    size_t count;
} Request;

typedef struct {
    char *templateText;
} TemplateT;

typedef struct {
    Request request;
    TemplateT tmpl;
} Environment;

typedef struct {
    int status;
    char *body;
} Response;

static char* strdup_safe(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, s, len + 1);
    return out;
}

static const char* request_get_param(const Request* req, const char* key) {
    if (!req || !key) return NULL;
    for (size_t i = 0; i < req->count; ++i) {
        if (req->keys[i] && strcmp(req->keys[i], key) == 0) {
            return req->values[i];
        }
    }
    return NULL;
}

static char* html_escape(const char* s) {
    if (!s) {
        char* out = (char*)malloc(1);
        if (out) out[0] = '\0';
        return out;
    }
    size_t extra = 0;
    for (const char* p = s; *p; ++p) {
        switch (*p) {
            case '&': extra += 4; break;      // &amp; -> 5-1
            case '<': extra += 3; break;      // &lt; -> 4-1
            case '>': extra += 3; break;      // &gt; -> 4-1
            case '"': extra += 5; break;      // &quot; -> 6-1
            case '\'': extra += 5; break;     // &#x27; -> 6-1
            case '/': extra += 5; break;      // &#x2F; -> 6-1
            default: break;
        }
    }
    size_t len = strlen(s);
    char* out = (char*)malloc(len + extra + 1);
    if (!out) return NULL;
    char* o = out;
    for (const char* p = s; *p; ++p) {
        switch (*p) {
            case '&': memcpy(o, "&amp;", 5); o += 5; break;
            case '<': memcpy(o, "&lt;", 4); o += 4; break;
            case '>': memcpy(o, "&gt;", 4); o += 4; break;
            case '"': memcpy(o, "&quot;", 6); o += 6; break;
            case '\'': memcpy(o, "&#x27;", 6); o += 6; break;
            case '/': memcpy(o, "&#x2F;", 6); o += 6; break;
            default: *o++ = *p; break;
        }
    }
    *o = '\0';
    return out;
}

static char* sanitize_name(const char* name) {
    const char* n = name ? name : "";
    // Trim
    while (isspace((unsigned char)*n)) n++;
    const char* end = n + strlen(n);
    while (end > n && isspace((unsigned char)*(end - 1))) end--;
    size_t len = (size_t)(end - n);
    // Remove control characters and limit length to 100
    char* tmp = (char*)malloc(101 + 1);
    if (!tmp) {
        return strdup_safe("Guest");
    }
    size_t written = 0;
    for (size_t i = 0; i < len && written < 100; ++i) {
        unsigned char c = (unsigned char)n[i];
        if (c >= 32 && c != 127) {
            tmp[written++] = (char)c;
        }
    }
    if (written == 0) {
        free(tmp);
        return strdup_safe("Guest");
    }
    tmp[written] = '\0';
    return tmp;
}

static char* render_template(const char* templateText, const char* nameVal) {
    if (!templateText) templateText = "";
    const char* token = "{{name}}";
    const char* pos = strstr(templateText, token);
    char* escaped = html_escape(nameVal ? nameVal : "");
    if (!escaped) escaped = strdup_safe("");
    if (!pos) {
        // No token found
        return strdup_safe(templateText);
    }
    size_t beforeLen = (size_t)(pos - templateText);
    size_t tokenLen = strlen(token);
    size_t afterLen = strlen(templateText) - beforeLen - tokenLen;
    size_t escapedLen = strlen(escaped);
    size_t newLen = beforeLen + escapedLen + afterLen;
    char* out = (char*)malloc(newLen + 1);
    if (!out) {
        free(escaped);
        return NULL;
    }
    memcpy(out, templateText, beforeLen);
    memcpy(out + beforeLen, escaped, escapedLen);
    memcpy(out + beforeLen + escapedLen, pos + tokenLen, afterLen);
    out[newLen] = '\0';
    free(escaped);
    return out;
}

static Response render_response_from_env(const Environment* env) {
    if (!env) {
        Response r = {500, strdup_safe("Internal Server Error")};
        return r;
    }
    const char* rawName = request_get_param(&env->request, "name");
    char* safeName = sanitize_name(rawName);
    if (!env->tmpl.templateText) {
        free(safeName);
        Response r = {500, strdup_safe("Template not found")};
        return r;
    }
    char* rendered = render_template(env->tmpl.templateText, safeName);
    free(safeName);
    Response r;
    r.status = 200;
    r.body = rendered ? rendered : strdup_safe("");
    return r;
}

static Response home(const Environment* env) {
    return render_response_from_env(env);
}

// Utilities for creating a Request
static Request make_request(size_t count, const char** keys, const char** values) {
    Request req;
    req.count = count;
    req.keys = (char**)calloc(count, sizeof(char*));
    req.values = (char**)calloc(count, sizeof(char*));
    for (size_t i = 0; i < count; ++i) {
        req.keys[i] = strdup_safe(keys[i]);
        req.values[i] = strdup_safe(values[i]);
    }
    return req;
}

static void free_request(Request* req) {
    if (!req) return;
    for (size_t i = 0; i < req->count; ++i) {
        free(req->keys[i]);
        free(req->values[i]);
    }
    free(req->keys);
    free(req->values);
    req->keys = NULL;
    req->values = NULL;
    req->count = 0;
}

int main(void) {
    // Common template
    TemplateT tpl;
    tpl.templateText = strdup_safe("Hello, {{name}}!");

    // Test 1: Normal name
    const char* k1[] = {"name"};
    const char* v1[] = {"Alice"};
    Environment e1;
    e1.request = make_request(1, k1, v1);
    e1.tmpl = tpl;
    Response r1 = home(&e1);
    printf("Status: %d, Body: %s\n", r1.status, r1.body);
    free(r1.body);
    free_request(&e1.request);

    // Test 2: Missing name
    Environment e2;
    e2.request = make_request(0, NULL, NULL);
    e2.tmpl = tpl;
    Response r2 = home(&e2);
    printf("Status: %d, Body: %s\n", r2.status, r2.body);
    free(r2.body);
    free_request(&e2.request);

    // Test 3: XSS attempt
    const char* k3[] = {"name"};
    const char* v3[] = {"<script>alert(1)</script>"};
    Environment e3;
    e3.request = make_request(1, k3, v3);
    e3.tmpl = tpl;
    Response r3 = home(&e3);
    printf("Status: %d, Body: %s\n", r3.status, r3.body);
    free(r3.body);
    free_request(&e3.request);

    // Test 4: Whitespace and control chars
    const char* k4[] = {"name"};
    const char* v4[] = {"  Bob \n\t "};
    Environment e4;
    e4.request = make_request(1, k4, v4);
    e4.tmpl = tpl;
    Response r4 = home(&e4);
    printf("Status: %d, Body: %s\n", r4.status, r4.body);
    free(r4.body);
    free_request(&e4.request);

    // Test 5: Long name
    char* longName = (char*)malloc(201);
    if (longName) {
        for (int i = 0; i < 200; ++i) longName[i] = 'A';
        longName[200] = '\0';
    }
    const char* k5[] = {"name"};
    const char* v5[] = { longName ? longName : "A" };
    Environment e5;
    e5.request = make_request(1, k5, v5);
    e5.tmpl = tpl;
    Response r5 = home(&e5);
    printf("Status: %d, Body: %s\n", r5.status, r5.body);
    free(r5.body);
    free_request(&e5.request);
    free(longName);

    free(tpl.templateText);
    return 0;
}