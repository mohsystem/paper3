#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if defined(__unix__) || defined(__APPLE__)
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#endif

#define MAX_PASSWORD_LENGTH 1024

static void secure_memzero(void *v, size_t n) {
#if defined(__STDC_LIB_EXT1__)
    memset_s(v, n, 0, n);
#else
    volatile unsigned char *p = (volatile unsigned char *)v;
    while (n--) *p++ = 0;
#endif
}

int compare_password_from_file(const char* file_path, const char* user_password) {
    if (file_path == NULL || user_password == NULL) return 0;

    FILE *fp = fopen(file_path, "rb");
    if (!fp) return 0;

    char buf[MAX_PASSWORD_LENGTH + 1];
    size_t len = 0;
    int c;
    int too_long = 0;

    while ((c = fgetc(fp)) != EOF) {
        if (c == '\n') break;
        if (c == '\r') {
            int next = fgetc(fp);
            if (next != '\n' && next != EOF) {
                ungetc(next, fp);
            }
            break;
        }
        if (len >= MAX_PASSWORD_LENGTH) {
            too_long = 1;
            // continue to consume line safely
        } else {
            buf[len++] = (char)c;
        }
    }
    fclose(fp);

    if (too_long) {
        secure_memzero(buf, sizeof(buf));
        return 0;
    }

    buf[len] = '\0';

    const char* up = user_password;
    size_t lenA = len;
    size_t lenB = strlen(up);
    size_t maxLen = lenA > lenB ? lenA : lenB;

    unsigned int diff = (unsigned int)(lenA ^ lenB);
    for (size_t i = 0; i < maxLen; i++) {
        unsigned char ca = (i < lenA) ? (unsigned char)buf[i] : 0;
        unsigned char cb = (i < lenB) ? (unsigned char)up[i] : 0;
        diff |= (unsigned int)(ca ^ cb);
    }

    secure_memzero(buf, sizeof(buf));
    return diff == 0 ? 1 : 0;
}

#if defined(__unix__) || defined(__APPLE__)
static char* create_secure_temp_file(const char* content) {
    char tmpl[] = "/tmp/task89_XXXXXX.pwd";
    int fd = mkstemp(tmpl);
    if (fd == -1) return NULL;
    fchmod(fd, S_IRUSR | S_IWUSR);
    size_t len = strlen(content);
    const char* p = content;
    while (len > 0) {
        ssize_t w = write(fd, p, len);
        if (w <= 0) {
            close(fd);
            unlink(tmpl);
            return NULL;
        }
        p += (size_t)w;
        len -= (size_t)w;
    }
    close(fd);
    char* path = (char*)malloc(strlen(tmpl) + 1);
    if (!path) return NULL;
    strcpy(path, tmpl);
    return path;
}
#else
static char* create_secure_temp_file(const char* content) {
    const char* base = "task89_fallback.pwd";
    FILE* f = fopen(base, "wb");
    if (!f) return NULL;
    fwrite(content, 1, strlen(content), f);
    fclose(f);
    char* path = (char*)malloc(strlen(base) + 1);
    if (!path) return NULL;
    strcpy(path, base);
    return path;
}
#endif

int main(void) {
    char* f1 = create_secure_temp_file("secret123");
    char* f2 = create_secure_temp_file("secret123");
    char* f3 = create_secure_temp_file("pass with spaces");
    char* f4 = create_secure_temp_file("");
    char* f5 = create_secure_temp_file("short");

    printf("%d\n", compare_password_from_file(f1, "secret123"));      // 1
    printf("%d\n", compare_password_from_file(f2, "secret124"));      // 0
    printf("%d\n", compare_password_from_file(f3, "pass with spaces"));// 1
    printf("%d\n", compare_password_from_file(f4, ""));               // 1
    printf("%d\n", compare_password_from_file(f5, "shorter"));        // 0

#if defined(__unix__) || defined(__APPLE__)
    if (f1) { unlink(f1); free(f1); }
    if (f2) { unlink(f2); free(f2); }
    if (f3) { unlink(f3); free(f3); }
    if (f4) { unlink(f4); free(f4); }
    if (f5) { unlink(f5); free(f5); }
#else
    if (f1) { remove(f1); free(f1); }
    if (f2) { remove(f2); free(f2); }
    if (f3) { remove(f3); free(f3); }
    if (f4) { remove(f4); free(f4); }
    if (f5) { remove(f5); free(f5); }
#endif

    return 0;
}