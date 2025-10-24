#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

char* read_file_and_print(const char* path) {
    const size_t MAX_BYTES = 10u * 1024u * 1024u; // 10 MB

    if (path == NULL || *path == '\0') {
        fprintf(stderr, "Error: file path is null or empty.\n");
        return NULL;
    }

    struct stat sb;
    if (lstat(path, &sb) != 0) {
        fprintf(stderr, "Error: cannot access file metadata.\n");
        return NULL;
    }
    if (!S_ISREG(sb.st_mode)) {
        fprintf(stderr, "Error: not a regular file.\n");
        return NULL;
    }

    int flags = O_RDONLY;
#ifdef O_CLOEXEC
    flags |= O_CLOEXEC;
#endif
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif

    int fd = open(path, flags);
    if (fd < 0) {
        fprintf(stderr, "Error: cannot open file for reading.\n");
        return NULL;
    }

    char *out = NULL;
    size_t cap = 0, len = 0;
    char buf[8192];
    ssize_t r;
    int error = 0;

    while ((r = read(fd, buf, sizeof buf)) > 0) {
        if ((size_t)r > MAX_BYTES || len + (size_t)r > MAX_BYTES) {
            fprintf(stderr, "Error: file exceeds maximum allowed size of 10 MB.\n");
            error = 1;
            break;
        }
        if (len + (size_t)r + 1 > cap) {
            size_t newcap = cap ? cap * 2 : 16384;
            while (newcap < len + (size_t)r + 1) {
                newcap *= 2;
            }
            if (newcap > MAX_BYTES + 1) {
                newcap = MAX_BYTES + 1;
            }
            char *tmp = (char*)realloc(out, newcap);
            if (!tmp) {
                fprintf(stderr, "Error: out of memory.\n");
                error = 1;
                break;
            }
            out = tmp;
            cap = newcap;
        }
        memcpy(out + len, buf, (size_t)r);
        len += (size_t)r;
    }

    if (r < 0) {
        fprintf(stderr, "Error: I/O error while reading the file.\n");
        error = 1;
    }

    close(fd);

    if (error) {
        free(out);
        return NULL;
    }

    if (!out) {
        out = (char*)malloc(1);
        if (!out) {
            fprintf(stderr, "Error: out of memory.\n");
            return NULL;
        }
        len = 0;
        cap = 1;
    }
    out[len] = '\0';

    if (len > 0) {
        if (fwrite(out, 1, len, stdout) != len) {
            fprintf(stderr, "Error: failed to write to stdout.\n");
            // continue; still return out
        }
        fflush(stdout);
    }
    return out;
}

static char* create_temp_file_with_content(const char* content) {
    char tmpl[] = "/tmp/task36_XXXXXX";
    int fd = mkstemp(tmpl);
    if (fd < 0) {
        return NULL;
    }
    size_t len = content ? strlen(content) : 0;
    if (len > 0) {
        ssize_t w = write(fd, content, len);
        (void)w; // ignore short write in this test harness
    }
    close(fd);
    // Return a heap-allocated copy of the path
    char *path = (char*)malloc(strlen(tmpl) + 1);
    if (!path) return NULL;
    strcpy(path, tmpl);
    return path;
}

int main(void) {
    // Create 4 temp files
    char *p1 = create_temp_file_with_content("Hello, Task36!\n");
    char *p2 = create_temp_file_with_content(""); // empty
    char *p3 = create_temp_file_with_content("こんにちは世界\nEmoji: \xF0\x9F\x98\x80\n"); // UTF-8 bytes for 😀
    char *p4 = create_temp_file_with_content("Line1\nLine2\nLine3\n");

    // Non-existent path
    char non_existent[256];
    snprintf(non_existent, sizeof(non_existent), "/tmp/does_not_exist_%ld.txt", (long)getpid());

    // Run 5 test cases
    if (p1) { char* s = read_file_and_print(p1); free(s); }
    if (p2) { char* s = read_file_and_print(p2); free(s); }
    if (p3) { char* s = read_file_and_print(p3); free(s); }
    if (p4) { char* s = read_file_and_print(p4); free(s); }
    { char* s = read_file_and_print(non_existent); if (s) free(s); }

    // Cleanup temp files
    if (p1) { unlink(p1); free(p1); }
    if (p2) { unlink(p2); free(p2); }
    if (p3) { unlink(p3); free(p3); }
    if (p4) { unlink(p4); free(p4); }

    return 0;
}