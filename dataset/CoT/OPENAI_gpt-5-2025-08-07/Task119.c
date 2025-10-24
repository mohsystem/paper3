/* Step 1-5: Secure CSV parser with robust handling of quotes, commas, and newlines (RFC 4180-like) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* String builder */
typedef struct {
    char* buf;
    size_t len;
    size_t cap;
} StrB;

static void sb_init(StrB* sb) {
    sb->buf = NULL;
    sb->len = 0;
    sb->cap = 0;
}

static void sb_reset(StrB* sb) {
    sb->len = 0;
}

static bool sb_ensure(StrB* sb, size_t need) {
    if (need <= sb->cap) return true;
    size_t newcap = sb->cap ? sb->cap : 16;
    while (newcap < need) {
        if (newcap > (SIZE_MAX / 2)) return false;
        newcap *= 2;
    }
    char* nbuf = (char*)realloc(sb->buf, newcap);
    if (!nbuf) return false;
    sb->buf = nbuf;
    sb->cap = newcap;
    return true;
}

static bool sb_append_char(StrB* sb, char c) {
    if (!sb_ensure(sb, sb->len + 1)) return false;
    sb->buf[sb->len++] = c;
    return true;
}

static char* sb_dup_cstr(const StrB* sb) {
    char* out = (char*)malloc(sb->len + 1);
    if (!out) return NULL;
    if (sb->len > 0) memcpy(out, sb->buf, sb->len);
    out[sb->len] = '\0';
    return out;
}

static void sb_free(StrB* sb) {
    free(sb->buf);
    sb->buf = NULL;
    sb->len = 0;
    sb->cap = 0;
}

/* Row and CSV structures */
typedef struct {
    char** fields;
    size_t nfields;
    size_t cap;
} Row;

typedef struct {
    Row* rows;
    size_t nrows;
    size_t cap;
    bool ok;
} CSV;

static void row_init(Row* row) {
    row->fields = NULL;
    row->nfields = 0;
    row->cap = 0;
}

static void row_free(Row* row) {
    if (row->fields) {
        for (size_t i = 0; i < row->nfields; ++i) {
            free(row->fields[i]);
        }
    }
    free(row->fields);
    row->fields = NULL;
    row->nfields = 0;
    row->cap = 0;
}

static bool row_add_field(Row* row, char* field) {
    if (row->nfields == row->cap) {
        size_t newcap = row->cap ? row->cap * 2 : 4;
        char** nf = (char**)realloc(row->fields, newcap * sizeof(char*));
        if (!nf) return false;
        row->fields = nf;
        row->cap = newcap;
    }
    row->fields[row->nfields++] = field;
    return true;
}

static void csv_init(CSV* csv) {
    csv->rows = NULL;
    csv->nrows = 0;
    csv->cap = 0;
    csv->ok = true;
}

static void csv_free(CSV* csv) {
    if (csv->rows) {
        for (size_t i = 0; i < csv->nrows; ++i) {
            row_free(&csv->rows[i]);
        }
    }
    free(csv->rows);
    csv->rows = NULL;
    csv->nrows = 0;
    csv->cap = 0;
    csv->ok = true;
}

static bool csv_add_row(CSV* csv, Row* row) {
    if (csv->nrows == csv->cap) {
        size_t newcap = csv->cap ? csv->cap * 2 : 4;
        Row* nr = (Row*)realloc(csv->rows, newcap * sizeof(Row));
        if (!nr) return false;
        csv->rows = nr;
        csv->cap = newcap;
    }
    csv->rows[csv->nrows++] = *row; /* transfer ownership */
    row->fields = NULL;
    row->nfields = 0;
    row->cap = 0;
    return true;
}

/* Parser */
CSV parse_csv_c(const char* content, char delimiter) {
    CSV csv; csv_init(&csv);

    if (!content || content[0] == '\0') {
        return csv;
    }

    Row cur; row_init(&cur);
    StrB sb; sb_init(&sb);

    bool in_quotes = false;

    for (size_t i = 0; content[i] != '\0'; ++i) {
        char ch = content[i];
        if (in_quotes) {
            if (ch == '"') {
                if (content[i + 1] == '"') {
                    if (!sb_append_char(&sb, '"')) { csv.ok = false; goto cleanup; }
                    ++i;
                } else {
                    in_quotes = false;
                }
            } else {
                if (!sb_append_char(&sb, ch)) { csv.ok = false; goto cleanup; }
            }
        } else {
            if (ch == '"') {
                in_quotes = true;
            } else if (ch == delimiter) {
                char* field = sb_dup_cstr(&sb);
                if (!field) { csv.ok = false; goto cleanup; }
                if (!row_add_field(&cur, field)) { free(field); csv.ok = false; goto cleanup; }
                sb_reset(&sb);
            } else if (ch == '\n' || ch == '\r') {
                char* field = sb_dup_cstr(&sb);
                if (!field) { csv.ok = false; goto cleanup; }
                if (!row_add_field(&cur, field)) { free(field); csv.ok = false; goto cleanup; }
                sb_reset(&sb);
                if (!csv_add_row(&csv, &cur)) { csv.ok = false; goto cleanup; }
                row_init(&cur);
                if (ch == '\r' && content[i + 1] == '\n') {
                    ++i;
                }
            } else {
                if (!sb_append_char(&sb, ch)) { csv.ok = false; goto cleanup; }
            }
        }
    }

    if (in_quotes || sb.len > 0 || cur.nfields > 0) {
        char* field = sb_dup_cstr(&sb);
        if (!field) { csv.ok = false; goto cleanup; }
        if (!row_add_field(&cur, field)) { free(field); csv.ok = false; goto cleanup; }
        if (!csv_add_row(&csv, &cur)) { csv.ok = false; goto cleanup; }
        row_init(&cur);
    }

cleanup:
    sb_free(&sb);
    row_free(&cur);
    return csv;
}

/* Helper to print CSV (for tests) */
static void csv_print(const char* title, const CSV* csv) {
    printf("=== %s ===\n", title);
    if (!csv->ok) {
        printf("Parse error or memory allocation failure.\n\n");
        return;
    }
    for (size_t i = 0; i < csv->nrows; ++i) {
        printf("Row %zu: ", i);
        for (size_t j = 0; j < csv->rows[i].nfields; ++j) {
            const char* f = csv->rows[i].fields[j];
            /* Replace CR/LF in output for visibility */
            printf("[");
            for (const char* p = f; *p; ++p) {
                if (*p == '\n') printf("\\n");
                else if (*p == '\r') printf("\\r");
                else putchar(*p);
            }
            printf("]");
            if (j + 1 < csv->rows[i].nfields) printf(" | ");
        }
        printf("\n");
    }
    printf("\n");
}

/* Main with 5 test cases */
int main(void) {
    const char* t1 = "a,b,c\n1,2,3";
    const char* t2 = "Name,Note\n\"Smith, John\",\"He said \"\"Hello\"\".\"";
    const char* t3 = "A,B\n\"Line1\nLine2\",X";
    const char* t4 = "col1,col2,col3,col4\n, , ,\n,,";
    const char* t5 = "x,y\r\n1,2\r\n3,\"4\r\n5\",6";

    CSV c1 = parse_csv_c(t1, ','); csv_print("Test 1", &c1); csv_free(&c1);
    CSV c2 = parse_csv_c(t2, ','); csv_print("Test 2", &c2); csv_free(&c2);
    CSV c3 = parse_csv_c(t3, ','); csv_print("Test 3", &c3); csv_free(&c3);
    CSV c4 = parse_csv_c(t4, ','); csv_print("Test 4", &c4); csv_free(&c4);
    CSV c5 = parse_csv_c(t5, ','); csv_print("Test 5", &c5); csv_free(&c5);

    return 0;
}