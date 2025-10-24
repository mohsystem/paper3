#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdbool.h>

static bool is_dir(const char* path) {
    struct stat st;
    if (stat(path, &st) != 0) return false;
    return S_ISDIR(st.st_mode);
}

static bool is_regular_file(const char* path) {
    struct stat st;
    if (stat(path, &st) != 0) return false;
    return S_ISREG(st.st_mode);
}

char* safe_copy_to_temp(const char* base_dir, const char* relative_path, const char* temp_dir) {
    if (!base_dir || !relative_path) {
        fprintf(stderr, "Invalid arguments: base_dir and relative_path must not be NULL.\n");
        return NULL;
    }

    char base_real[PATH_MAX];
    if (!realpath(base_dir, base_real)) {
        fprintf(stderr, "Unable to resolve base directory: %s\n", strerror(errno));
        return NULL;
    }
    if (!is_dir(base_real)) {
        fprintf(stderr, "Base is not a directory.\n");
        return NULL;
    }

    // Build candidate path
    char candidate[PATH_MAX];
    int n = snprintf(candidate, sizeof(candidate), "%s/%s", base_real, relative_path);
    if (n < 0 || (size_t)n >= sizeof(candidate)) {
        fprintf(stderr, "Path too long.\n");
        return NULL;
    }

    char candidate_real[PATH_MAX];
    if (!realpath(candidate, candidate_real)) {
        fprintf(stderr, "Unable to resolve source file: %s\n", strerror(errno));
        return NULL;
    }

    size_t base_len = strlen(base_real);
    if (!(strncmp(candidate_real, base_real, base_len) == 0 &&
          (candidate_real[base_len] == '/' || candidate_real[base_len] == '\0'))) {
        fprintf(stderr, "Security violation: Attempted path traversal outside base directory.\n");
        return NULL;
    }

    if (!is_regular_file(candidate_real)) {
        fprintf(stderr, "Source is not a regular file.\n");
        return NULL;
    }

    char temp_base[PATH_MAX];
    if (temp_dir && temp_dir[0] != '\0') {
        if (!realpath(temp_dir, temp_base)) {
            fprintf(stderr, "Unable to resolve temp directory: %s\n", strerror(errno));
            return NULL;
        }
    } else {
        const char* env_tmp = getenv("TMPDIR");
        if (!env_tmp || env_tmp[0] == '\0') env_tmp = "/tmp";
        if (!realpath(env_tmp, temp_base)) {
            // Fallback if realpath fails (e.g., path does not exist)
            strncpy(temp_base, env_tmp, sizeof(temp_base) - 1);
            temp_base[sizeof(temp_base) - 1] = '\0';
        }
    }

    char tmpl[PATH_MAX];
    n = snprintf(tmpl, sizeof(tmpl), "%s/%s", temp_base, "safe_copy_XXXXXX");
    if (n < 0 || (size_t)n >= sizeof(tmpl)) {
        fprintf(stderr, "Temp path too long.\n");
        return NULL;
    }

    int out_fd = mkstemp(tmpl);
    if (out_fd == -1) {
        fprintf(stderr, "Unable to create temporary file: %s\n", strerror(errno));
        return NULL;
    }
    (void)fchmod(out_fd, S_IRUSR | S_IWUSR); // 0600

    int in_fd = open(candidate_real, O_RDONLY
#ifdef O_CLOEXEC
    | O_CLOEXEC
#endif
#ifdef O_NOFOLLOW
    | O_NOFOLLOW
#endif
    );
    if (in_fd == -1) {
        fprintf(stderr, "Unable to open source file: %s\n", strerror(errno));
        close(out_fd);
        unlink(tmpl);
        return NULL;
    }

    char buffer[8192];
    ssize_t r;
    while ((r = read(in_fd, buffer, sizeof(buffer))) > 0) {
        ssize_t off = 0;
        while (off < r) {
            ssize_t w = write(out_fd, buffer + off, (size_t)(r - off));
            if (w <= 0) {
                fprintf(stderr, "Write error: %s\n", strerror(errno));
                close(in_fd);
                close(out_fd);
                unlink(tmpl);
                return NULL;
            }
            off += w;
        }
    }
    if (r < 0) {
        fprintf(stderr, "Read error: %s\n", strerror(errno));
        close(in_fd);
        close(out_fd);
        unlink(tmpl);
        return NULL;
    }

    close(in_fd);
    if (close(out_fd) != 0) {
        fprintf(stderr, "Close error: %s\n", strerror(errno));
        unlink(tmpl);
        return NULL;
    }

    char* result = strdup(tmpl);
    if (!result) {
        fprintf(stderr, "Memory allocation failed.\n");
        unlink(tmpl);
        return NULL;
    }
    return result;
}

static int write_text_file(const char* path, const char* content) {
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (fd < 0) return -1;
    size_t len = strlen(content);
    ssize_t w = write(fd, content, len);
    close(fd);
    return (w == (ssize_t)len) ? 0 : -1;
}

int main(void) {
    char base_tmpl[] = "/tmp/task127_c_base_XXXXXX";
    char* base_dir = mkdtemp(base_tmpl);
    if (!base_dir) {
        fprintf(stderr, "Failed to create base directory: %s\n", strerror(errno));
        return 1;
    }

    char a_path[PATH_MAX];
    snprintf(a_path, sizeof(a_path), "%s/%s", base_dir, "a.txt");
    write_text_file(a_path, "Hello A");

    char sub_dir[PATH_MAX];
    snprintf(sub_dir, sizeof(sub_dir), "%s/%s", base_dir, "sub");
    mkdir(sub_dir, 0700);

    char b_path[PATH_MAX];
    snprintf(b_path, sizeof(b_path), "%s/%s", sub_dir, "b.txt");
    write_text_file(b_path, "Hello B");

    // 5 Test cases
    char* r1 = safe_copy_to_temp(base_dir, "a.txt", NULL);
    printf("Test1: %s\n", r1 ? r1 : "NULL");
    free(r1);

    char* r2 = safe_copy_to_temp(base_dir, "sub/b.txt", NULL);
    printf("Test2: %s\n", r2 ? r2 : "NULL");
    free(r2);

    char* r3 = safe_copy_to_temp(base_dir, "../../etc/passwd", NULL);
    printf("Test3: %s\n", r3 ? r3 : "NULL");
    free(r3);

    char* r4 = safe_copy_to_temp(base_dir, "missing.txt", NULL);
    printf("Test4: %s\n", r4 ? r4 : "NULL");
    free(r4);

    char* r5 = safe_copy_to_temp(a_path, "a.txt", NULL); // base_dir is a file, not a dir
    printf("Test5: %s\n", r5 ? r5 : "NULL");
    free(r5);

    return 0;
}