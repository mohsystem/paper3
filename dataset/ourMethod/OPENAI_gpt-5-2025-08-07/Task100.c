#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <errno.h>

#define MAX_FILE_SIZE (5 * 1024 * 1024) /* 5 MiB */
#define MAX_PATTERN_LENGTH 10000
#define MAX_MATCHES 10000

typedef struct {
    char **items;
    int count;
} MatchList;

static int is_regular_mode(mode_t mode) {
    return S_ISREG(mode) ? 1 : 0;
}

static int path_within_base(const char *base, const char *target) {
    size_t blen = strlen(base);
    if (strncmp(base, target, blen) != 0) return 0;
    if (target[blen] == '\0') return 1;
    if (base[blen - 1] == '/') return 1;
    if (target[blen] == '/') return 1;
    return 0;
}

static int is_symlink_path(const char *path) {
    struct stat st;
    if (lstat(path, &st) != 0) {
        return 1; /* treat error as unsafe */
    }
    if (S_ISLNK(st.st_mode)) {
        return 1;
    }
    return 0;
}

static void free_match_list(MatchList *ml) {
    if (!ml) return;
    for (int i = 0; i < ml->count; ++i) {
        free(ml->items[i]);
    }
    free(ml->items);
    ml->items = NULL;
    ml->count = 0;
}

MatchList search_file(const char *pattern, const char *filePath) {
    MatchList out;
    out.items = NULL;
    out.count = 0;

    if (!pattern || !filePath) {
        fprintf(stderr, "Invalid input\n");
        return out;
    }
    if (strlen(pattern) == 0 || strlen(pattern) > MAX_PATTERN_LENGTH) {
        fprintf(stderr, "Invalid pattern length\n");
        return out;
    }

    char base[PATH_MAX];
    if (!getcwd(base, sizeof(base))) {
        fprintf(stderr, "getcwd failed\n");
        return out;
    }

    char joined[PATH_MAX];
    if (filePath[0] == '/') {
        snprintf(joined, sizeof(joined), "%s", filePath);
    } else {
        if (snprintf(joined, sizeof(joined), "%s/%s", base, filePath) >= (int)sizeof(joined)) {
            fprintf(stderr, "Path too long\n");
            return out;
        }
    }

    if (is_symlink_path(joined)) {
        fprintf(stderr, "Symbolic links are not allowed\n");
        return out;
    }

    char resolved[PATH_MAX];
    if (!realpath(joined, resolved)) {
        fprintf(stderr, "realpath failed: %s\n", strerror(errno));
        return out;
    }

    if (!path_within_base(base, resolved)) {
        fprintf(stderr, "Path escapes base directory\n");
        return out;
    }

    struct stat st;
    if (lstat(resolved, &st) != 0) {
        fprintf(stderr, "lstat failed: %s\n", strerror(errno));
        return out;
    }
    if (!is_regular_mode(st.st_mode)) {
        fprintf(stderr, "Not a regular file\n");
        return out;
    }
    if (st.st_size < 0 || st.st_size > MAX_FILE_SIZE) {
        fprintf(stderr, "File too large\n");
        return out;
    }

    int fd;
#ifdef O_NOFOLLOW
    fd = open(resolved, O_RDONLY | O_NOFOLLOW);
    if (fd < 0 && errno == ELOOP) {
        fprintf(stderr, "Refusing to open symlink\n");
        return out;
    }
    if (fd < 0) {
        fd = open(resolved, O_RDONLY);
    }
#else
    fd = open(resolved, O_RDONLY);
#endif
    if (fd < 0) {
        fprintf(stderr, "open failed: %s\n", strerror(errno));
        return out;
    }

    char *buf = (char*)malloc((size_t)st.st_size + 1);
    if (!buf) {
        close(fd);
        fprintf(stderr, "Memory allocation failed\n");
        return out;
    }

    ssize_t total = 0;
    while (total < st.st_size) {
        ssize_t r = read(fd, buf + total, (size_t)(st.st_size - total));
        if (r < 0) {
            if (errno == EINTR) continue;
            fprintf(stderr, "read failed: %s\n", strerror(errno));
            free(buf);
            close(fd);
            return out;
        }
        if (r == 0) break;
        total += r;
    }
    close(fd);
    buf[total] = '\0';

    regex_t re;
    int rc = regcomp(&re, pattern, REG_EXTENDED);
    if (rc != 0) {
        char errbuf[256];
        regerror(rc, &re, errbuf, sizeof(errbuf));
        fprintf(stderr, "Invalid regex: %s\n", errbuf);
        free(buf);
        return out;
    }

    size_t cap = 16;
    out.items = (char**)calloc(cap, sizeof(char*));
    if (!out.items) {
        fprintf(stderr, "Memory allocation failed\n");
        regfree(&re);
        free(buf);
        return out;
    }

    int matches = 0;
    regmatch_t pm[1];
    size_t offset = 0;
    size_t buflen = (size_t)total;

    while (offset <= buflen && matches < MAX_MATCHES) {
        rc = regexec(&re, buf + offset, 1, pm, 0);
        if (rc == REG_NOMATCH) {
            break;
        } else if (rc != 0) {
            char errbuf[256];
            regerror(rc, &re, errbuf, sizeof(errbuf));
            fprintf(stderr, "Regex exec error: %s\n", errbuf);
            break;
        } else {
            if (pm[0].rm_so >= 0 && pm[0].rm_eo >= pm[0].rm_so) {
                size_t start = (size_t)pm[0].rm_so;
                size_t end = (size_t)pm[0].rm_eo;
                size_t len = end - start;
                char *m = (char*)malloc(len + 1);
                if (!m) {
                    fprintf(stderr, "Memory allocation failed\n");
                    break;
                }
                memcpy(m, buf + offset + start, len);
                m[len] = '\0';
                if ((size_t)matches >= cap) {
                    size_t ncap = cap * 2;
                    char **tmp = (char**)realloc(out.items, ncap * sizeof(char*));
                    if (!tmp) {
                        fprintf(stderr, "Memory allocation failed\n");
                        free(m);
                        break;
                    }
                    out.items = tmp;
                    memset(out.items + cap, 0, (ncap - cap) * sizeof(char*));
                    cap = ncap;
                }
                out.items[matches++] = m;
                if (len == 0) {
                    offset += end + 1; /* avoid infinite loop for zero-length matches */
                } else {
                    offset += end;
                }
            } else {
                break;
            }
        }
    }

    regfree(&re);
    free(buf);
    out.count = matches;
    return out;
}

static int create_file(const char *name, const char *content) {
    char base[PATH_MAX];
    if (!getcwd(base, sizeof(base))) return -1;

    char path[PATH_MAX];
    if (snprintf(path, sizeof(path), "%s/%s", base, name) >= (int)sizeof(path)) return -1;

    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC
#ifdef O_NOFOLLOW
                  | O_NOFOLLOW
#endif
                  , 0600);
    if (fd < 0) {
        /* fallback without O_NOFOLLOW if needed */
        fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
        if (fd < 0) return -1;
    }
    size_t len = strlen(content);
    ssize_t w = write(fd, content, len);
    close(fd);
    return (w == (ssize_t)len) ? 0 : -1;
}

static void print_results(const MatchList *ml) {
    printf("Matches: %d\n", ml->count);
    for (int i = 0; i < ml->count; ++i) {
        char *s = ml->items[i];
        size_t n = strlen(s);
        char buf[256];
        size_t k = 0;
        for (size_t j = 0; j < n && k < sizeof(buf) - 1; ++j) {
            if (s[j] == '\n') {
                if (k + 2 >= sizeof(buf)) break;
                buf[k++] = '\\'; buf[k++] = 'n';
            } else if (s[j] == '\r') {
                if (k + 2 >= sizeof(buf)) break;
                buf[k++] = '\\'; buf[k++] = 'r';
            } else {
                buf[k++] = s[j];
            }
            if (k >= 200) break;
        }
        if (k >= 200) {
            if (k + 3 < sizeof(buf)) {
                buf[k++] = '.';
                buf[k++] = '.';
                buf[k++] = '.';
            }
        }
        buf[k] = '\0';
        printf("[%d] %s\n", i, buf);
    }
}

int main(int argc, char **argv) {
    if (argc == 3) {
        MatchList ml = search_file(argv[1], argv[2]);
        print_results(&ml);
        free_match_list(&ml);
        if (ml.count == 0) {
            return 1; /* indicate no matches or error */
        }
        return 0;
    }

    /* 5 test cases */
    if (create_file("c_test1.txt", "Hello world!\nThis is a test file.") != 0 ||
        create_file("c_test2.txt", "Numbers: 123 456 789\nEnd.") != 0 ||
        create_file("c_test3.txt", "First line.\nSecond line.\nThird line.") != 0 ||
        create_file("c_test4.txt", "Contacts: alice@example.com, bob@test.com") != 0 ||
        create_file("c_test5.txt", "No matching tokens here.") != 0) {
        fprintf(stderr, "Failed to create test files\n");
        return 1;
    }

    MatchList r1 = search_file("world", "c_test1.txt");
    printf("Test1: pattern='world' file='c_test1.txt'\n");
    print_results(&r1);
    free_match_list(&r1);

    MatchList r2 = search_file("[0-9]+", "c_test2.txt");
    printf("Test2: pattern='[0-9]+' file='c_test2.txt'\n");
    print_results(&r2);
    free_match_list(&r2);

    MatchList r3 = search_file("Second line\\.", "c_test3.txt");
    printf("Test3: pattern='Second line\\.' file='c_test3.txt'\n");
    print_results(&r3);
    free_match_list(&r3);

    MatchList r4 = search_file("[A-Za-z]+@[A-Za-z]+\\.com", "c_test4.txt");
    printf("Test4: pattern='[A-Za-z]+@[A-Za-z]+\\.com' file='c_test4.txt'\n");
    print_results(&r4);
    free_match_list(&r4);

    MatchList r5 = search_file("XYZ", "c_test5.txt");
    printf("Test5: pattern='XYZ' file='c_test5.txt'\n");
    print_results(&r5);
    free_match_list(&r5);

    return 0;
}