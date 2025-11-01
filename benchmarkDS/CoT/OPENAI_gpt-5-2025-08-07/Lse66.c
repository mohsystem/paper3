/*
Chain-of-Through process:
1) Problem understanding: Accept PDF bytes, validate, and save securely into ./uploads. Return saved path via out param.
2) Security requirements: Validate PDF magic header, size limit, ignore client filename; create directory safely; exclusive creation; no traversal.
3) Secure coding generation: Implement checks, use mkstemp for exclusive creation.
4) Code review: In-line comments explain mitigations.
5) Secure code output: Final implementation below.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <fcntl.h>
#define PATH_SEP "\\"
#else
#include <unistd.h>
#define PATH_SEP "/"
#endif

#define MAX_SIZE (1 * 1024 * 1024) // 1 MiB

static int ensure_uploads_dir(char* out_dir, size_t out_len) {
    // Resolve to "./uploads"
    const char* base = "uploads";
    if (strlen(base) + 1 > out_len) return -1;
    snprintf(out_dir, out_len, "%s", base);
#ifdef _WIN32
    struct _stat st;
    if (_stat(out_dir, &st) != 0) {
        if (_mkdir(out_dir) != 0) return -1;
    }
#else
    struct stat st;
    if (stat(out_dir, &st) != 0) {
        if (mkdir(out_dir, 0700) != 0) return -1;
    }
#endif
    return 0;
}

static int has_pdf_header(const unsigned char* data, size_t len) {
    if (!data || len < 5) return 0;
    return data[0] == 0x25 && data[1] == 0x50 && data[2] == 0x44 && data[3] == 0x46 && data[4] == 0x2D; // "%PDF-"
}

// Returns 0 on success and writes saved path into out_path; non-zero on error.
int save_pdf(const unsigned char* data, size_t len, const char* original_filename, char* out_path, size_t out_path_len) {
    (void)original_filename; // not trusted, ignored
    if (!data) return EINVAL;
    if (len < 5) return EINVAL;
    if (len > MAX_SIZE) return EFBIG;
    if (!has_pdf_header(data, len)) return EINVAL;

    char uploads[512];
    if (ensure_uploads_dir(uploads, sizeof(uploads)) != 0) return EIO;

    // Create a unique file using mkstemp-like approach.
    // We'll create a temp file "uploads/pdf-XXXXXX.tmp" and then rename to .pdf
    char temp_template[1024];
    snprintf(temp_template, sizeof(temp_template), "%s%s%s", uploads, PATH_SEP, "pdf-XXXXXX");
#ifdef _WIN32
    // Windows lacks mkstemp; emulate using _mktemp_s and _open with _O_CREAT|_O_EXCL
    if (_mktemp_s(temp_template, strlen(temp_template) + 1) != 0) return EIO;
    char temp_path[1060];
    snprintf(temp_path, sizeof(temp_path), "%s.tmp", temp_template);
    int fd = _open(temp_path, _O_CREAT | _O_EXCL | _O_WRONLY | _O_BINARY, _S_IREAD | _S_IWRITE);
    if (fd == -1) return EIO;
    int wrote = _write(fd, data, (unsigned int)len);
    _close(fd);
    if (wrote < 0 || (size_t)wrote != len) {
        remove(temp_path);
        return EIO;
    }
    char final_path[1060];
    snprintf(final_path, sizeof(final_path), "%s.pdf", temp_template);
    if (rename(temp_path, final_path) != 0) {
        remove(temp_path);
        return EIO;
    }
    if (strlen(final_path) + 1 > out_path_len) return ENAMETOOLONG;
    snprintf(out_path, out_path_len, "%s", final_path);
    return 0;
#else
    int fd = -1;
    // mkstemp modifies the template in-place
    fd = mkstemp(temp_template);
    if (fd < 0) return EIO;
    // Write data
    size_t off = 0;
    while (off < len) {
        ssize_t n = write(fd, data + off, len - off);
        if (n <= 0) {
            close(fd);
            remove(temp_template);
            return EIO;
        }
        off += (size_t)n;
    }
    // Close and rename to .pdf
    close(fd);
    char final_path[1060];
    snprintf(final_path, sizeof(final_path), "%s.pdf", temp_template);
    if (rename(temp_template, final_path) != 0) {
        remove(temp_template);
        return EIO;
    }
    if (strlen(final_path) + 1 > out_path_len) return ENAMETOOLONG;
    snprintf(out_path, out_path_len, "%s", final_path);
    return 0;
#endif
}

int main(void) {
    // 1) Valid small PDF
    {
        const char* s = "%PDF-1.4\n%%EOF";
        char out[1060];
        int rc = save_pdf((const unsigned char*)s, strlen(s), "test1.pdf", out, sizeof(out));
        if (rc == 0) printf("Test1 OK: %s\n", out);
        else printf("Test1 FAILED: rc=%d\n", rc);
    }

    // 2) Invalid header
    {
        const char* s = "HELLO";
        char out[1060];
        int rc = save_pdf((const unsigned char*)s, strlen(s), "x.pdf", out, sizeof(out));
        if (rc != 0) printf("Test2 OK (rejected): rc=%d\n", rc);
        else printf("Test2 FAILED: %s\n", out);
    }

    // 3) Oversized
    {
        size_t big_len = MAX_SIZE + 10;
        unsigned char* big = (unsigned char*)malloc(big_len);
        if (!big) {
            printf("Test3 SKIPPED: OOM\n");
        } else {
            memset(big, 0, big_len);
            big[0] = 0x25; big[1] = 0x50; big[2] = 0x44; big[3] = 0x46; big[4] = 0x2D;
            char out[1060];
            int rc = save_pdf(big, big_len, "big.pdf", out, sizeof(out));
            if (rc != 0) printf("Test3 OK (rejected): rc=%d\n", rc);
            else printf("Test3 FAILED: %s\n", out);
            free(big);
        }
    }

    // 4) Path traversal in filename (ignored)
    {
        const char* s = "%PDF-1.7\n%%EOF";
        char out[1060];
        int rc = save_pdf((const unsigned char*)s, strlen(s), "../evil.pdf", out, sizeof(out));
        if (rc == 0) printf("Test4 OK: %s\n", out);
        else printf("Test4 FAILED: rc=%d\n", rc);
    }

    // 5) Too small data
    {
        unsigned char tiny[4] = {0x25, 0x50, 0x44, 0x46}; // "%PDF" missing '-'
        char out[1060];
        int rc = save_pdf(tiny, sizeof(tiny), "tiny.pdf", out, sizeof(out));
        if (rc != 0) printf("Test5 OK (rejected): rc=%d\n", rc);
        else printf("Test5 FAILED: %s\n", out);
    }

    return 0;
}