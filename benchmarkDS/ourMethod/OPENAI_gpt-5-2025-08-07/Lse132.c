#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#ifdef _WIN32
#include <direct.h>
#define mkdir_p(path) _mkdir(path)
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

// Validate username: 1..32 length and allowed chars [A-Za-z0-9_-]
int is_valid_username(const char* username) {
    if (username == NULL) return 0;
    size_t len = strlen(username);
    if (len < 1 || len > 32) return 0;
    for (size_t i = 0; i < len; i++) {
        char c = username[i];
        if (!( (c >= 'A' && c <= 'Z') ||
               (c >= 'a' && c <= 'z') ||
               (c >= '0' && c <= '9') ||
               c == '_' || c == '-' )) {
            return 0;
        }
    }
    return 1;
}

// Escape HTML special characters
char* escape_html(const char* in) {
    if (!in) return NULL;
    size_t len = strlen(in);
    // Worst-case expansion per char: '&' -> "&amp;" (5), others less; use 6x as safe upper bound
    size_t cap = len * 6 + 1;
    char* out = (char*)malloc(cap);
    if (!out) return NULL;

    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        char c = in[i];
        const char* rep = NULL;
        switch (c) {
            case '&': rep = "&amp;"; break;
            case '<': rep = "&lt;"; break;
            case '>': rep = "&gt;"; break;
            case '"': rep = "&quot;"; break;
            case '\'': rep = "&#x27;"; break;
            case '/': rep = "&#x2F;"; break;
            default: rep = NULL; break;
        }
        if (rep) {
            size_t rlen = strlen(rep);
            if (j + rlen + 1 > cap) {
                // grow
                size_t newcap = cap * 2 + rlen + 16;
                char* tmp = (char*)realloc(out, newcap);
                if (!tmp) { free(out); return NULL; }
                out = tmp; cap = newcap;
            }
            memcpy(out + j, rep, rlen);
            j += rlen;
        } else {
            if (j + 2 > cap) {
                size_t newcap = cap * 2 + 16;
                char* tmp = (char*)realloc(out, newcap);
                if (!tmp) { free(out); return NULL; }
                out = tmp; cap = newcap;
            }
            out[j++] = c;
        }
    }
    out[j] = '\0';
    return out;
}

// Read entire file up to max_size bytes into buffer; returns malloc'd buffer and sets out_len
char* read_file_limited(const char* path, size_t max_size, size_t* out_len) {
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    long sz = ftell(f);
    if (sz < 0) { fclose(f); return NULL; }
    if (sz > (long)max_size) { fclose(f); return NULL; }
    if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return NULL; }
    size_t size = (size_t)sz;
    char* buf = (char*)malloc(size + 1);
    if (!buf) { fclose(f); return NULL; }
    size_t n = fread(buf, 1, size, f);
    fclose(f);
    if (n != size) { free(buf); return NULL; }
    buf[size] = '\0';
    if (out_len) *out_len = size;
    return buf;
}

// Replace all occurrences of placeholder with replacement; returns malloc'd string
char* replace_all(const char* source, const char* placeholder, const char* replacement) {
    size_t src_len = strlen(source);
    size_t ph_len = strlen(placeholder);
    size_t rep_len = strlen(replacement);

    if (ph_len == 0) {
        char* dup = (char*)malloc(src_len + 1);
        if (dup) memcpy(dup, source, src_len + 1);
        return dup;
    }

    // Count occurrences
    size_t count = 0;
    const char* p = source;
    while ((p = strstr(p, placeholder)) != NULL) {
        count++;
        p += ph_len;
    }

    if (count == 0) {
        char* dup = (char*)malloc(src_len + 1);
        if (dup) memcpy(dup, source, src_len + 1);
        return dup;
    }

    // Allocate result
    size_t out_len = src_len + count * (rep_len - ph_len);
    char* out = (char*)malloc(out_len + 1);
    if (!out) return NULL;

    const char* srcp = source;
    char* outp = out;
    while ((p = strstr(srcp, placeholder)) != NULL) {
        size_t chunk = (size_t)(p - srcp);
        memcpy(outp, srcp, chunk);
        outp += chunk;
        memcpy(outp, replacement, rep_len);
        outp += rep_len;
        srcp = p + ph_len;
    }
    // Tail
    size_t tail = strlen(srcp);
    memcpy(outp, srcp, tail);
    outp += tail;
    *outp = '\0';
    return out;
}

// Render hello template with username
// Returns malloc'd HTML string or an error message malloc'd as well.
char* render_hello(const char* username, const char* template_path) {
    if (!is_valid_username(username)) {
        char* msg = (char*)malloc(17);
        if (msg) strcpy(msg, "Invalid username");
        return msg;
    }
    size_t tpl_len = 0;
    char* tpl = read_file_limited(template_path, 65536, &tpl_len);
    if (!tpl) {
        char* msg = (char*)malloc(18);
        if (msg) strcpy(msg, "Template not found");
        return msg;
    }
    char* esc = escape_html(username);
    if (!esc) { free(tpl); return NULL; }
    const char* placeholder = "{{ username }}";
    char* out = replace_all(tpl, placeholder, esc);
    free(esc);
    free(tpl);
    return out;
}

// Create hello.html template in a temp directory and return its path (malloc'd)
char* create_template_file(void) {
    // Create a temp directory under system temp
    const char* tmp = getenv("TMPDIR");
    if (!tmp) tmp = getenv("TEMP");
    if (!tmp) tmp = "/tmp";
    char dirbuf[512];
    snprintf(dirbuf, sizeof(dirbuf), "%s/%s", tmp, "lse132_c_templates");
#ifdef _WIN32
    mkdir_p(dirbuf);
#else
    mkdir(dirbuf, 0700);
#endif
    char* path = (char*)malloc(512);
    if (!path) return NULL;
    snprintf(path, 512, "%s/hello.html", dirbuf);
    const char* tpl =
        "<!doctype html>\n"
        "<html>\n"
        "<head><meta charset=\"utf-8\"><title>Hello</title></head>\n"
        "<body>\n"
        "  <h1>Hello, {{ username }}</h1>\n"
        "</body>\n"
        "</html>\n";
    FILE* f = fopen(path, "wb");
    if (!f) { free(path); return NULL; }
    size_t w = fwrite(tpl, 1, strlen(tpl), f);
    fclose(f);
    if (w != strlen(tpl)) { free(path); return NULL; }
    return path;
}

int main(void) {
    char* tmpl = create_template_file();
    if (!tmpl) {
        fprintf(stderr, "Failed to create template file\n");
        return 1;
    }

    const char* tests[5] = {"Alice", "bob_123", "user-01", "X", "Z9_9-_"};
    for (int i = 0; i < 5; i++) {
        printf("---- Test: %s ----\n", tests[i]);
        char* out = render_hello(tests[i], tmpl);
        if (out) {
            printf("%s\n", out);
            free(out);
        } else {
            printf("Error rendering\n");
        }
    }

    free(tmpl);
    return 0;
}