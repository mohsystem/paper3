#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#define MAX_BYTES (5 * 1024 * 1024) /* 5 MB */
#define MAX_PATTERN_LEN 10000
#define MAX_MATCHES 100000

typedef struct {
    char **items;
    size_t count;
} MatchList;

static void free_match_list(MatchList *ml) {
    if (!ml) return;
    if (ml->items) {
        for (size_t i = 0; i < ml->count; ++i) {
            free(ml->items[i]);
        }
        free(ml->items);
    }
    ml->items = NULL;
    ml->count = 0;
}

static int is_regular_file_nofollow(const char *path) {
    struct stat st;
    if (lstat(path, &st) != 0) return 0;
    return S_ISREG(st.st_mode);
}

static char* read_file_limited(const char *path, size_t *out_size) {
    struct stat st;
    if (lstat(path, &st) != 0) {
        return NULL;
    }
    if (!S_ISREG(st.st_mode)) {
        errno = EINVAL;
        return NULL;
    }
    if ((size_t)st.st_size > MAX_BYTES) {
        errno = EFBIG;
        return NULL;
    }
    FILE *fp = fopen(path, "rb");
    if (!fp) return NULL;
    size_t sz = (size_t)st.st_size;
    char *buf = (char*)malloc(sz + 1);
    if (!buf) {
        fclose(fp);
        errno = ENOMEM;
        return NULL;
    }
    size_t rd = fread(buf, 1, sz, fp);
    fclose(fp);
    buf[rd] = '\0';
    if (out_size) *out_size = rd;
    return buf;
}

MatchList search_in_file(const char *pattern, const char *file_path) {
    MatchList result;
    result.items = NULL;
    result.count = 0;

    if (!pattern || !file_path) {
        return result;
    }
    if (strlen(pattern) > MAX_PATTERN_LEN) {
        return result;
    }
    if (!is_regular_file_nofollow(file_path)) {
        return result;
    }

    size_t content_size = 0;
    char *content = read_file_limited(file_path, &content_size);
    if (!content) {
        return result;
    }

    regex_t re;
    int rc = regcomp(&re, pattern, REG_EXTENDED);
    if (rc != 0) {
        free(content);
        return result;
    }

    size_t cap = 16;
    result.items = (char**)calloc(cap, sizeof(char*));
    if (!result.items) {
        regfree(&re);
        free(content);
        return result;
    }

    size_t offset = 0;
    regmatch_t pm[1];
    size_t safety_counter = 0;

    while (offset <= content_size && safety_counter < MAX_MATCHES) {
        int exec_rc = regexec(&re, content + offset, 1, pm, 0);
        if (exec_rc == REG_NOMATCH) {
            break;
        } else if (exec_rc != 0) {
            break;
        }
        size_t so = (size_t)pm[0].rm_so;
        size_t eo = (size_t)pm[0].rm_eo;
        if (eo >= so) {
            size_t mlen = eo - so;
            char *m = (char*)malloc(mlen + 1);
            if (!m) {
                break;
            }
            memcpy(m, content + offset + so, mlen);
            m[mlen] = '\0';
            if (result.count == cap) {
                cap *= 2;
                char **tmp = (char**)realloc(result.items, cap * sizeof(char*));
                if (!tmp) {
                    free(m);
                    break;
                }
                result.items = tmp;
            }
            result.items[result.count++] = m;
        }
        size_t advance = (pm[0].rm_eo > 0) ? (size_t)pm[0].rm_eo : 1;
        offset += advance;
        safety_counter++;
    }

    regfree(&re);
    free(content);
    return result;
}

static int write_temp_file(const char *content, char *out_path, size_t out_size) {
    const char *tmpdir = getenv("TMPDIR");
    if (!tmpdir) tmpdir = "/tmp";
    snprintf(out_path, out_size, "%s/task100_%ld.txt", tmpdir, (long)getpid());
    FILE *fp = fopen(out_path, "wb");
    if (!fp) return -1;
    size_t len = strlen(content);
    if (fwrite(content, 1, len, fp) != len) {
        fclose(fp);
        unlink(out_path);
        return -1;
    }
    fclose(fp);
    return 0;
}

int main(int argc, char **argv) {
    if (argc == 3) {
        MatchList ml = search_in_file(argv[1], argv[2]);
        printf("Matches found: %zu\n", ml.count);
        for (size_t i = 0; i < ml.count; ++i) {
            printf("%s\n", ml.items[i]);
        }
        free_match_list(&ml);
        return 0;
    }

    const char *content = "The quick brown fox jumps over the lazy dog.\nNumbers: 12345 and 67890.\nTHE end.\n";
    const char *patterns[5] = {
        "fox",
        "[A-Za-z]{4}",
        "[Tt][Hh][Ee]",
        "[0-9]+",
        "notfound"
    };
    char path[512];
    if (write_temp_file(content, path, sizeof(path)) != 0) {
        fprintf(stderr, "Failed to create temp file\n");
        return 1;
    }
    printf("Test file: %s\n", path);
    for (int i = 0; i < 5; ++i) {
        printf("Test %d Pattern: %s\n", i + 1, patterns[i]);
        MatchList ml = search_in_file(patterns[i], path);
        printf("Matches found: %zu\n", ml.count);
        for (size_t j = 0; j < ml.count; ++j) {
            printf("%s\n", ml.items[j]);
        }
        free_match_list(&ml);
        printf("---\n");
    }
    unlink(path);
    return 0;
}