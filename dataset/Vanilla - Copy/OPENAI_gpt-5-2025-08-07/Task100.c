#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

typedef struct {
    char* data;
    size_t size;
    size_t cap;
} StrBuf;

static void sb_init(StrBuf* sb) {
    sb->cap = 256;
    sb->size = 0;
    sb->data = (char*)malloc(sb->cap);
    if (sb->data) sb->data[0] = '\0';
}

static void sb_free(StrBuf* sb) {
    free(sb->data);
    sb->data = NULL;
    sb->size = sb->cap = 0;
}

static int sb_ensure(StrBuf* sb, size_t add) {
    if (sb->size + add + 1 <= sb->cap) return 1;
    size_t ncap = sb->cap * 2;
    while (sb->size + add + 1 > ncap) ncap *= 2;
    char* nd = (char*)realloc(sb->data, ncap);
    if (!nd) return 0;
    sb->data = nd;
    sb->cap = ncap;
    return 1;
}

static int sb_append_n(StrBuf* sb, const char* s, size_t n) {
    if (!sb_ensure(sb, n)) return 0;
    memcpy(sb->data + sb->size, s, n);
    sb->size += n;
    sb->data[sb->size] = '\0';
    return 1;
}

static int sb_append_char(StrBuf* sb, char c) {
    if (!sb_ensure(sb, 1)) return 0;
    sb->data[sb->size++] = c;
    sb->data[sb->size] = '\0';
    return 1;
}

static char* read_file_all(const char* filename, size_t* out_len) {
    FILE* f = fopen(filename, "rb");
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    long sz = ftell(f);
    if (sz < 0) { fclose(f); return NULL; }
    rewind(f);
    char* buf = (char*)malloc((size_t)sz + 1);
    if (!buf) { fclose(f); return NULL; }
    size_t rd = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    buf[rd] = '\0';
    if (out_len) *out_len = rd;
    return buf;
}

char* search_in_file(const char* pattern, const char* filename) {
    size_t text_len = 0;
    char* text = read_file_all(filename, &text_len);
    if (!text) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }

    regex_t re;
    int rc = regcomp(&re, pattern, REG_EXTENDED);
    if (rc != 0) {
        free(text);
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }

    StrBuf sb; sb_init(&sb);
    size_t offset = 0;
    while (offset <= text_len) {
        regmatch_t pmatch[1];
        int e = regexec(&re, text + offset, 1, pmatch, 0);
        if (e != 0) break;
        if (pmatch[0].rm_so == -1 || pmatch[0].rm_eo == -1) break;

        size_t start = (size_t)pmatch[0].rm_so;
        size_t end = (size_t)pmatch[0].rm_eo;
        size_t mlen = end - start;

        if (mlen > 0) {
            sb_append_n(&sb, text + offset + start, mlen);
            sb_append_char(&sb, '\n');
        }

        if (end == 0) {
            offset += 1;
        } else {
            offset += end;
        }
    }

    regfree(&re);
    free(text);

    if (sb.size == 0) {
        sb_append_n(&sb, "", 0);
    }

    return sb.data; // caller must free
}

int main(int argc, char** argv) {
    if (argc >= 3) {
        char* res = search_in_file(argv[1], argv[2]);
        int count = 0;
        for (char* p = res; *p; ++p) if (*p == '\n') count++;
        printf("Matches: %d\n", count);
        if (res && *res) printf("%s", res);
        free(res);
        return 0;
    }

    const char* filename = "test_input.txt";
    const char* content =
        "foo bar baz 123 foo42\n"
        "New line with Foo and BAR.\n"
        "email: test@example.com; ip: 192.168.0.1; dates: 2025-10-04;\n"
        "ababa bab abab ababab\n";

    FILE* wf = fopen(filename, "wb");
    if (wf) {
        fwrite(content, 1, strlen(content), wf);
        fclose(wf);
    }

    const char* patterns[5] = {
        "foo",
        "[0-9]+",
        "(ab)+",
        "[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}",
        "192\\.168\\.[0-9]+\\.[0-9]+"
    };

    for (int i = 0; i < 5; ++i) {
        char* res = search_in_file(patterns[i], filename);
        int count = 0;
        for (char* p = res; *p; ++p) if (*p == '\n') count++;
        printf("Test %d Pattern: %s\n", i + 1, patterns[i]);
        printf("Matches: %d\n", count);
        if (res && *res) printf("%s", res);
        printf("----\n");
        free(res);
    }

    return 0;
}