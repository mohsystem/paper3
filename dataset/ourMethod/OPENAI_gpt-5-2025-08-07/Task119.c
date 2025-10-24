#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <limits.h>

#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif

typedef struct {
    char *data;
    size_t len;
} CSVCell;

typedef struct {
    CSVCell *cells;
    size_t n_cells;
    size_t cap_cells;
} CSVRow;

typedef struct {
    CSVRow *rows;
    size_t n_rows;
    size_t cap_rows;
} CSVTable;

typedef struct {
    int ok;
    char msg[128];
} Result;

static void secure_bzero(void *ptr, size_t len) {
    volatile unsigned char *p = (volatile unsigned char *)ptr;
    while (len--) {
        *p++ = 0;
    }
}

static void free_csv_table(CSVTable *t) {
    if (!t) return;
    if (t->rows) {
        for (size_t i = 0; i < t->n_rows; i++) {
            CSVRow *r = &t->rows[i];
            if (r->cells) {
                for (size_t j = 0; j < r->n_cells; j++) {
                    if (r->cells[j].data) {
                        free(r->cells[j].data);
                    }
                }
                free(r->cells);
            }
        }
        free(t->rows);
    }
    t->rows = NULL;
    t->n_rows = 0;
    t->cap_rows = 0;
}

static int check_mul_overflow_size(size_t a, size_t b, size_t *out) {
    if (a == 0 || b == 0) { *out = 0; return 0; }
    if (a > SIZE_MAX / b) return -1;
    *out = a * b;
    return 0;
}

static int check_add_overflow_size(size_t a, size_t b, size_t *out) {
    if (a > SIZE_MAX - b) return -1;
    *out = a + b;
    return 0;
}

static int safe_reallocarray(void **ptr, size_t nmemb, size_t size) {
    size_t total = 0;
    if (check_mul_overflow_size(nmemb, size, &total) != 0) return -1;
    void *p = realloc(*ptr, total == 0 ? 1 : total);
    if (!p && total != 0) return -1;
    *ptr = p;
    return 0;
}

static int grow_rows(CSVTable *t, size_t add, size_t max_rows) {
    if (!t) return -1;
    if (add == 0) return 0;
    if (t->n_rows > SIZE_MAX - add) return -1;
    size_t need = t->n_rows + add;
    if (need > max_rows) return -1;
    if (need <= t->cap_rows) return 0;
    size_t newcap = t->cap_rows ? t->cap_rows : 4;
    while (newcap < need) {
        if (newcap > SIZE_MAX / 2) { newcap = need; break; }
        newcap *= 2;
    }
    if (safe_reallocarray((void **)&t->rows, newcap, sizeof(CSVRow)) != 0) return -1;
    // zero-init new slots' metadata
    for (size_t i = t->cap_rows; i < newcap; i++) {
        t->rows[i].cells = NULL;
        t->rows[i].n_cells = 0;
        t->rows[i].cap_cells = 0;
    }
    t->cap_rows = newcap;
    return 0;
}

static int grow_cells(CSVRow *r, size_t add, size_t max_cols) {
    if (!r) return -1;
    if (add == 0) return 0;
    if (r->n_cells > SIZE_MAX - add) return -1;
    size_t need = r->n_cells + add;
    if (need > max_cols) return -1;
    if (need <= r->cap_cells) return 0;
    size_t newcap = r->cap_cells ? r->cap_cells : 4;
    while (newcap < need) {
        if (newcap > SIZE_MAX / 2) { newcap = need; break; }
        newcap *= 2;
    }
    if (safe_reallocarray((void **)&r->cells, newcap, sizeof(CSVCell)) != 0) return -1;
    for (size_t i = r->cap_cells; i < newcap; i++) {
        r->cells[i].data = NULL;
        r->cells[i].len = 0;
    }
    r->cap_cells = newcap;
    return 0;
}

static int push_row(CSVTable *t, CSVRow *cur, size_t max_rows) {
    if (grow_rows(t, 1, max_rows) != 0) return -1;
    t->rows[t->n_rows] = *cur; // move
    t->n_rows++;
    // reset cur to empty to avoid double free by caller
    cur->cells = NULL;
    cur->n_cells = 0;
    cur->cap_cells = 0;
    return 0;
}

static int append_cell(CSVRow *r, const char *data, size_t len, size_t max_cols, size_t max_cell_len) {
    if (len > max_cell_len) return -1;
    if (grow_cells(r, 1, max_cols) != 0) return -1;
    char *copy = NULL;
    size_t allocsz = 0;
    if (check_add_overflow_size(len, 1, &allocsz) != 0) return -1;
    copy = (char *)malloc(allocsz);
    if (!copy) return -1;
    if (len > 0) {
        memcpy(copy, data, len);
    }
    copy[len] = '\0';
    r->cells[r->n_cells].data = copy;
    r->cells[r->n_cells].len = len;
    r->n_cells++;
    return 0;
}

static int is_safe_rel_name(const char *name) {
    if (!name) return 0;
    size_t n = strnlen(name, 512);
    if (n == 0 || n >= 256) return 0;
    for (size_t i = 0; i < n; i++) {
        unsigned char c = (unsigned char)name[i];
        if (c == '/' || c == '\\') return 0;
        if (c == '.') {
            // disallow path traversal patterns ".." or leading dot
            if (i == 0) return 0;
            if (i + 1 < n && name[i + 1] == '.') return 0;
        }
        if (!(isalnum(c) || c == '_' || c == '-' || c == '.' )) return 0;
    }
    return 1;
}

static int write_file_in_dir_fd(int dirfd, const char *rel_name, const char *data, size_t len) {
    if (!is_safe_rel_name(rel_name)) return -1;
    int fd = openat(dirfd, rel_name, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0600);
    if (fd < 0) return -1;
    ssize_t w = 0;
    size_t off = 0;
    while (off < len) {
        size_t chunk = len - off;
        if (chunk > SSIZE_MAX) chunk = SSIZE_MAX;
        w = write(fd, data + off, chunk);
        if (w < 0) {
            int saved = errno;
            close(fd);
            errno = saved;
            return -1;
        }
        off += (size_t)w;
    }
    if (fsync(fd) != 0) {
        int saved = errno;
        close(fd);
        errno = saved;
        return -1;
    }
    if (close(fd) != 0) return -1;
    if (fsync(dirfd) != 0) return -1;
    return 0;
}

static int open_dir_fd(const char *base_dir) {
    if (!base_dir) return -1;
    int dfd = open(base_dir, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (dfd < 0) return -1;
    struct stat st;
    if (fstat(dfd, &st) != 0) {
        int saved = errno;
        close(dfd);
        errno = saved;
        return -1;
    }
    if (!S_ISDIR(st.st_mode)) {
        close(dfd);
        errno = ENOTDIR;
        return -1;
    }
    return dfd;
}

static int read_file_from_dir(const char *base_dir, const char *rel_name, size_t max_size, unsigned char **out_buf, size_t *out_len) {
    if (!out_buf || !out_len) return -1;
    *out_buf = NULL;
    *out_len = 0;
    if (!is_safe_rel_name(rel_name)) return -1;
    int dfd = open_dir_fd(base_dir);
    if (dfd < 0) return -1;

    int fd = openat(dfd, rel_name, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (fd < 0) {
        int saved = errno;
        close(dfd);
        errno = saved;
        return -1;
    }
    struct stat st;
    if (fstat(fd, &st) != 0) {
        int saved = errno;
        close(fd);
        close(dfd);
        errno = saved;
        return -1;
    }
    if (!S_ISREG(st.st_mode)) {
        close(fd);
        close(dfd);
        errno = EINVAL;
        return -1;
    }
    if (st.st_size < 0) {
        close(fd);
        close(dfd);
        errno = EINVAL;
        return -1;
    }
    size_t fsz = (size_t)st.st_size;
    if (fsz > max_size) {
        close(fd);
        close(dfd);
        errno = EFBIG;
        return -1;
    }
    size_t allocsz = 0;
    if (check_add_overflow_size(fsz, 1, &allocsz) != 0) {
        close(fd);
        close(dfd);
        errno = EOVERFLOW;
        return -1;
    }
    unsigned char *buf = (unsigned char *)malloc(allocsz);
    if (!buf) {
        close(fd);
        close(dfd);
        return -1;
    }
    size_t off = 0;
    while (off < fsz) {
        size_t chunk = fsz - off;
        if (chunk > SSIZE_MAX) chunk = SSIZE_MAX;
        ssize_t r = read(fd, buf + off, chunk);
        if (r < 0) {
            int saved = errno;
            free(buf);
            close(fd);
            close(dfd);
            errno = saved;
            return -1;
        }
        if (r == 0) break;
        off += (size_t)r;
    }
    // Null-terminate for convenience, though parser uses lengths.
    buf[off] = 0;
    close(fd);
    close(dfd);
    *out_buf = buf;
    *out_len = off;
    return 0;
}

static int is_newline_at(const unsigned char *buf, size_t len, size_t i, size_t *adv) {
    if (i >= len) return 0;
    if (buf[i] == '\n') { if (adv) *adv = 1; return 1; }
    if (buf[i] == '\r') {
        if (i + 1 < len && buf[i + 1] == '\n') { if (adv) *adv = 2; return 1; }
        if (adv) *adv = 1;
        return 1;
    }
    return 0;
}

static int parse_csv_buffer(const unsigned char *buf, size_t len, char delimiter, CSVTable *out, char *errmsg, size_t errmsgsz) {
    if (!buf || !out) return -1;
    if (delimiter == '\0' || delimiter == '\r' || delimiter == '\n' || delimiter == '"') return -1;

    const size_t MAX_ROWS = 200000;
    const size_t MAX_COLS = 4096;
    const size_t MAX_CELL_LEN = 4 * 1024 * 1024; // 4 MiB cap per cell

    out->rows = NULL;
    out->n_rows = 0;
    out->cap_rows = 0;
    CSVRow cur; cur.cells = NULL; cur.n_cells = 0; cur.cap_cells = 0;

    size_t i = 0;
    int status = -1;

    while (i < len) {
        // Parse field
        if (buf[i] == '"') {
            i++; // consume opening quote
            // dynamic buffer for quoted content
            char *field = NULL;
            size_t flen = 0, fcap = 0;
            int closed = 0;
            while (i < len) {
                unsigned char c = buf[i];
                if (c == '"') {
                    if (i + 1 < len && buf[i + 1] == '"') {
                        // escaped quote
                        if (flen + 1 > MAX_CELL_LEN) { if (errmsg && errmsgsz) snprintf(errmsg, errmsgsz, "Invalid CSV"); goto fail; }
                        if (flen + 1 > fcap) {
                            size_t newcap = fcap ? fcap * 2 : 32;
                            while (newcap < flen + 1) {
                                if (newcap > SIZE_MAX / 2) { newcap = flen + 1; break; }
                                newcap *= 2;
                            }
                            if (safe_reallocarray((void **)&field, newcap, sizeof(char)) != 0) { if (errmsg && errmsgsz) snprintf(errmsg, errmsgsz, "Invalid CSV"); goto fail; }
                            fcap = newcap;
                        }
                        field[flen++] = '"';
                        i += 2;
                    } else {
                        // closing quote
                        i++;
                        closed = 1;
                        break;
                    }
                } else {
                    // regular char inside quotes (including CR/LF)
                    if (flen + 1 > MAX_CELL_LEN) { if (errmsg && errmsgsz) snprintf(errmsg, errmsgsz, "Invalid CSV"); goto fail; }
                    if (flen + 1 > fcap) {
                        size_t newcap = fcap ? fcap * 2 : 32;
                        while (newcap < flen + 1) {
                            if (newcap > SIZE_MAX / 2) { newcap = flen + 1; break; }
                            newcap *= 2;
                        }
                        if (safe_reallocarray((void **)&field, newcap, sizeof(char)) != 0) { if (errmsg && errmsgsz) snprintf(errmsg, errmsgsz, "Invalid CSV"); goto fail; }
                        fcap = newcap;
                    }
                    field[flen++] = (char)c;
                    i++;
                }
            }
            if (!closed) { if (errmsg && errmsgsz) snprintf(errmsg, errmsgsz, "Invalid CSV"); goto fail; }
            // After closing quote, allow optional spaces/tabs before delimiter/newline/end
            while (i < len && (buf[i] == ' ' || buf[i] == '\t')) i++;

            // Determine what follows
            if (i < len && buf[i] != (unsigned char)delimiter) {
                size_t adv = 0;
                if (!is_newline_at(buf, len, i, &adv)) {
                    // invalid trailing content
                    if (errmsg && errmsgsz) snprintf(errmsg, errmsgsz, "Invalid CSV");
                    free(field);
                    goto fail;
                }
            }

            if (append_cell(&cur, field ? field : "", flen, MAX_COLS, MAX_CELL_LEN) != 0) {
                if (errmsg && errmsgsz) snprintf(errmsg, errmsgsz, "Invalid CSV");
                free(field);
                goto fail;
            }
            if (field) { secure_bzero(field, flen); free(field); field = NULL; }

            // Delimiter, newline, or end
            if (i >= len) {
                break;
            } else if (buf[i] == (unsigned char)delimiter) {
                i++; // next field
                continue;
            } else {
                size_t adv = 0;
                if (is_newline_at(buf, len, i, &adv)) {
                    i += adv;
                    // finish row
                    if (push_row(out, &cur, MAX_ROWS) != 0) { if (errmsg && errmsgsz) snprintf(errmsg, errmsgsz, "Invalid CSV"); goto fail; }
                    continue;
                } else {
                    // end?
                    continue;
                }
            }
        } else {
            // unquoted field
            size_t start = i;
            size_t adv = 0;
            while (i < len) {
                unsigned char c = buf[i];
                if (c == (unsigned char)delimiter) break;
                if (is_newline_at(buf, len, i, &adv)) break;
                i++;
            }
            size_t end = i;
            // If just before newline and it's CRLF or CR, do not include CR in field
            if (end > start && buf[end - 1] == '\r') {
                end--;
            }
            size_t flen = end - start;
            if (append_cell(&cur, (const char *)(buf + start), flen, MAX_COLS, MAX_CELL_LEN) != 0) { if (errmsg && errmsgsz) snprintf(errmsg, errmsgsz, "Invalid CSV"); goto fail; }
            if (i >= len) {
                break;
            } else if (buf[i] == (unsigned char)delimiter) {
                i++; // consume delimiter, keep parsing same row
                continue;
            } else if (is_newline_at(buf, len, i, &adv)) {
                i += adv;
                // finish row
                if (push_row(out, &cur, MAX_ROWS) != 0) { if (errmsg && errmsgsz) snprintf(errmsg, errmsgsz, "Invalid CSV"); goto fail; }
                continue;
            }
        }
    }

    // end of buffer: if current row has cells, push it
    if (cur.n_cells > 0 || out->n_rows == 0) {
        if (push_row(out, &cur, MAX_ROWS) != 0) { if (errmsg && errmsgsz) snprintf(errmsg, errmsgsz, "Invalid CSV"); goto fail; }
    }
    status = 0;

fail:
    if (status != 0) {
        // cleanup
        if (cur.cells) {
            for (size_t j = 0; j < cur.n_cells; j++) {
                free(cur.cells[j].data);
            }
            free(cur.cells);
        }
        free_csv_table(out);
    }
    return status;
}

static void print_csv_table(const CSVTable *t) {
    if (!t) return;
    for (size_t i = 0; i < t->n_rows; i++) {
        const CSVRow *r = &t->rows[i];
        for (size_t j = 0; j < r->n_cells; j++) {
            const CSVCell *c = &r->cells[j];
            // Print with basic escaping for visibility
            fputc('"', stdout);
            for (size_t k = 0; k < c->len; k++) {
                unsigned char ch = (unsigned char)c->data[k];
                if (ch == '"' || ch == '\\') {
                    fputc('\\', stdout);
                    fputc(ch, stdout);
                } else if (ch == '\n') {
                    fputc('\\', stdout); fputc('n', stdout);
                } else if (ch == '\r') {
                    fputc('\\', stdout); fputc('r', stdout);
                } else {
                    fputc(ch, stdout);
                }
            }
            fputc('"', stdout);
            if (j + 1 < r->n_cells) fputc(',', stdout);
        }
        fputc('\n', stdout);
    }
}

static Result process_csv_file(const char *base_dir, const char *rel_name, char delimiter, CSVTable *out_table) {
    Result res = { .ok = 0, .msg = {0} };
    unsigned char *buf = NULL;
    size_t len = 0;
    const size_t MAX_FILE = 16 * 1024 * 1024; // 16 MiB
    if (read_file_from_dir(base_dir, rel_name, MAX_FILE, &buf, &len) != 0) {
        snprintf(res.msg, sizeof(res.msg), "Read error");
        return res;
    }
    char err[128] = {0};
    if (parse_csv_buffer(buf, len, delimiter, out_table, err, sizeof(err)) != 0) {
        snprintf(res.msg, sizeof(res.msg), "Parse error");
        free(buf);
        return res;
    }
    free(buf);
    res.ok = 1;
    snprintf(res.msg, sizeof(res.msg), "OK");
    return res;
}

static int make_test_dir(char *out_path, size_t out_sz) {
    if (!out_path || out_sz < 32) return -1;
    // Template for mkdtemp must end with XXXXXX
    const char *prefix = "/tmp/csvsafe_";
    char tmpl[PATH_MAX];
    int n = snprintf(tmpl, sizeof(tmpl), "%sXXXXXXXX", prefix);
    if (n <= 0 || (size_t)n >= sizeof(tmpl)) return -1;
    char *p = strdup(tmpl);
    if (!p) return -1;
    char *made = mkdtemp(p);
    if (!made) {
        free(p);
        return -1;
    }
    if (strlen(made) >= out_sz) {
        free(p);
        errno = ENAMETOOLONG;
        return -1;
    }
    strncpy(out_path, made, out_sz - 1);
    out_path[out_sz - 1] = '\0';
    free(p);
    return 0;
}

int main(void) {
    // Create a secure temp directory for tests
    char testdir[PATH_MAX] = {0};
    if (make_test_dir(testdir, sizeof(testdir)) != 0) {
        fprintf(stderr, "Failed to create temp dir\n");
        return 1;
    }
    int dirfd = open_dir_fd(testdir);
    if (dirfd < 0) {
        fprintf(stderr, "Failed to open temp dir\n");
        return 1;
    }

    // Prepare 5 test CSV files (content kept small)
    struct {
        const char *name;
        const char *content;
        int expect_ok;
        const char *desc;
    } cases[5] = {
        {
            "t1.csv",
            "a,b,c\n1,2,3\n",
            1,
            "Simple CSV"
        },
        {
            "t2.csv",
            "\"hello, world\",42,\"line1\nline2\"\n",
            1,
            "Quotes, comma, and newline in field"
        },
        {
            "t3.csv",
            "\"She said \"\"Hi\"\"\",X\n",
            1,
            "Embedded quotes"
        },
        {
            "t4.csv",
            "\"CRLF\"\r\n1,2,3\r\n",
            1,
            "CRLF newlines"
        },
        {
            "t5.csv",
            "\"bad\"x,1\n",
            0,
            "Invalid trailing characters after quoted field"
        }
    };

    for (size_t i = 0; i < 5; i++) {
        if (write_file_in_dir_fd(dirfd, cases[i].name, cases[i].content, strlen(cases[i].content)) != 0) {
            fprintf(stderr, "Failed to write %s\n", cases[i].name);
            close(dirfd);
            return 1;
        }
    }

    // Run tests
    for (size_t i = 0; i < 5; i++) {
        CSVTable table;
        memset(&table, 0, sizeof(table));
        Result r = process_csv_file(testdir, cases[i].name, ',', &table);
        printf("Test %zu - %s: %s\n", i + 1, cases[i].desc, r.ok ? "OK" : "FAIL");
        if (r.ok != cases[i].expect_ok) {
            printf("  Expected %s but got %s\n", cases[i].expect_ok ? "OK" : "FAIL", r.ok ? "OK" : "FAIL");
        }
        if (r.ok) {
            printf("  Parsed %zu rows\n", table.n_rows);
            print_csv_table(&table);
        }
        free_csv_table(&table);
    }

    close(dirfd);
    // Cleanup directory contents and dir (best-effort)
    // Unlink files
    for (size_t i = 0; i < 5; i++) {
        char path[PATH_MAX];
        int n = snprintf(path, sizeof(path), "%s/%s", testdir, cases[i].name);
        if (n > 0 && (size_t)n < sizeof(path)) unlink(path);
    }
    rmdir(testdir);
    return 0;
}