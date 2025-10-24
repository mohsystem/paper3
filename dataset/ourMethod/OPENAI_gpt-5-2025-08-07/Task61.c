#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    int ok;
    char* root;       // must be freed by caller
    char* structure;  // newline-separated lines; must be freed by caller
    char* error;      // must be freed by caller
} ParseResultC;

static int is_name_start(char c) {
    return (c == ':' || c == '_' || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'));
}
static int is_name_char(char c) {
    return is_name_start(c) || c == '-' || c == '.' || (c >= '0' && c <= '9');
}

static int starts_with(const char* s, size_t i, size_t n, const char* pfx) {
    size_t m = strlen(pfx);
    if (i + m > n) return 0;
    return strncmp(s + i, pfx, m) == 0;
}

static void append_str(char** buf, size_t* len, size_t* cap, const char* s) {
    if (!s) return;
    size_t add = strlen(s);
    if (*cap - *len < add + 1) {
        size_t newcap = (*cap == 0) ? 256 : *cap;
        while (newcap - *len < add + 1) newcap *= 2;
        char* nb = (char*)realloc(*buf, newcap);
        if (!nb) return;
        *buf = nb;
        *cap = newcap;
    }
    memcpy(*buf + *len, s, add);
    *len += add;
    (*buf)[*len] = '\0';
}

static void append_indent_and_name(char** buf, size_t* len, size_t* cap, size_t depth, const char* name) {
    size_t spaces = depth * 2;
    if (*cap - *len < spaces + strlen(name) + 2) {
        size_t newcap = (*cap == 0) ? 256 : *cap;
        while (newcap - *len < spaces + strlen(name) + 2) newcap *= 2;
        char* nb = (char*)realloc(*buf, newcap);
        if (!nb) return;
        *buf = nb;
        *cap = newcap;
    }
    for (size_t i = 0; i < spaces; ++i) (*buf)[(*len)++] = ' ';
    memcpy(*buf + *len, name, strlen(name));
    *len += strlen(name);
    (*buf)[(*len)++] = '\n';
    (*buf)[*len] = '\0';
}

static char* substr_dup(const char* s, size_t start, size_t end) {
    if (end < start) return NULL;
    size_t len = end - start;
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, s + start, len);
    out[len] = '\0';
    return out;
}

static void free_parse_result(ParseResultC* r) {
    if (!r) return;
    if (r->root) free(r->root);
    if (r->structure) free(r->structure);
    if (r->error) free(r->error);
    r->root = NULL; r->structure = NULL; r->error = NULL;
}

static ParseResultC parse_xml_internal(const char* xml) {
    ParseResultC res;
    res.ok = 0;
    res.root = NULL;
    res.structure = NULL;
    res.error = NULL;

    if (!xml) {
        res.error = strdup("Null input.");
        return res;
    }
    size_t n = strlen(xml);
    const size_t MAX_LEN = 5 * 1024 * 1024;
    if (n == 0 || n > MAX_LEN) {
        res.error = strdup("Invalid input size.");
        return res;
    }

    size_t i = 0;

    // Skip UTF-8 BOM
    if (n >= 3 && (unsigned char)xml[0] == 0xEF && (unsigned char)xml[1] == 0xBB && (unsigned char)xml[2] == 0xBF) {
        i = 3;
    }

    #define SKIP_WS() while (i < n && (xml[i] == ' ' || xml[i] == '\t' || xml[i] == '\r' || xml[i] == '\n')) { i++; }

    char** stack = NULL;
    size_t stack_size = 0, stack_cap = 0;
    const size_t MAX_DEPTH = 4096;

    char* struct_buf = NULL;
    size_t struct_len = 0, struct_cap = 0;

    char* root = NULL;

    // Helpers to manage stack safely
    #define STACK_PUSH(nameStr) do { \
        if (stack_size + 1 > MAX_DEPTH) { res.error = strdup("Exceeded maximum XML depth."); goto cleanup; } \
        if (stack_size == stack_cap) { \
            size_t newcap = (stack_cap == 0) ? 16 : stack_cap * 2; \
            char** ns = (char**)realloc(stack, newcap * sizeof(char*)); \
            if (!ns) { res.error = strdup("Out of memory."); goto cleanup; } \
            stack = ns; stack_cap = newcap; \
        } \
        stack[stack_size++] = nameStr; \
    } while(0)

    #define STACK_POP() do { \
        if (stack_size > 0) { \
            free(stack[stack_size - 1]); \
            stack_size--; \
        } \
    } while(0)

    while (i < n) {
        // Find next '<'
        const char* p = memchr(xml + i, '<', n - i);
        if (!p) break;
        i = (size_t)(p - xml);

        if (starts_with(xml, i, n, "<!--")) {
            i += 4;
            while (i + 2 < n) {
                if (xml[i] == '-' && xml[i + 1] == '-' && xml[i + 2] == '>') { i += 3; break; }
                i++;
            }
            if (i > n - 1) { res.error = strdup("Unterminated comment."); goto cleanup; }
            continue;
        }
        if (starts_with(xml, i, n, "<?")) {
            i += 2;
            while (i + 1 < n) {
                if (xml[i] == '?' && xml[i + 1] == '>') { i += 2; break; }
                i++;
            }
            if (i > n - 1) { res.error = strdup("Unterminated processing instruction."); goto cleanup; }
            continue;
        }
        if (starts_with(xml, i, n, "<![CDATA[")) {
            i += 9;
            while (i + 2 < n) {
                if (xml[i] == ']' && xml[i + 1] == ']' && xml[i + 2] == '>') { i += 3; break; }
                i++;
            }
            if (i > n - 1) { res.error = strdup("Unterminated CDATA section."); goto cleanup; }
            continue;
        }
        if (starts_with(xml, i, n, "<!DOCTYPE")) {
            i += 9;
            int bracketDepth = 0;
            int inQuotes = 0; char quoteChar = 0;
            while (i < n) {
                char c = xml[i];
                if (inQuotes) {
                    if (c == quoteChar) inQuotes = 0;
                    i++;
                    continue;
                }
                if (c == '"' || c == '\'') { inQuotes = 1; quoteChar = c; i++; continue; }
                if (c == '[') { bracketDepth++; i++; continue; }
                if (c == ']') { if (bracketDepth > 0) bracketDepth--; i++; continue; }
                if (c == '>' && bracketDepth == 0) { i++; break; }
                i++;
            }
            if (i > n) { res.error = strdup("Unterminated DOCTYPE."); goto cleanup; }
            continue;
        }

        if (i + 1 < n && xml[i + 1] == '/') {
            // closing tag
            i += 2;
            SKIP_WS();
            size_t start = i;
            if (i < n && is_name_start(xml[i])) {
                i++;
                while (i < n && is_name_char(xml[i])) i++;
            } else { res.error = strdup("Invalid closing tag name."); goto cleanup; }
            size_t end = i;
            SKIP_WS();
            if (i >= n || xml[i] != '>') { res.error = strdup("Malformed closing tag."); goto cleanup; }
            i++;
            char* cname = substr_dup(xml, start, end);
            if (!cname) { res.error = strdup("Out of memory."); goto cleanup; }

            if (stack_size == 0 || strcmp(stack[stack_size - 1], cname) != 0) {
                free(cname);
                res.error = strdup("Mismatched closing tag.");
                goto cleanup;
            }
            free(cname);
            STACK_POP();
            continue;
        }

        if (i + 1 >= n || xml[i + 1] == '>' || xml[i + 1] == '/') {
            res.error = strdup("Malformed tag start.");
            goto cleanup;
        }

        // start or empty-element tag
        i++; // consume '<'
        SKIP_WS();
        size_t sName = i;
        if (i < n && is_name_start(xml[i])) {
            i++;
            while (i < n && is_name_char(xml[i])) i++;
        } else { res.error = strdup("Invalid start tag name."); goto cleanup; }
        size_t eName = i;

        char* name = substr_dup(xml, sName, eName);
        if (!name) { res.error = strdup("Out of memory."); goto cleanup; }

        if (!root && stack_size == 0) {
            root = strdup(name);
            if (!root) { free(name); res.error = strdup("Out of memory."); goto cleanup; }
        }

        // parse until end of tag, respecting quotes
        int inQuotes = 0; char quoteChar = 0; int selfClose = 0;
        while (i < n) {
            char c = xml[i];
            if (inQuotes) {
                if (c == quoteChar) inQuotes = 0;
                i++;
                continue;
            }
            if (c == '"' || c == '\'') {
                inQuotes = 1; quoteChar = c; i++; continue;
            }
            if (c == '/' && (i + 1 < n) && xml[i + 1] == '>') {
                selfClose = 1; i += 2; break;
            }
            if (c == '>') { i++; break; }
            i++;
        }
        if (i > n) { free(name); res.error = strdup("Unterminated start tag."); goto cleanup; }

        // record structure line
        append_indent_and_name(&struct_buf, &struct_len, &struct_cap, stack_size, name);

        if (!selfClose) {
            STACK_PUSH(name); // name ownership transferred to stack
        } else {
            free(name);
        }
    }

    if (stack_size != 0) {
        res.error = strdup("Unclosed tags remain.");
        goto cleanup;
    }
    if (!root) {
        res.error = strdup("No root element found.");
        goto cleanup;
    }

    res.ok = 1;
    res.root = root;
    res.structure = struct_buf;
    root = NULL; struct_buf = NULL;

cleanup:
    // Free remaining stack
    while (stack_size > 0) {
        free(stack[stack_size - 1]);
        stack_size--;
    }
    free(stack);
    if (!res.ok) {
        if (root) free(root);
        if (struct_buf) free(struct_buf);
    }
    return res;
}

ParseResultC parse_xml(const char* xml) {
    return parse_xml_internal(xml);
}

// Returns newly allocated string with the root element name, or NULL on error.
char* find_root_element(const char* xml) {
    ParseResultC r = parse_xml_internal(xml);
    char* out = NULL;
    if (r.ok && r.root) {
        out = strdup(r.root);
    }
    free_parse_result(&r);
    return out;
}

// Returns a newly allocated string containing the structure lines, or NULL on error.
char* extract_structure(const char* xml) {
    ParseResultC r = parse_xml_internal(xml);
    char* out = NULL;
    if (r.ok && r.structure) {
        out = strdup(r.structure);
    }
    free_parse_result(&r);
    return out;
}

int main(void) {
    const char* tests[5] = {
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?><root><child/><child2><x/></child2></root>",
        "  <!-- leading comment -->\n<a id=\"1\"><b></b><c/></a>",
        "<!DOCTYPE note [ <!ELEMENT note (to,from)> ]><note><![CDATA[<fake>]]><to>T</to><from>F</from></note>",
        "<?xml version='1.0'?><?pi test?><x y='1'><y1><z/></y1></x>",
        "<r><a></r>"
    };

    for (int i = 0; i < 5; ++i) {
        printf("Test %d:\n", i + 1);
        ParseResultC r = parse_xml(tests[i]);
        if (r.ok) {
            printf("Root: %s\n", r.root ? r.root : "(none)");
            printf("Structure:\n%s", r.structure ? r.structure : "");
        } else {
            printf("Error: %s\n", r.error ? r.error : "Unknown error");
        }
        printf("----\n");
        free_parse_result(&r);
    }

    // Demonstrate direct helper usage
    char* root = find_root_element(tests[0]);
    char* struct_lines = extract_structure(tests[0]);
    if (root) {
        printf("Direct root: %s\n", root);
        free(root);
    }
    if (struct_lines) {
        printf("Direct structure:\n%s", struct_lines);
        free(struct_lines);
    }

    return 0;
}