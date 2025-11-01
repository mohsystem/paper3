#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

static int ensure_dir(const char* path) {
    struct stat st;
    if (stat(path, &st) == 0) {
        return S_ISDIR(st.st_mode) ? 0 : -1;
    }
    return mkdir(path, 0700);
}

static char* normalize_join(const char* base_real, const char* requested_path) {
    if (!base_real || !requested_path) return NULL;
    if (requested_path[0] == '/' || strchr(requested_path, '\\') != NULL) return NULL;

    char* result = (char*)malloc(PATH_MAX);
    if (!result) return NULL;
    strncpy(result, base_real, PATH_MAX - 1);
    result[PATH_MAX - 1] = '\0';
    size_t len = strlen(result);

    if (len + 1 < PATH_MAX) {
        if (result[len - 1] != '/') {
            result[len++] = '/';
            result[len] = '\0';
        }
    } else {
        free(result);
        return NULL;
    }

    int stack_idx[256];
    int stack_top = 0;
    stack_idx[stack_top++] = (int)len; // mark after base '/'

    char temp[PATH_MAX];
    strncpy(temp, requested_path, PATH_MAX - 1);
    temp[PATH_MAX - 1] = '\0';

    char* saveptr = NULL;
    char* token = strtok_r(temp, "/", &saveptr);
    while (token) {
        if (strcmp(token, ".") == 0) {
            // skip
        } else if (strcmp(token, "..") == 0) {
            if (stack_top > 1) {
                // pop last component
                len = (size_t)stack_idx[--stack_top];
                result[len] = '\0';
            } else {
                // would escape base
                free(result);
                return NULL;
            }
        } else if (*token != '\0') {
            size_t tlen = strlen(token);
            if (len + tlen + 1 >= PATH_MAX) { free(result); return NULL; }
            memcpy(result + len, token, tlen);
            len += tlen;
            result[len] = '\0';
            // append trailing slash for further components
            if (len + 1 < PATH_MAX) {
                result[len++] = '/';
                result[len] = '\0';
                stack_idx[stack_top++] = (int)len;
            } else {
                free(result);
                return NULL;
            }
        }
        token = strtok_r(NULL, "/", &saveptr);
    }

    // remove trailing slash if present
    if (len > 0 && result[len - 1] == '/') {
        result[--len] = '\0';
    }

    // ensure still within base prefix
    size_t base_len = strlen(base_real);
    if (strncmp(result, base_real, base_len) != 0 || (result[base_len] != '/' && result[base_len] != '\0')) {
        free(result);
        return NULL;
    }

    return result;
}

char* read_file_from_safe_dir(const char* safe_dir, const char* requested_path) {
    if (!safe_dir || !requested_path) return NULL;

    if (ensure_dir(safe_dir) != 0) return NULL;

    char base_real[PATH_MAX];
    if (!realpath(safe_dir, base_real)) {
        // try creating then resolve again
        if (ensure_dir(safe_dir) != 0) return NULL;
        if (!realpath(safe_dir, base_real)) return NULL;
    }

    char* fullpath = normalize_join(base_real, requested_path);
    if (!fullpath) return NULL;

    FILE* f = fopen(fullpath, "rb");
    if (!f) { free(fullpath); return NULL; }

    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); free(fullpath); return NULL; }
    long sz = ftell(f);
    if (sz < 0) { fclose(f); free(fullpath); return NULL; }
    if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); free(fullpath); return NULL; }

    char* buf = (char*)malloc((size_t)sz + 1);
    if (!buf) { fclose(f); free(fullpath); return NULL; }
    size_t rd = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    free(fullpath);
    buf[rd] = '\0';
    return buf;
}

int main(void) {
    const char* base = "safe_dir_c";
    ensure_dir(base);

    // create subdir
    char subdir[PATH_MAX];
    snprintf(subdir, sizeof(subdir), "%s/%s", base, "sub");
    ensure_dir(subdir);

    // create files
    {
        char path1[PATH_MAX]; snprintf(path1, sizeof(path1), "%s/hello.txt", base);
        FILE* f1 = fopen(path1, "wb"); if (f1) { fputs("Hello from C", f1); fclose(f1); }

        char path2[PATH_MAX]; snprintf(path2, sizeof(path2), "%s/sub/inner.txt", base);
        FILE* f2 = fopen(path2, "wb"); if (f2) { fputs("Inner from C", f2); fclose(f2); }
    }

    const char* tests[5];
    tests[0] = "hello.txt";                     // valid
    tests[1] = "sub/inner.txt";                 // valid in subdir
    tests[2] = "../hello.txt";                  // traversal attempt
    // absolute path attempt
    char abs_attempt[PATH_MAX];
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));
    snprintf(abs_attempt, sizeof(abs_attempt), "%s/%s/hello.txt", cwd, base);
    tests[3] = abs_attempt;
    tests[4] = "missing.txt";                   // non-existent

    for (int i = 0; i < 5; ++i) {
        char* content = read_file_from_safe_dir(base, tests[i]);
        if (content) {
            printf("Request: %s => %s\n", tests[i], content);
            free(content);
        } else {
            printf("Request: %s => (blocked or not found)\n", tests[i]);
        }
    }

    return 0;
}