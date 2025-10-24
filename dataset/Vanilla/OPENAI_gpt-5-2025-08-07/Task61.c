#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static int is_name_char(char c) {
    unsigned char uc = (unsigned char)c;
    return isalnum(uc) || c=='_' || c=='-' || c=='.' || c==':';
}

char* get_root_element(const char* xml) {
    if (!xml) return NULL;
    size_t n = strlen(xml);
    size_t i = 0;
    while (i < n) {
        const char* lt_ptr = strchr(xml + i, '<');
        if (!lt_ptr) return NULL;
        size_t lt = (size_t)(lt_ptr - xml);
        i = lt + 1;
        if (i >= n) return NULL;
        char c = xml[i];
        if (c == '?') {
            const char* end = strstr(xml + i + 1, "?>");
            if (!end) return NULL;
            i = (size_t)((end - xml) + 2);
            continue;
        }
        if (c == '!') {
            // Comment
            if (i + 2 < n && strncmp(xml + i, "--", 2) == 0) {
                const char* end = strstr(xml + i + 2, "-->");
                if (!end) return NULL;
                i = (size_t)((end - xml) + 3);
                continue;
            }
            // CDATA
            if (i + 7 < n && strncmp(xml + i, "[CDATA[", 7) == 0) {
                const char* end = strstr(xml + i + 7, "]]>");
                if (!end) return NULL;
                i = (size_t)((end - xml) + 3);
                continue;
            }
            // DOCTYPE or other declaration
            if (i + 7 <= n && strncmp(xml + i, "DOCTYPE", 7) == 0) {
                size_t j = i + 7;
                int bracketDepth = 0;
                while (j < n) {
                    if (xml[j] == '[') { bracketDepth++; j++; continue; }
                    if (xml[j] == ']') { if (bracketDepth > 0) bracketDepth--; j++; continue; }
                    if (xml[j] == '>' && bracketDepth == 0) { j++; break; }
                    j++;
                }
                i = j;
                continue;
            } else {
                // Generic declaration: skip until '>'
                const char* end = strchr(xml + i + 1, '>');
                if (!end) return NULL;
                i = (size_t)((end - xml) + 1);
                continue;
            }
        }
        if (c == '/') {
            // Closing tag before root; skip to '>'
            const char* end = strchr(xml + i + 1, '>');
            if (!end) return NULL;
            i = (size_t)((end - xml) + 1);
            continue;
        }
        // Start tag for root
        size_t j = i;
        if (j >= n || !is_name_char(xml[j])) return NULL;
        while (j < n && is_name_char(xml[j])) j++;
        size_t len = j - i;
        char* name = (char*)malloc(len + 1);
        if (!name) return NULL;
        memcpy(name, xml + i, len);
        name[len] = '\0';
        return name;
    }
    return NULL;
}

int main(void) {
    const char* tests[5] = {
        "<root></root>",
        "<?xml version=\"1.0\"?><a><b/></a>",
        "  \n<!-- comment -->\n<ns:catalog attr='1'><item/></ns:catalog>",
        "<!DOCTYPE note [<!ELEMENT note (to)>]><note><to>Tove</to></note>",
        "<unclosed"
    };
    for (int i = 0; i < 5; ++i) {
        char* res = get_root_element(tests[i]);
        if (res) {
            printf("Test %d: %s\n", i+1, res);
            free(res);
        } else {
            printf("Test %d: null\n", i+1);
        }
    }
    return 0;
}