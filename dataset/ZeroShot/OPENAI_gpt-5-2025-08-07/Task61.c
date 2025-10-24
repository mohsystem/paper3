#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static int starts_with_at(const char* s, size_t idx, const char* pat) {
    size_t sn = strlen(s);
    size_t pn = strlen(pat);
    if (idx + pn > sn) return 0;
    return strncmp(s + idx, pat, pn) == 0;
}

static size_t find_substr(const char* s, size_t start, const char* pat) {
    const char* p = strstr(s + start, pat);
    if (!p) return (size_t)(-1);
    return (size_t)(p - s);
}

static char* str_dup_range(const char* s, size_t start, size_t end) {
    if (end <= start) {
        char* out = (char*)malloc(1);
        if (out) out[0] = '\0';
        return out;
    }
    size_t len = end - start;
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, s + start, len);
    out[len] = '\0';
    return out;
}

char* getRootElementName(const char* xml) {
    if (xml == NULL) {
        char* out = (char*)malloc(1);
        if (out) out[0] = '\0';
        return out;
    }
    size_t n = strlen(xml);
    size_t i = 0;

    // Skip UTF-8 BOM if present
    if (n >= 3 && (unsigned char)xml[0] == 0xEF && (unsigned char)xml[1] == 0xBB && (unsigned char)xml[2] == 0xBF) {
        i = 3;
    }

    // Also skip literal U+FEFF if somehow present as a single char (environment dependent)
    if (i < n && (unsigned char)xml[i] == 0xFE) {
        // best-effort; not reliable for all locales
        i++;
    }

    while (i < n) {
        // Skip whitespace
        while (i < n && (xml[i] == ' ' || xml[i] == '\t' || xml[i] == '\r' || xml[i] == '\n')) i++;
        if (i >= n) break;

        // Processing Instruction
        if (starts_with_at(xml, i, "<?")) {
            size_t end = find_substr(xml, i + 2, "?>");
            if (end == (size_t)(-1)) {
                char* out = (char*)malloc(1);
                if (out) out[0] = '\0';
                return out;
            }
            i = end + 2;
            continue;
        }

        // Comment
        if (starts_with_at(xml, i, "<!--")) {
            size_t end = find_substr(xml, i + 4, "-->");
            if (end == (size_t)(-1)) {
                char* out = (char*)malloc(1);
                if (out) out[0] = '\0';
                return out;
            }
            i = end + 3;
            continue;
        }

        // CDATA
        if (starts_with_at(xml, i, "<![CDATA[")) {
            size_t end = find_substr(xml, i + 9, "]]>");
            if (end == (size_t)(-1)) {
                char* out = (char*)malloc(1);
                if (out) out[0] = '\0';
                return out;
            }
            i = end + 3;
            continue;
        }

        // Declaration (DOCTYPE or others)
        if (starts_with_at(xml, i, "<!")) {
            i += 2; // after "<!"
            int depth = 0;
            int in_quote = 0;
            char quote_ch = 0;
            while (i < n) {
                char ch = xml[i];
                if (in_quote) {
                    if (ch == quote_ch) in_quote = 0;
                } else {
                    if (ch == '"' || ch == '\'') {
                        in_quote = 1; quote_ch = ch;
                    } else if (ch == '[') {
                        depth++;
                    } else if (ch == ']') {
                        if (depth > 0) depth--;
                    } else if (ch == '>' && depth == 0) {
                        i++;
                        break;
                    }
                }
                i++;
            }
            continue;
        }

        // Start tag
        if (xml[i] == '<') {
            if (i + 1 < n && xml[i + 1] == '/') {
                char* out = (char*)malloc(1);
                if (out) out[0] = '\0';
                return out;
            }
            size_t j = i + 1;
            while (j < n) {
                char ch = xml[j];
                if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n' || ch == '/' || ch == '>') break;
                j++;
            }
            if (j == i + 1) {
                char* out = (char*)malloc(1);
                if (out) out[0] = '\0';
                return out;
            }
            return str_dup_range(xml, i + 1, j);
        }

        i++;
    }

    char* out = (char*)malloc(1);
    if (out) out[0] = '\0';
    return out;
}

int main(void) {
    const char* tests[5] = {
        "<root></root>",
        "<?xml version=\"1.0\"?><root attr='1'><child/></root>",
        "<!-- comment --><!DOCTYPE note [<!ELEMENT note ANY>]><note><to>T</to></note>",
        "<x:Root xmlns:x='urn:x'><x:child/></x:Root>",
        "\xEF\xBB\xBF  <?pi test?><root/>"
    };
    for (int k = 0; k < 5; ++k) {
        char* name = getRootElementName(tests[k]);
        if (name) {
            printf("%s\n", name);
            free(name);
        } else {
            printf("\n");
        }
    }
    return 0;
}