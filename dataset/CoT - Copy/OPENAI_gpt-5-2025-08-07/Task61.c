#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static int is_name_char(char c) {
    return isalnum((unsigned char)c) || c == '_' || c == '-' || c == ':' || c == '.';
}

static size_t skip_spaces(const char* s, size_t i) {
    while (s[i] == ' ' || s[i] == '\t' || s[i] == '\n' || s[i] == '\r') i++;
    return i;
}

static int starts_with(const char* s, size_t i, const char* p) {
    size_t lp = strlen(p);
    size_t ls = strlen(s);
    if (i + lp > ls) return 0;
    return strncmp(s + i, p, lp) == 0;
}

static long index_of_seq2(const char* s, size_t from, char x, char y) {
    size_t n = strlen(s);
    for (size_t i = from; i + 1 < n; ++i) {
        if (s[i] == x && s[i + 1] == y) return (long)(i + 1);
    }
    return -1;
}

static long index_of_seq3(const char* s, size_t from, char x, char y, char z) {
    size_t n = strlen(s);
    for (size_t i = from; i + 2 < n; ++i) {
        if (s[i] == x && s[i + 1] == y && s[i + 2] == z) return (long)(i + 2);
    }
    return -1;
}

static long skip_declaration(const char* s, size_t from) {
    size_t n = strlen(s);
    int depth = 0;
    for (size_t i = from; i < n; ++i) {
        char c = s[i];
        if (c == '[') depth++;
        else if (c == ']') { if (depth > 0) depth--; }
        else if (c == '>' && depth == 0) return (long)i;
    }
    return -1;
}

static long skip_to_tag_end(const char* s, size_t from) {
    size_t n = strlen(s);
    char quote = 0;
    for (size_t i = from; i < n; ++i) {
        char c = s[i];
        if (quote) {
            if (c == quote) quote = 0;
        } else {
            if (c == '\'' || c == '"') quote = c;
            else if (c == '>') return (long)i;
        }
    }
    return -1;
}

char* getRootElementName(const char* xml) {
    if (xml == NULL) {
        char* empty = (char*)calloc(1, 1);
        return empty;
    }
    const char* a = xml;
    size_t n = strlen(a);
    size_t i = 0;

    char* root = NULL;

    while (i < n) {
        const char* ltptr = strchr(a + i, '<');
        if (!ltptr) break;
        i = (size_t)(ltptr - a);
        if (i + 1 >= n) break;

        if (starts_with(a, i, "<?")) {
            long j = index_of_seq2(a, i + 2, '?', '>');
            if (j < 0) break;
            i = (size_t)j + 1;
            continue;
        }

        if (starts_with(a, i, "<!--")) {
            long j = index_of_seq3(a, i + 4, '-', '-', '>');
            if (j < 0) break;
            i = (size_t)j + 1;
            continue;
        }

        if (starts_with(a, i, "<![CDATA[")) {
            long j = index_of_seq3(a, i + 9, ']', ']', '>');
            if (j < 0) break;
            i = (size_t)j + 1;
            continue;
        }

        if (starts_with(a, i, "<!")) {
            long j = skip_declaration(a, i + 2);
            if (j < 0) break;
            i = (size_t)j + 1;
            continue;
        }

        if (a[i + 1] == '/') {
            size_t j = i + 2;
            j = skip_spaces(a, j);
            while (j < n && is_name_char(a[j])) j++;
            long end = skip_to_tag_end(a, j);
            if (end < 0) break;
            i = (size_t)end + 1;
            continue;
        }

        size_t j = i + 1;
        j = skip_spaces(a, j);
        size_t start = j;
        while (j < n && is_name_char(a[j])) j++;
        if (j == start) {
            i++;
            continue;
        }
        size_t len = j - start;
        root = (char*)malloc(len + 1);
        if (!root) {
            // allocation failed
            char* empty = (char*)calloc(1, 1);
            return empty;
        }
        memcpy(root, a + start, len);
        root[len] = '\0';
        return root; // first opening tag is the root
    }

    if (!root) {
        char* empty = (char*)calloc(1, 1);
        return empty;
    }
    return root;
}

int main(void) {
    const char* tests[5] = {
        "<root><child/></root>",
        "<?xml version='1.0'?><!-- comment --><a id='1'><b></b><c/></a>",
        "  \n <root>text<inner>more</inner></root> ",
        "<x><y></x>",
        "<!DOCTYPE note SYSTEM 'Note.dtd'><note><![CDATA[<notatag>]]><to>T</to></note>"
    };
    for (int t = 0; t < 5; ++t) {
        char* r = getRootElementName(tests[t]);
        if (r) {
            printf("%s\n", r);
            free(r);
        } else {
            printf("\n");
        }
    }
    return 0;
}