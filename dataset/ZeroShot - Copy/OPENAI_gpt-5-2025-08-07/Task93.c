#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    char* key;
    char* value;
} KeyValue;

static void safe_free(void* p) {
    if (p) free(p);
}

static char* strdup_safe(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char* d = (char*)malloc(len + 1);
    if (!d) return NULL;
    memcpy(d, s, len + 1);
    return d;
}

static void trim_inplace(char* s) {
    if (!s) return;
    size_t len = strlen(s);
    size_t start = 0;
    while (start < len && isspace((unsigned char)s[start])) start++;
    size_t end = len;
    while (end > start && isspace((unsigned char)s[end - 1])) end--;
    size_t newlen = end - start;
    if (start > 0 && newlen > 0) memmove(s, s + start, newlen);
    s[newlen] = '\0';
}

static int kv_cmp(const void* a, const void* b) {
    const KeyValue* ka = (const KeyValue*)a;
    const KeyValue* kb = (const KeyValue*)b;
    int kc = strcmp(ka->key, kb->key);
    if (kc != 0) return kc;
    return strcmp(ka->value, kb->value);
}

static int append_kv(KeyValue** arr, size_t* count, size_t* cap, const char* k, const char* v) {
    if (*count >= *cap) {
        size_t ncap = (*cap == 0) ? 16 : (*cap * 2);
        KeyValue* n = (KeyValue*)realloc(*arr, ncap * sizeof(KeyValue));
        if (!n) return -1;
        *arr = n;
        *cap = ncap;
    }
    (*arr)[*count].key = strdup_safe(k);
    (*arr)[*count].value = strdup_safe(v);
    if (!(*arr)[*count].key || !(*arr)[*count].value) return -1;
    (*count)++;
    return 0;
}

static void free_kv_array(KeyValue* arr, size_t count) {
    if (!arr) return;
    for (size_t i = 0; i < count; ++i) {
        safe_free(arr[i].key);
        safe_free(arr[i].value);
    }
    free(arr);
}

static int collect_output_lines(KeyValue* arr, size_t count, char*** out_lines, size_t* out_count) {
    char** lines = (char**)calloc(count, sizeof(char*));
    if (!lines) return -1;
    for (size_t i = 0; i < count; ++i) {
        size_t lk = strlen(arr[i].key);
        size_t lv = strlen(arr[i].value);
        if (lk > (size_t)(SIZE_MAX - lv - 2)) { // check overflow
            for (size_t j = 0; j < i; ++j) free(lines[j]);
            free(lines);
            return -1;
        }
        size_t total = lk + 1 + lv + 1;
        char* s = (char*)malloc(total);
        if (!s) {
            for (size_t j = 0; j < i; ++j) free(lines[j]);
            free(lines);
            return -1;
        }
        memcpy(s, arr[i].key, lk);
        s[lk] = '=';
        memcpy(s + lk + 1, arr[i].value, lv);
        s[lk + 1 + lv] = '\0';
        lines[i] = s;
    }
    *out_lines = lines;
    *out_count = count;
    return 0;
}

int read_and_sort_key_values(const char* filePath, char*** out_lines, size_t* out_count) {
    if (!filePath || !out_lines || !out_count) return -1;
    *out_lines = NULL;
    *out_count = 0;

    FILE* f = fopen(filePath, "rb");
    if (!f) return -1;

    KeyValue* arr = NULL;
    size_t count = 0, cap = 0;
    const size_t CHUNK = 4096;
    char* buf = (char*)malloc(CHUNK);
    if (!buf) {
        fclose(f);
        return -1;
    }
    size_t len = 0;
    int c;

    while ((c = fgetc(f)) != EOF) {
        if (c == '\r') {
            // ignore CR, handle on LF
            continue;
        }
        if (c == '\n') {
            buf[len] = '\0';
            char* line = buf;

            // process line
            trim_inplace(line);
            if (line[0] != '\0' && line[0] != '#' && line[0] != ';') {
                char* eq = strchr(line, '=');
                if (eq && eq != line) {
                    *eq = '\0';
                    char* key = line;
                    char* val = eq + 1;
                    trim_inplace(key);
                    trim_inplace(val);
                    if (key[0] != '\0') {
                        if (append_kv(&arr, &count, &cap, key, val) != 0) {
                            free(buf);
                            fclose(f);
                            free_kv_array(arr, count);
                            return -1;
                        }
                    }
                }
            }
            // reset buffer for next line
            len = 0;
            // allocate new buffer for next line if needed
            if (!buf) {
                buf = (char*)malloc(CHUNK);
                if (!buf) {
                    fclose(f);
                    free_kv_array(arr, count);
                    return -1;
                }
            }
            continue;
        }
        if (len + 1 >= CHUNK && len % CHUNK == CHUNK - 1) {
            // grow buffer
            size_t newsize = ((len / CHUNK) + 2) * CHUNK;
            char* nb = (char*)realloc(buf, newsize);
            if (!nb) {
                free(buf);
                fclose(f);
                free_kv_array(arr, count);
                return -1;
            }
            buf = nb;
        }
        buf[len++] = (char)c;
    }

    // handle last line if not empty and not ended with newline
    if (len > 0) {
        buf[len] = '\0';
        char* line = buf;
        trim_inplace(line);
        if (line[0] != '\0' && line[0] != '#' && line[0] != ';') {
            char* eq = strchr(line, '=');
            if (eq && eq != line) {
                *eq = '\0';
                char* key = line;
                char* val = eq + 1;
                trim_inplace(key);
                trim_inplace(val);
                if (key[0] != '\0') {
                    if (append_kv(&arr, &count, &cap, key, val) != 0) {
                        free(buf);
                        fclose(f);
                        free_kv_array(arr, count);
                        return -1;
                    }
                }
            }
        }
    }
    free(buf);
    fclose(f);

    if (count > 1) {
        qsort(arr, count, sizeof(KeyValue), kv_cmp);
    }

    char** lines = NULL;
    size_t outc = 0;
    if (collect_output_lines(arr, count, &lines, &outc) != 0) {
        free_kv_array(arr, count);
        return -1;
    }
    free_kv_array(arr, count);
    *out_lines = lines;
    *out_count = outc;
    return 0;
}

static int write_file(const char* path, const char* content) {
    FILE* f = fopen(path, "wb");
    if (!f) return -1;
    size_t n = fwrite(content, 1, strlen(content), f);
    fclose(f);
    return (n == strlen(content)) ? 0 : -1;
}

int main(void) {
    const char* f1 = "task93_test1.txt";
    const char* f2 = "task93_test2.txt";
    const char* f3 = "task93_test3.txt";
    const char* f4 = "task93_test4.txt";
    const char* f5 = "task93_test5.txt";

    write_file(f1, "b=2\na=1\nc=3\n");
    write_file(f2, "  x = 10 \n\n=bad\n y= 5  \n  # comment\n");
    write_file(f3, "a=2\na=1\n");
    write_file(f4, "# comment\nfoo=bar\ninvalid line\nk=v=extra\n; another comment\n");
    write_file(f5, "Apple=1\napple=2\nss=double\n A=9 \n");

    const char* files[5] = {f1, f2, f3, f4, f5};
    for (int i = 0; i < 5; ++i) {
        printf("Test %d:\n", i + 1);
        char** lines = NULL;
        size_t count = 0;
        if (read_and_sort_key_values(files[i], &lines, &count) == 0) {
            for (size_t j = 0; j < count; ++j) {
                printf("%s\n", lines[j]);
                free(lines[j]);
            }
            free(lines);
        } else {
            printf("Error processing file: %s\n", files[i]);
        }
    }
    return 0;
}