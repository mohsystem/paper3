#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    char **fields;
    int nfields;
    int cap;
} CSVRow;

typedef struct {
    CSVRow *rows;
    int nrows;
    int cap;
} CSV;

typedef struct {
    char *data;
    size_t len;
    size_t cap;
} StrBuf;

static void sb_init(StrBuf *sb) {
    sb->data = NULL;
    sb->len = 0;
    sb->cap = 0;
}
static void sb_reserve(StrBuf *sb, size_t need) {
    if (sb->cap >= need) return;
    size_t newCap = sb->cap ? sb->cap : 16;
    while (newCap < need) newCap *= 2;
    char *nd = (char*)realloc(sb->data, newCap);
    if (!nd) exit(1);
    sb->data = nd;
    sb->cap = newCap;
}
static void sb_append_char(StrBuf *sb, char c) {
    sb_reserve(sb, sb->len + 2);
    sb->data[sb->len++] = c;
    sb->data[sb->len] = '\0';
}
static void sb_append_str(StrBuf *sb, const char *s) {
    size_t L = strlen(s);
    sb_reserve(sb, sb->len + L + 1);
    memcpy(sb->data + sb->len, s, L);
    sb->len += L;
    sb->data[sb->len] = '\0';
}
static char* sb_detach(StrBuf *sb) {
    if (!sb->data) {
        char *z = (char*)malloc(1);
        if (!z) exit(1);
        z[0] = '\0';
        return z;
    }
    char *ret = (char*)realloc(sb->data, sb->len + 1);
    if (!ret) exit(1);
    ret[sb->len] = '\0';
    sb->data = NULL;
    sb->len = 0;
    sb->cap = 0;
    return ret;
}
static void sb_clear(StrBuf *sb) {
    sb->len = 0;
    if (sb->data) sb->data[0] = '\0';
}
static void sb_free(StrBuf *sb) {
    free(sb->data);
    sb->data = NULL;
    sb->len = sb->cap = 0;
}

static void row_init(CSVRow *row) {
    row->fields = NULL;
    row->nfields = 0;
    row->cap = 0;
}
static void row_push_field(CSVRow *row, char *field) {
    if (row->nfields == row->cap) {
        int newCap = row->cap ? row->cap * 2 : 4;
        char **nd = (char**)realloc(row->fields, newCap * sizeof(char*));
        if (!nd) exit(1);
        row->fields = nd;
        row->cap = newCap;
    }
    row->fields[row->nfields++] = field;
}
static void row_free(CSVRow *row) {
    for (int i = 0; i < row->nfields; i++) free(row->fields[i]);
    free(row->fields);
    row->fields = NULL;
    row->nfields = row->cap = 0;
}

static void csv_init(CSV *csv) {
    csv->rows = NULL;
    csv->nrows = 0;
    csv->cap = 0;
}
static void csv_push_row(CSV *csv, CSVRow *row) {
    if (csv->nrows == csv->cap) {
        int newCap = csv->cap ? csv->cap * 2 : 4;
        CSVRow *nd = (CSVRow*)realloc(csv->rows, newCap * sizeof(CSVRow));
        if (!nd) exit(1);
        csv->rows = nd;
        csv->cap = newCap;
    }
    csv->rows[csv->nrows++] = *row; // move
    row_init(row);
}
static void csv_free(CSV *csv) {
    for (int i = 0; i < csv->nrows; i++) row_free(&csv->rows[i]);
    free(csv->rows);
    csv->rows = NULL;
    csv->nrows = csv->cap = 0;
}

static CSV parse_csv(const char *input) {
    CSV csv; csv_init(&csv);
    CSVRow row; row_init(&row);
    StrBuf field; sb_init(&field);

    bool in_quotes = false;
    bool anyChar = false;
    bool endedOnNewline = false;

    size_t i = 0;
    size_t n = input ? strlen(input) : 0;
    while (i < n) {
        char c = input[i];
        anyChar = true;
        if (in_quotes) {
            if (c == '"') {
                if (i + 1 < n && input[i + 1] == '"') {
                    sb_append_char(&field, '"');
                    i += 2;
                } else {
                    in_quotes = false;
                    i++;
                }
            } else {
                sb_append_char(&field, c);
                i++;
            }
            endedOnNewline = false;
        } else {
            if (c == '"') {
                in_quotes = true;
                i++;
                endedOnNewline = false;
            } else if (c == ',') {
                char *f = sb_detach(&field);
                row_push_field(&row, f);
                sb_clear(&field);
                i++;
                endedOnNewline = false;
            } else if (c == '\r' || c == '\n') {
                char *f = sb_detach(&field);
                row_push_field(&row, f);
                sb_clear(&field);
                csv_push_row(&csv, &row);
                if (c == '\r' && i + 1 < n && input[i + 1] == '\n') i += 2;
                else i++;
                endedOnNewline = true;
            } else {
                sb_append_char(&field, c);
                i++;
                endedOnNewline = false;
            }
        }
    }
    if (in_quotes || field.len > 0 || row.nfields > 0 || (anyChar && !endedOnNewline)) {
        char *f = sb_detach(&field);
        row_push_field(&row, f);
        csv_push_row(&csv, &row);
    }
    sb_free(&field);
    row_free(&row); // in case moved or empty
    return csv;
}

static char* to_csv(const CSV *csv) {
    StrBuf out; sb_init(&out);
    for (int r = 0; r < csv->nrows; r++) {
        const CSVRow *row = &csv->rows[r];
        for (int c = 0; c < row->nfields; c++) {
            const char *f = row->fields[c];
            bool needQuotes = false;
            for (const char *p = f; *p; ++p) {
                if (*p == '"' || *p == ',' || *p == '\n' || *p == '\r') { needQuotes = true; break; }
            }
            if (needQuotes) {
                sb_append_char(&out, '"');
                for (const char *p = f; *p; ++p) {
                    if (*p == '"') sb_append_str(&out, "\"\"");
                    else sb_append_char(&out, *p);
                }
                sb_append_char(&out, '"');
            } else {
                sb_append_str(&out, f);
            }
            if (c + 1 < row->nfields) sb_append_char(&out, ',');
        }
        if (r + 1 < csv->nrows) sb_append_char(&out, '\n');
    }
    return sb_detach(&out);
}

static void print_rows(const char *title, const CSV *csv) {
    printf("%s\n", title);
    for (int i = 0; i < csv->nrows; i++) {
        printf("Row %d [", i);
        for (int j = 0; j < csv->rows[i].nfields; j++) {
            if (j) printf(" | ");
            printf("%s", csv->rows[i].fields[j]);
        }
        printf("]\n");
    }
}

int main(void) {
    const char *tests[5] = {
        "a,b,c\n1,2,3",
        "name,quote\nJohn,\"Hello, world\"",
        "id,notes\n1,\"Line1\nLine2\"\n2,\"A\"",
        "text\n\"He said \"\"Hi\"\"\"",
        "a,b,c\r\n1,,3\r\n,2,\r\n"
    };
    for (int t = 0; t < 5; t++) {
        printf("==== Test %d ====\n", t + 1);
        CSV parsed = parse_csv(tests[t]);
        print_rows("Parsed:", &parsed);
        char *serialized = to_csv(&parsed);
        printf("Serialized:\n%s\n\n", serialized);
        free(serialized);
        csv_free(&parsed);
    }
    return 0;
}