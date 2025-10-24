#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    char **tokens;
    size_t count;
    size_t cap;
} TokenArray;

typedef struct {
    char *ptr;
    size_t len;
    int is_empty;
} LineView;

static void free_token_array(TokenArray *ta) {
    if (!ta) return;
    free(ta->tokens);
    ta->tokens = NULL;
    ta->count = 0;
    ta->cap = 0;
}

static int push_token(TokenArray *ta, char *tok) {
    if (ta->count == ta->cap) {
        size_t ncap = ta->cap == 0 ? 4 : ta->cap * 2;
        char **newptr = (char**)realloc(ta->tokens, ncap * sizeof(char*));
        if (!newptr) return 0;
        ta->tokens = newptr;
        ta->cap = ncap;
    }
    ta->tokens[ta->count++] = tok;
    return 1;
}

static int is_all_space_or_newline(const char *s) {
    if (!s) return 1;
    for (const char *p = s; *p; ++p) {
        if (!(*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t')) return 0;
    }
    return 1;
}

char* transpose(const char* content, int* error) {
    if (error) *error = 1;
    if (content == NULL || error == NULL) return NULL;

    if (is_all_space_or_newline(content)) {
        char *out = (char*)malloc(1);
        if (!out) return NULL;
        out[0] = '\0';
        *error = 0;
        return out;
    }

    size_t len = strlen(content);
    char *buf = (char*)malloc(len + 1);
    if (!buf) return NULL;
    memcpy(buf, content, len + 1);

    // Build line views by splitting on '\n' and removing trailing '\r'
    size_t lines_cap = 8, lines_count = 0;
    LineView *lines = (LineView*)malloc(lines_cap * sizeof(LineView));
    if (!lines) { free(buf); return NULL; }

    char *start = buf;
    for (size_t i = 0; i <= len; ++i) {
        if (buf[i] == '\n' || buf[i] == '\0') {
            buf[i] = '\0';
            char *line_ptr = start;
            size_t L = strlen(line_ptr);
            if (L > 0 && line_ptr[L - 1] == '\r') {
                line_ptr[L - 1] = '\0';
                L -= 1;
            }
            if (lines_count == lines_cap) {
                size_t ncap = lines_cap * 2;
                LineView *tmp = (LineView*)realloc(lines, ncap * sizeof(LineView));
                if (!tmp) { free(lines); free(buf); return NULL; }
                lines = tmp;
                lines_cap = ncap;
            }
            lines[lines_count].ptr = line_ptr;
            lines[lines_count].len = L;
            lines[lines_count].is_empty = (L == 0);
            lines_count++;
            start = (i < len) ? (&buf[i + 1]) : (&buf[i]);
        }
    }

    // Determine first/last non-empty
    ssize_t firstNonEmpty = -1, lastNonEmpty = -1;
    for (size_t i = 0; i < lines_count; ++i) {
        if (!lines[i].is_empty) { firstNonEmpty = (ssize_t)i; break; }
    }
    for (ssize_t i = (ssize_t)lines_count - 1; i >= 0; --i) {
        if (!lines[i].is_empty) { lastNonEmpty = i; break; }
    }
    if (firstNonEmpty == -1) {
        char *out = (char*)malloc(1);
        if (!out) { free(lines); free(buf); return NULL; }
        out[0] = '\0';
        *error = 0;
        free(lines);
        free(buf);
        return out;
    }

    for (ssize_t i = firstNonEmpty; i <= lastNonEmpty; ++i) {
        if (lines[i].is_empty) {
            free(lines); free(buf);
            return NULL; // empty line within data block -> error
        }
    }

    // Tokenize and validate
    size_t rows_cap = 8, rows_count = 0;
    TokenArray *rows = (TokenArray*)calloc(rows_cap, sizeof(TokenArray));
    if (!rows) { free(lines); free(buf); return NULL; }

    size_t expected_cols = (size_t)(-1);
    size_t sumTokenLen = 0;
    int invalid = 0;

    for (ssize_t idx = firstNonEmpty; idx <= lastNonEmpty && !invalid; ++idx) {
        char *ln = lines[idx].ptr;
        size_t L = lines[idx].len;

        if (L > 0) {
            if (ln[0] == ' ' || ln[L - 1] == ' ') { invalid = 1; break; }
            for (size_t k = 1; k < L; ++k) {
                if (ln[k] == ' ' && ln[k - 1] == ' ') { invalid = 1; break; }
            }
            if (invalid) break;
        }

        TokenArray ta = {0};
        size_t token_count = 0;
        if (L > 0) {
            char *p = ln;
            char *tok_start = p;
            for (size_t k = 0; k <= L; ++k) {
                if (ln[k] == ' ' || ln[k] == '\0') {
                    char saved = ln[k];
                    ln[k] = '\0';
                    if (!push_token(&ta, tok_start)) { invalid = 1; break; }
                    sumTokenLen += strlen(tok_start);
                    token_count++;
                    if (saved == '\0') break;
                    ln[k] = saved; // restore temporarily to continue scanning
                    tok_start = &ln[k + 1];
                }
            }
        }
        if (invalid) { free_token_array(&ta); break; }

        if (rows_count == rows_cap) {
            size_t ncap = rows_cap * 2;
            TokenArray *tmp = (TokenArray*)realloc(rows, ncap * sizeof(TokenArray));
            if (!tmp) { free_token_array(&ta); invalid = 1; break; }
            // Initialize new slots
            for (size_t z = rows_cap; z < ncap; ++z) {
                tmp[z].tokens = NULL; tmp[z].count = 0; tmp[z].cap = 0;
            }
            rows = tmp;
            rows_cap = ncap;
        }
        rows[rows_count++] = ta;

        if (expected_cols == (size_t)(-1)) expected_cols = token_count;
        else if (expected_cols != token_count) { invalid = 1; break; }
    }

    if (invalid) {
        for (size_t i = 0; i < rows_count; ++i) free_token_array(&rows[i]);
        free(rows);
        free(lines);
        free(buf);
        return NULL;
    }

    if (rows_count == 0 || expected_cols == 0) {
        char *out = (char*)malloc(1);
        if (!out) { 
            for (size_t i = 0; i < rows_count; ++i) free_token_array(&rows[i]);
            free(rows); free(lines); free(buf); 
            return NULL; 
        }
        out[0] = '\0';
        *error = 0;
        for (size_t i = 0; i < rows_count; ++i) free_token_array(&rows[i]);
        free(rows);
        free(lines);
        free(buf);
        return out;
    }

    size_t r = rows_count;
    size_t c = expected_cols;
    size_t spaces = c * (r > 0 ? (r - 1) : 0);
    size_t newlines = (c > 0 ? (c - 1) : 0);
    size_t totalLen = sumTokenLen + spaces + newlines;

    char *out = (char*)malloc(totalLen + 1);
    if (!out) {
        for (size_t i = 0; i < rows_count; ++i) free_token_array(&rows[i]);
        free(rows);
        free(lines);
        free(buf);
        return NULL;
    }

    char *pout = out;
    for (size_t col = 0; col < c; ++col) {
        for (size_t row = 0; row < r; ++row) {
            char *tok = rows[row].tokens[col];
            size_t tl = strlen(tok);
            memcpy(pout, tok, tl);
            pout += tl;
            if (row + 1 < r) { *pout = ' '; ++pout; }
        }
        if (col + 1 < c) { *pout = '\n'; ++pout; }
    }
    *pout = '\0';

    for (size_t i = 0; i < rows_count; ++i) free_token_array(&rows[i]);
    free(rows);
    free(lines);
    free(buf);
    *error = 0;
    return out;
}

static void run_test(const char* label, const char* input) {
    int err = 0;
    char *res = transpose(input, &err);
    printf("%s:\n", label);
    if (err || res == NULL) {
        printf("ERROR\n");
    } else {
        printf("%s\n", res);
    }
    printf("----\n");
    free(res);
}

int main(void) {
    run_test("Test 1", "name age\nalice 21\nryan 30");
    run_test("Test 2", "a b c");
    run_test("Test 3", "a\nb\nc");
    run_test("Test 4", "a b\nc d\n");
    run_test("Test 5 (invalid)", "a b\nc");
    return 0;
}