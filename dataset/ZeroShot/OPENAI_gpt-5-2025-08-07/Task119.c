#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct {
    char*** rows;       // rows[rowIndex][colIndex] -> char*
    size_t* colCounts;  // number of columns per row
    size_t rowCount;
} CSVData;

static int grow_ptr_array(void*** arr, size_t* cap, size_t needed) {
    if (*cap >= needed) return 1;
    size_t newCap = (*cap == 0) ? 8 : *cap;
    while (newCap < needed) {
        if (newCap > SIZE_MAX / 2) return 0;
        newCap *= 2;
    }
    void** tmp = (void**)realloc(*arr, newCap * sizeof(void*));
    if (!tmp) return 0;
    *arr = tmp;
    *cap = newCap;
    return 1;
}

static int grow_size_array(size_t** arr, size_t* cap, size_t needed) {
    if (*cap >= needed) return 1;
    size_t newCap = (*cap == 0) ? 8 : *cap;
    while (newCap < needed) {
        if (newCap > SIZE_MAX / 2) return 0;
        newCap *= 2;
    }
    size_t* tmp = (size_t*)realloc(*arr, newCap * sizeof(size_t));
    if (!tmp) return 0;
    *arr = tmp;
    *cap = newCap;
    return 1;
}

static int append_char(char** buf, size_t* len, size_t* cap, char ch) {
    if (*len + 1 >= *cap) {
        size_t newCap = (*cap == 0) ? 16 : (*cap * 2);
        if (newCap < *len + 2) newCap = *len + 2;
        char* tmp = (char*)realloc(*buf, newCap);
        if (!tmp) return 0;
        *buf = tmp;
        *cap = newCap;
    }
    (*buf)[(*len)++] = ch;
    return 1;
}

static int finalize_field(char*** row, size_t* colCount, size_t* colCap, char** fbuf, size_t* flen) {
    if (!grow_ptr_array((void***)row, colCap, *colCount + 1)) return 0;
    char* s = (char*)malloc(*flen + 1);
    if (!s) return 0;
    if (*flen) memcpy(s, *fbuf, *flen);
    s[*flen] = '\0';
    (*row)[*colCount] = s;
    (*colCount)++;
    *flen = 0;
    return 1;
}

static void free_csv(CSVData* d) {
    if (!d) return;
    if (d->rows) {
        for (size_t r = 0; r < d->rowCount; ++r) {
            if (d->rows[r]) {
                for (size_t c = 0; c < d->colCounts[r]; ++c) {
                    free(d->rows[r][c]);
                }
                free(d->rows[r]);
            }
        }
        free(d->rows);
    }
    free(d->colCounts);
    d->rows = NULL;
    d->colCounts = NULL;
    d->rowCount = 0;
}

CSVData parse_csv(const char* csv) {
    CSVData out = {0};
    if (csv == NULL || csv[0] == '\0') {
        return out;
    }

    size_t rowsCap = 0;
    size_t colCountsCap = 0;
    char*** rows = NULL;
    size_t* colCounts = NULL;

    char** currentRow = NULL;
    size_t currentRowCount = 0, currentRowCap = 0;

    char* fieldBuf = NULL;
    size_t fLen = 0, fCap = 0;

    int inQuotes = 0;
    int fieldStart = 1;

    size_t i = 0;
    size_t n = strlen(csv);

    while (i < n) {
        char ch = csv[i];
        if (inQuotes) {
            if (ch == '"') {
                if (i + 1 < n && csv[i + 1] == '"') {
                    if (!append_char(&fieldBuf, &fLen, &fCap, '"')) goto fail;
                    i++;
                } else {
                    inQuotes = 0;
                }
            } else {
                if (!append_char(&fieldBuf, &fLen, &fCap, ch)) goto fail;
            }
        } else {
            if (fieldStart && ch == '"') {
                inQuotes = 1;
                fieldStart = 0;
            } else if (ch == ',') {
                if (!finalize_field(&currentRow, &currentRowCount, &currentRowCap, &fieldBuf, &fLen)) goto fail;
                fieldStart = 1;
            } else if (ch == '\n') {
                if (!finalize_field(&currentRow, &currentRowCount, &currentRowCap, &fieldBuf, &fLen)) goto fail;
                if (!grow_ptr_array((void***)&rows, &rowsCap, out.rowCount + 1)) goto fail;
                if (!grow_size_array(&colCounts, &colCountsCap, out.rowCount + 1)) goto fail;
                rows[out.rowCount] = currentRow;
                colCounts[out.rowCount] = currentRowCount;
                out.rowCount++;
                currentRow = NULL;
                currentRowCount = 0;
                currentRowCap = 0;
                fieldStart = 1;
            } else if (ch == '\r') {
                if (!finalize_field(&currentRow, &currentRowCount, &currentRowCap, &fieldBuf, &fLen)) goto fail;
                if (i + 1 < n && csv[i + 1] == '\n') {
                    i++;
                }
                if (!grow_ptr_array((void***)&rows, &rowsCap, out.rowCount + 1)) goto fail;
                if (!grow_size_array(&colCounts, &colCountsCap, out.rowCount + 1)) goto fail;
                rows[out.rowCount] = currentRow;
                colCounts[out.rowCount] = currentRowCount;
                out.rowCount++;
                currentRow = NULL;
                currentRowCount = 0;
                currentRowCap = 0;
                fieldStart = 1;
            } else {
                if (!append_char(&fieldBuf, &fLen, &fCap, ch)) goto fail;
                fieldStart = 0;
            }
        }
        i++;
    }

    if (inQuotes) {
        // Unbalanced quotes: treat as end of field
    }

    if (fLen > 0 || !fieldStart || currentRowCount > 0 || (n > 0 && csv[n - 1] == ',')) {
        if (!finalize_field(&currentRow, &currentRowCount, &currentRowCap, &fieldBuf, &fLen)) goto fail;
    }
    if (currentRowCount > 0) {
        if (!grow_ptr_array((void***)&rows, &rowsCap, out.rowCount + 1)) goto fail;
        if (!grow_size_array(&colCounts, &colCountsCap, out.rowCount + 1)) goto fail;
        rows[out.rowCount] = currentRow;
        colCounts[out.rowCount] = currentRowCount;
        out.rowCount++;
        currentRow = NULL;
        currentRowCount = 0;
        currentRowCap = 0;
    }

    free(fieldBuf);
    out.rows = rows;
    out.colCounts = colCounts;
    return out;

fail:
    free(fieldBuf);
    if (currentRow) {
        for (size_t c = 0; c < currentRowCount; ++c) free(currentRow[c]);
        free(currentRow);
    }
    for (size_t r = 0; r < out.rowCount; ++r) {
        if (rows && rows[r]) {
            for (size_t c = 0; c < colCounts[r]; ++c) {
                free(rows[r][c]);
            }
            free(rows[r]);
        }
    }
    free(rows);
    free(colCounts);
    CSVData empty = {0};
    return empty;
}

static void print_csv(const CSVData* d) {
    if (!d) return;
    printf("Rows=%zu\n", d->rowCount);
    for (size_t r = 0; r < d->rowCount; ++r) {
        printf("Row %zu: [", r);
        for (size_t c = 0; c < d->colCounts[r]; ++c) {
            const char* s = d->rows[r][c] ? d->rows[r][c] : "";
            // Escape backslash and quote for display
            putchar('"');
            for (const char* p = s; *p; ++p) {
                if (*p == '\\' || *p == '"') putchar('\\');
                putchar(*p);
            }
            putchar('"');
            if (c + 1 < d->colCounts[r]) printf(", ");
        }
        printf("]\n");
    }
}

int main(void) {
    const char* tests[5] = {
        "name,age,city\nAlice,30,Seattle",
        "a,\"b,b\",c",
        "row1col1,row1col2\r\nrow2col1,\"row2\ncol2\",row2col3",
        "\"He said \"\"Hello\"\"\",42",
        "1,2,3,\n, ,\"\""
    };
    for (int i = 0; i < 5; ++i) {
        CSVData d = parse_csv(tests[i]);
        printf("Test %d:\n", i + 1);
        print_csv(&d);
        free_csv(&d);
    }
    return 0;
}