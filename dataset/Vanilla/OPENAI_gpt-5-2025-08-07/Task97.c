#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    char* data;
    size_t len;
    size_t cap;
} StrBuf;

static void sb_init(StrBuf* sb) {
    sb->len = 0;
    sb->cap = 256;
    sb->data = (char*)malloc(sb->cap);
    if (sb->data) sb->data[0] = '\0';
}
static void sb_free(StrBuf* sb) {
    free(sb->data);
    sb->data = NULL;
    sb->len = sb->cap = 0;
}
static void sb_reserve(StrBuf* sb, size_t need) {
    if (need <= sb->cap) return;
    while (sb->cap < need) sb->cap *= 2;
    sb->data = (char*)realloc(sb->data, sb->cap);
}
static void sb_append_len(StrBuf* sb, const char* s, size_t n) {
    sb_reserve(sb, sb->len + n + 1);
    memcpy(sb->data + sb->len, s, n);
    sb->len += n;
    sb->data[sb->len] = '\0';
}
static void sb_append_cstr(StrBuf* sb, const char* s) {
    sb_append_len(sb, s, strlen(s));
}
static int read_file_to_string(const char* path, char** out, size_t* out_len) {
    FILE* f = fopen(path, "rb");
    if (!f) return 0;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    if (sz < 0) { fclose(f); return 0; }
    fseek(f, 0, SEEK_SET);
    char* buf = (char*)malloc((size_t)sz + 1);
    if (!buf) { fclose(f); return 0; }
    size_t rd = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    buf[rd] = '\0';
    *out = buf;
    *out_len = rd;
    return 1;
}
static int write_sample_file(const char* path) {
    const char* xml =
        "<root>\n"
        "  <tag id=\"1\">Content one</tag>\n"
        "  <tag id=\"2\">\n"
        "     Two\n"
        "     <child>Alpha</child>\n"
        "  </tag>\n"
        "  <tag id=\"3\"/>\n"
        "  <other id=\"1\">Not a tag</other>\n"
        "  <tag id=\"nested\">\n"
        "     <tag id=\"inner\">Inner content</tag>\n"
        "  </tag>\n"
        "  <tag id=\"spaces\">  spaced  </tag>\n"
        "</root>\n";
    FILE* f = fopen(path, "wb");
    if (!f) return 0;
    fwrite(xml, 1, strlen(xml), f);
    fclose(f);
    return 1;
}

static long find_substr_from(const char* s, long start, const char* sub) {
    const char* p = strstr(s + start, sub);
    if (!p) return -1;
    return (long)(p - s);
}
static long find_char_from(const char* s, long start, char c) {
    const char* p = strchr(s + start, c);
    if (!p) return -1;
    return (long)(p - s);
}
static int is_alnum_c(char c) { return isalnum((unsigned char)c); }

static long index_of_word(const char* s, const char* w, long start) {
    size_t wl = strlen(w);
    long i = start - 1;
    while (1) {
        const char* p = strstr(s + (i + 1), w);
        if (!p) return -1;
        i = (long)(p - s);
        int leftOk = (i == 0) || !is_alnum_c(s[i - 1]);
        long end = i + (long)wl;
        int rightOk = (s[end] == '\0') || !is_alnum_c(s[end]);
        if (leftOk && rightOk) return i;
    }
}

static char* extract_attr_value(const char* attrs, const char* name) {
    long len = (long)strlen(attrs);
    long idx = index_of_word(attrs, name, 0);
    while (idx != -1) {
        long j = idx + (long)strlen(name);
        while (j < len && isspace((unsigned char)attrs[j])) j++;
        if (j < len && attrs[j] == '=') {
            j++;
            while (j < len && isspace((unsigned char)attrs[j])) j++;
            if (j < len) {
                char q = attrs[j];
                if (q == '"' || q == '\'') {
                    long k = j + 1;
                    while (k < len && attrs[k] != q) k++;
                    if (k < len) {
                        long valLen = k - (j + 1);
                        char* out = (char*)malloc((size_t)valLen + 1);
                        if (!out) return NULL;
                        memcpy(out, attrs + j + 1, (size_t)valLen);
                        out[valLen] = '\0';
                        return out;
                    }
                } else {
                    long k = j;
                    while (k < len && !isspace((unsigned char)attrs[k])) k++;
                    long valLen = k - j;
                    char* out = (char*)malloc((size_t)valLen + 1);
                    if (!out) return NULL;
                    memcpy(out, attrs + j, (size_t)valLen);
                    out[valLen] = '\0';
                    return out;
                }
            }
        }
        idx = index_of_word(attrs, name, idx + 1);
    }
    return NULL;
}

static long find_matching_end(const char* s, long from) {
    int depth = 1;
    long search = from;
    while (depth > 0) {
        long nextOpen = find_substr_from(s, search, "<tag");
        long nextClose = find_substr_from(s, search, "</tag>");
        if (nextClose == -1) return -1;
        if (nextOpen != -1 && nextOpen < nextClose) {
            long gt2 = find_char_from(s, nextOpen, '>');
            if (gt2 == -1) return -1;
            int selfClosing = gt2 > nextOpen && s[gt2 - 1] == '/';
            if (!selfClosing) depth++;
            search = gt2 + 1;
        } else {
            depth--;
            search = nextClose + 6;
        }
    }
    return search;
}

char* execute_xpath_like(const char* xml_file_path, const char* id_value) {
    char* content = NULL;
    size_t content_len = 0;
    if (!read_file_to_string(xml_file_path, &content, &content_len)) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    StrBuf res;
    sb_init(&res);
    long pos = 0;
    while (1) {
        long start = find_substr_from(content, pos, "<tag");
        if (start == -1) break;
        long gt = find_char_from(content, start, '>');
        if (gt == -1) break;
        int selfClosing = gt > start && content[gt - 1] == '/';
        long attrsStart = start + 4;
        if (attrsStart > gt) { pos = start + 1; continue; }
        long attrsLen = gt - attrsStart;
        char* attrs = (char*)malloc((size_t)attrsLen + 1);
        if (!attrs) { free(content); sb_free(&res); return NULL; }
        memcpy(attrs, content + attrsStart, (size_t)attrsLen);
        attrs[attrsLen] = '\0';
        char* attrVal = extract_attr_value(attrs, "id");
        if (attrVal && strcmp(attrVal, id_value) == 0) {
            if (!selfClosing) {
                long endIdx = find_matching_end(content, gt + 1);
                if (endIdx != -1) {
                    if (res.len > 0) sb_append_cstr(&res, "\n");
                    sb_append_len(&res, content + start, (size_t)(endIdx - start));
                }
            } else {
                if (res.len > 0) sb_append_cstr(&res, "\n");
                sb_append_len(&res, content + start, (size_t)(gt - start + 1));
            }
        }
        if (attrVal) free(attrVal);
        free(attrs);
        pos = start + 1;
    }
    free(content);
    return res.data; // caller owns
}

int main(int argc, char** argv) {
    if (argc >= 3) {
        const char* idVal = argv[1];
        const char* xmlPath = argv[2];
        char* out = execute_xpath_like(xmlPath, idVal);
        if (out) {
            printf("%s\n", out);
            free(out);
        }
        return 0;
    }
    const char* samplePath = "sample_task97.xml";
    write_sample_file(samplePath);
    const char* tests[5] = {"1", "2", "3", "missing", "nested"};
    for (int i = 0; i < 5; ++i) {
        const char* id = tests[i];
        printf("XPath: /tag[@id='%s']\n", id);
        char* res = execute_xpath_like(samplePath, id);
        if (res) {
            printf("%s\n", res);
            free(res);
        } else {
            printf("\n");
        }
        printf("-----\n");
    }
    return 0;
}