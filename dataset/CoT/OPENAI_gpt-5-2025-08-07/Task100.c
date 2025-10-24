#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

// Chain-of-Through process in code comments:
// 1) Problem: Compile regex and search file content, returning matches.
// 2) Security: Limit file read size, handle regex/file errors, bound matches, avoid zero-length infinite loops.
// 3) Implementation: Safe file reading with cap, POSIX regex with REG_EXTENDED, bounded loop.
// 4) Review: Checked memory allocation/free, error paths, bounds.
// 5) Output: Secure final code.

typedef struct {
    char** items;
    size_t count;
} Matches;

static void free_matches(Matches m) {
    if (m.items) {
        for (size_t i = 0; i < m.count; ++i) {
            free(m.items[i]);
        }
        free(m.items);
    }
}

static char* read_file_limited(const char* filename, size_t max_bytes, size_t* out_len) {
    *out_len = 0;
    FILE* f = fopen(filename, "rb");
    if (!f) return NULL;

    const size_t bufsize = 8192;
    char* buffer = (char*)malloc((max_bytes > 0 ? max_bytes : 1) + 1);
    if (!buffer) { fclose(f); return NULL; }

    size_t total = 0;
    while (total < max_bytes) {
        size_t to_read = bufsize;
        if (to_read > max_bytes - total) to_read = max_bytes - total;
        size_t got = fread(buffer + total, 1, to_read, f);
        if (got == 0) break;
        total += got;
    }
    buffer[total] = '\0';
    fclose(f);
    *out_len = total;
    return buffer;
}

Matches regex_search_in_file(const char* pattern, const char* filename, size_t max_bytes, size_t max_matches) {
    Matches res = {0};
    if (!pattern || !filename) return res;

    size_t len = 0;
    char* text = read_file_limited(filename, max_bytes > 0 ? max_bytes : 1048576, &len);
    if (!text) return res;

    regex_t re;
    int rc = regcomp(&re, pattern, REG_EXTENDED);
    if (rc != 0) {
        free(text);
        return res;
    }

    size_t capacity = 16;
    res.items = (char**)calloc(capacity, sizeof(char*));
    if (!res.items) {
        regfree(&re);
        free(text);
        return res;
    }

    size_t start = 0;
    while (res.count < max_matches && start <= len) {
        regmatch_t pm[1];
        int flags = 0;
        if (start > 0) flags |= REG_NOTBOL;
        rc = regexec(&re, text + start, 1, pm, flags);
        if (rc == REG_NOMATCH) {
            break;
        } else if (rc != 0) {
            break;
        }

        if (pm[0].rm_so >= 0 && pm[0].rm_eo >= pm[0].rm_so) {
            size_t so = (size_t)pm[0].rm_so;
            size_t eo = (size_t)pm[0].rm_eo;
            size_t mlen = eo - so;

            // store match
            if (res.count == capacity) {
                size_t newcap = capacity * 2;
                char** tmp = (char**)realloc(res.items, newcap * sizeof(char*));
                if (!tmp) break;
                res.items = tmp;
                capacity = newcap;
            }
            char* s = (char*)malloc(mlen + 1);
            if (!s) break;
            memcpy(s, text + start + so, mlen);
            s[mlen] = '\0';
            res.items[res.count++] = s;

            // advance
            if (mlen == 0) {
                if (start + eo >= len) break;
                start = start + eo + 1;
            } else {
                start = start + eo;
            }
        } else {
            break;
        }
    }

    regfree(&re);
    free(text);
    return res;
}

static char* create_temp_file_with_content(const char* content) {
    char tmpl[] = "./task100_XXXXXX";
    int fd = mkstemp(tmpl);
    if (fd == -1) return NULL;

    size_t len = strlen(content);
    ssize_t wrote = write(fd, content, len);
    (void)wrote; // ignore partial writes in sample; in production check!
    close(fd);

    char* path = strdup(tmpl);
    return path;
}

static void run_test(const char* title, const char* pattern, const char* content) {
    char* path = create_temp_file_with_content(content);
    if (!path) {
        printf("[%s] failed to create temp file\n", title);
        return;
    }
    Matches m = regex_search_in_file(pattern, path, 1048576, 10000);
    printf("[%s] pattern=%s file=%s\n", title, pattern, path);
    printf("matches=%zu\n", m.count);
    size_t limit = m.count < 10 ? m.count : 10;
    for (size_t i = 0; i < limit; ++i) {
        printf("%s\n", m.items[i]);
    }
    printf("---\n");
    free_matches(m);
    unlink(path);
    free(path);
}

int main(int argc, char** argv) {
    if (argc >= 3) {
        const char* pattern = argv[1];
        const char* filename = argv[2];
        Matches m = regex_search_in_file(pattern, filename, 1048576, 10000);
        for (size_t i = 0; i < m.count; ++i) {
            printf("%s\n", m.items[i]);
        }
        free_matches(m);
        return 0;
    }

    // 5 test cases
    run_test("T1 simple literal", "foo", "foo bar foo\nbaz");
    run_test("T2 3 letters", "[A-Za-z]{3}", "abc XYZ abcdef");
    run_test("T3 digits", "[0-9]+", "Order 123: 456 items.");
    run_test("T4 end anchor", "end$", "line with end");
    run_test("T5 optional u", "colou?r", "color colour colr");
    return 0;
}