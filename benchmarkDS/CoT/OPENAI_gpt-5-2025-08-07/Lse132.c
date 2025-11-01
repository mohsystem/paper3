#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

static const char* TEMPLATE_PATH = "hello.html";

static void ensure_template_file() {
    FILE* f = fopen(TEMPLATE_PATH, "r");
    if (!f) {
        f = fopen(TEMPLATE_PATH, "w");
        if (f) {
            const char* content =
                "<!doctype html>\n"
                "<html>\n"
                "<head><title>Hello</title></head>\n"
                "<body>\n"
                "<h1>Hello, {{ username }}!</h1>\n"
                "</body>\n"
                "</html>\n";
            fputs(content, f);
            fclose(f);
        }
    } else {
        fclose(f);
    }
}

static char* read_file(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buf = (char*)malloc((size_t)len + 1);
    if (!buf) { fclose(f); return NULL; }
    size_t readn = fread(buf, 1, (size_t)len, f);
    buf[readn] = '\0';
    fclose(f);
    return buf;
}

static char* replace_all(const char* haystack, const char* needle, const char* repl) {
    if (!haystack || !needle || !repl || needle[0] == '\0') {
        return NULL;
    }
    size_t hlen = strlen(haystack);
    size_t nlen = strlen(needle);
    size_t rlen = strlen(repl);

    // Count occurrences
    size_t count = 0;
    const char* p = haystack;
    while ((p = strstr(p, needle)) != NULL) {
        count++;
        p += nlen;
    }
    if (count == 0) {
        char* out = (char*)malloc(hlen + 1);
        if (out) strcpy(out, haystack);
        return out;
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
        memcpy(dst, repl, rlen);
        dst += rlen;
        src = p + nlen;
    }
    strcpy(dst, src);
    return out;
}

char* render_template(const char* path, const char* username) {
    char* tpl = read_file(path);
    if (!tpl) {
        char* err = (char*)malloc(64);
        if (err) strcpy(err, "Error loading template");
        return err;
    }
    char* out = replace_all(tpl, "{{ username }}", username);
    char* out2 = replace_all(out, "{{username}}", username);
    char* out3 = replace_all(out2, "{{username }}", username);
    char* out4 = replace_all(out3, "{{ username}}", username);
    free(tpl);
    free(out);
    free(out2);
    free(out3);
    return out4;
}

// Handler corresponding to /hello/<username>
char* hello(const char* username) {
    return render_template(TEMPLATE_PATH, username);
}

char* handle_request(const char* url) {
    regex_t regex;
    regmatch_t pm[2];
    // Pattern: ^/hello/([^/]+)$
    if (regcomp(&regex, "^/hello/\\([^/]+\\)$", REG_EXTENDED) != 0) {
        char* s = (char*)malloc(16);
        if (s) strcpy(s, "Regex error");
        return s;
    }
    int reti = regexec(&regex, url, 2, pm, 0);
    if (reti == 0) {
        int start = pm[1].rm_so;
        int end = pm[1].rm_eo;
        int len = end - start;
        char* uname = (char*)malloc((size_t)len + 1);
        if (!uname) {
            regfree(&regex);
            char* s = (char*)malloc(16);
            if (s) strcpy(s, "Alloc error");
            return s;
        }
        memcpy(uname, url + start, (size_t)len);
        uname[len] = '\0';
        char* resp = hello(uname);
        free(uname);
        regfree(&regex);
        return resp;
    } else {
        regfree(&regex);
        char* s = (char*)malloc(14);
        if (s) strcpy(s, "404 Not Found");
        return s;
    }
}

int main(void) {
    ensure_template_file();
    const char* tests[5] = {
        "/hello/alice",
        "/hello/Bob",
        "/hello/charlie_123",
        "/hello/Dana-XYZ",
        "/hello/Eve"
    };
    for (int i = 0; i < 5; ++i) {
        char* out = handle_request(tests[i]);
        printf("URL: %s\n", tests[i]);
        printf("%s\n", out);
        printf("-----\n");
        free(out);
    }
    return 0;
}