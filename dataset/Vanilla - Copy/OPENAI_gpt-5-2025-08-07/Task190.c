#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* strndup_local(const char* s, size_t n) {
    char* out = (char*)malloc(n + 1);
    if (!out) return NULL;
    memcpy(out, s, n);
    out[n] = '\0';
    return out;
}

typedef struct {
    char** cells;
    int cols;
    int cap;
} Row;

typedef struct {
    Row* rows;
    int nrows;
    int cap;
} Table;

static void row_init(Row* r) {
    r->cells = NULL;
    r->cols = 0;
    r->cap = 0;
}
static void row_add_cell(Row* r, char* cell) {
    if (r->cols == r->cap) {
        int ncap = r->cap ? r->cap * 2 : 4;
        char** ncells = (char**)realloc(r->cells, ncap * sizeof(char*));
        if (!ncells) exit(1);
        r->cells = ncells;
        r->cap = ncap;
    }
    r->cells[r->cols++] = cell;
}
static void table_init(Table* t) {
    t->rows = NULL;
    t->nrows = 0;
    t->cap = 0;
}
static void table_add_row(Table* t, Row r) {
    if (t->nrows == t->cap) {
        int ncap = t->cap ? t->cap * 2 : 4;
        Row* nrows = (Row*)realloc(t->rows, ncap * sizeof(Row));
        if (!nrows) exit(1);
        t->rows = nrows;
        t->cap = ncap;
    }
    t->rows[t->nrows++] = r;
}
static void table_free(Table* t) {
    for (int i = 0; i < t->nrows; ++i) {
        for (int j = 0; j < t->rows[i].cols; ++j) {
            free(t->rows[i].cells[j]);
        }
        free(t->rows[i].cells);
    }
    free(t->rows);
}

char* transpose(const char* content) {
    if (!content) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    // Normalize: remove '\r'
    size_t len = strlen(content);
    char* clean = (char*)malloc(len + 1);
    if (!clean) exit(1);
    size_t w = 0;
    for (size_t i = 0; i < len; ++i) {
        if (content[i] != '\r') clean[w++] = content[i];
    }
    clean[w] = '\0';

    Table table;
    table_init(&table);

    char* p = clean;
    while (*p) {
        char* line_end = strchr(p, '\n');
        size_t line_len = line_end ? (size_t)(line_end - p) : strlen(p);
        if (line_len > 0) {
            char* line = strndup_local(p, line_len);
            Row row;
            row_init(&row);
            // Split by single space (robust to multiple spaces via strtok)
            char* saveptr = NULL;
            char* tok = strtok_r(line, " ", &saveptr);
            while (tok) {
                if (tok[0] != '\0') {
                    char* cell = strdup(tok);
                    row_add_cell(&row, cell);
                }
                tok = strtok_r(NULL, " ", &saveptr);
            }
            if (row.cols > 0) {
                table_add_row(&table, row);
            } else {
                free(row.cells);
            }
            free(line);
        }
        if (!line_end) break;
        p = line_end + 1;
    }
    free(clean);

    if (table.nrows == 0) {
        char* empty = (char*)malloc(1);
        if (!empty) exit(1);
        empty[0] = '\0';
        return empty;
    }
    int R = table.nrows;
    int C = table.rows[0].cols;

    // Compute total length
    size_t total_tokens_len = 0;
    for (int i = 0; i < R; ++i) {
        for (int j = 0; j < table.rows[i].cols; ++j) {
            total_tokens_len += strlen(table.rows[i].cells[j]);
        }
    }
    size_t spaces = (R > 0 && C > 0) ? (size_t)C * (size_t)(R - 1) : 0;
    size_t newlines = (C > 0) ? (size_t)(C - 1) : 0;
    size_t out_len = total_tokens_len + spaces + newlines;

    char* out = (char*)malloc(out_len + 1);
    if (!out) exit(1);
    size_t idx = 0;
    for (int i = 0; i < C; ++i) {
        for (int j = 0; j < R; ++j) {
            char* cell = table.rows[j].cells[i];
            size_t L = strlen(cell);
            memcpy(out + idx, cell, L);
            idx += L;
            if (j < R - 1) out[idx++] = ' ';
        }
        if (i < C - 1) out[idx++] = '\n';
    }
    out[idx] = '\0';

    table_free(&table);
    return out;
}

int main() {
    const char* tests[5] = {
        "name age\nalice 21\nryan 30",
        "a b c",
        "a\na\na",
        "1 2 3\n4 5 6\n7 8 9",
        "w x y z\n1 2 3 4"
    };
    for (int i = 0; i < 5; ++i) {
        char* res = transpose(tests[i]);
        printf("%s\n", res);
        if (i < 4) printf("-----\n");
        free(res);
    }
    return 0;
}