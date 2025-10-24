#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Helper to safely duplicate strings
static char* safe_strdup(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char* dup = (char*)malloc(len + 1);
    if (!dup) return NULL;
    memcpy(dup, s, len + 1);
    return dup;
}

typedef struct {
    char** items;
    size_t count;
} Tokens;

// Tokenize a line by one or more spaces, ignore leading/trailing spaces
static Tokens tokenize_line(const char* line) {
    Tokens t = {NULL, 0};
    if (!line) return t;

    size_t cap = 4;
    t.items = (char**)malloc(cap * sizeof(char*));
    if (!t.items) { t.count = 0; return t; }

    size_t len = strlen(line);
    size_t i = 0;

    // skip leading spaces
    while (i < len && line[i] == ' ') i++;

    while (i < len) {
        // collect token
        size_t start = i;
        while (i < len && line[i] != ' ') i++;
        size_t tok_len = i - start;
        if (tok_len > 0) {
            if (t.count == cap) {
                cap *= 2;
                char** ni = (char**)realloc(t.items, cap * sizeof(char*));
                if (!ni) {
                    // free current tokens on failure
                    for (size_t k = 0; k < t.count; ++k) free(t.items[k]);
                    free(t.items);
                    t.items = NULL; t.count = 0;
                    return t;
                }
                t.items = ni;
            }
            char* tok = (char*)malloc(tok_len + 1);
            if (!tok) {
                for (size_t k = 0; k < t.count; ++k) free(t.items[k]);
                free(t.items);
                t.items = NULL; t.count = 0;
                return t;
            }
            memcpy(tok, line + start, tok_len);
            tok[tok_len] = '\0';
            t.items[t.count++] = tok;
        }
        // skip spaces between tokens
        while (i < len && line[i] == ' ') i++;
    }

    return t;
}

static void free_tokens(Tokens* t) {
    if (!t || !t->items) return;
    for (size_t i = 0; i < t->count; ++i) free(t->items[i]);
    free(t->items);
    t->items = NULL;
    t->count = 0;
}

// Transpose function: accepts array of lines and count, returns newly allocated string result
char* transpose(const char* lines[], size_t n) {
    if (!lines || n == 0) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }

    Tokens* rows = (Tokens*)calloc(n, sizeof(Tokens));
    if (!rows) return NULL;

    size_t cols = (size_t)(-1);
    bool ok = true;

    for (size_t i = 0; i < n; ++i) {
        if (!lines[i]) { ok = false; break; }
        rows[i] = tokenize_line(lines[i]);
        if (!rows[i].items || rows[i].count == 0) { ok = false; break; }
        if (cols == (size_t)(-1)) cols = rows[i].count;
        else if (cols != rows[i].count) { ok = false; break; }
    }

    if (!ok) {
        for (size_t i = 0; i < n; ++i) free_tokens(&rows[i]);
        free(rows);
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }

    // compute total length
    size_t total = 0;
    for (size_t c = 0; c < cols; ++c) {
        for (size_t r = 0; r < n; ++r) {
            total += strlen(rows[r].items[c]);
            if (r + 1 < n) total += 1; // space
        }
        if (c + 1 < cols) total += 1; // newline
    }

    char* out = (char*)malloc(total + 1);
    if (!out) {
        for (size_t i = 0; i < n; ++i) free_tokens(&rows[i]);
        free(rows);
        return NULL;
    }

    // fill output
    size_t pos = 0;
    for (size_t c = 0; c < cols; ++c) {
        for (size_t r = 0; r < n; ++r) {
            const char* tok = rows[r].items[c];
            size_t L = strlen(tok);
            memcpy(out + pos, tok, L);
            pos += L;
            if (r + 1 < n) out[pos++] = ' ';
        }
        if (c + 1 < cols) out[pos++] = '\n';
    }
    out[pos] = '\0';

    for (size_t i = 0; i < n; ++i) free_tokens(&rows[i]);
    free(rows);
    return out;
}

int main(void) {
    // 1) Example from prompt
    const char* tc1[] = {
        "name age",
        "alice 21",
        "ryan 30"
    };
    char* r1 = transpose(tc1, 3);
    printf("TC1:\n%s\n\n", r1 ? r1 : "");
    free(r1);

    // 2) 1x1
    const char* tc2[] = { "a" };
    char* r2 = transpose(tc2, 1);
    printf("TC2:\n%s\n\n", r2 ? r2 : "");
    free(r2);

    // 3) 1x3
    const char* tc3[] = { "x y z" };
    char* r3 = transpose(tc3, 1);
    printf("TC3:\n%s\n\n", r3 ? r3 : "");
    free(r3);

    // 4) 3x1
    const char* tc4[] = { "x", "y", "z" };
    char* r4 = transpose(tc4, 3);
    printf("TC4:\n%s\n\n", r4 ? r4 : "");
    free(r4);

    // 5) 3x3
    const char* tc5[] = { "a b c", "d e f", "g h i" };
    char* r5 = transpose(tc5, 3);
    printf("TC5:\n%s\n\n", r5 ? r5 : "");
    free(r5);

    return 0;
}