#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if defined(_WIN32)
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#include <direct.h>
#include <io.h>
#define PATH_SEP "\\"
#else
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#define PATH_SEP "/"
#endif

#define MAX_SIZE_BYTES (5u * 1024u * 1024u)
static const char *UPLOAD_DIR = "uploads";

static int is_allowed_ext(const char *ext) {
    const char *allowed[] = {"txt","pdf","png","jpg","jpeg","gif"};
    for (size_t i = 0; i < sizeof(allowed)/sizeof(allowed[0]); i++) {
        if (strcmp(ext, allowed[i]) == 0) return 1;
    }
    return 0;
}

static int contains_traversal(const char *name) {
    if (strstr(name, "..") != NULL) return 1;
    if (strchr(name, '/') != NULL) return 1;
    if (strchr(name, '\\') != NULL) return 1;
    return 0;
}

static char *sanitize_filename(const char *name, char **out_ext) {
    if (contains_traversal(name)) {
        return NULL;
    }
    // keep only safe chars
    size_t n = strlen(name);
    char *buf = (char*)malloc(n + 1);
    if (!buf) return NULL;
    size_t j = 0;
    for (size_t i = 0; i < n; i++) {
        char c = name[i];
        if ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            c == '.' || c == '_' || c == '-') {
            buf[j++] = c;
        }
    }
    buf[j] = '\0';
    // remove leading dots
    while (buf[0] == '.') memmove(buf, buf + 1, strlen(buf));
    // must have extension
    char *dot = strrchr(buf, '.');
    if (!dot || dot == buf || *(dot + 1) == '\0') {
        free(buf);
        return NULL;
    }
    // extract and validate extension
    char *ext = dot + 1;
    // to lower (ASCII)
    for (char *p = ext; *p; ++p) {
        if (*p >= 'A' && *p <= 'Z') *p = (char)(*p - 'A' + 'a');
    }
    if (strlen(ext) == 0 || strlen(ext) > 10) {
        free(buf);
        return NULL;
    }
    if (!is_allowed_ext(ext)) {
        free(buf);
        return NULL;
    }
    if (strlen(buf) > 255) {
        // keep last 255 chars
        char *tmp = (char*)malloc(256);
        if (!tmp) {
            free(buf);
            return NULL;
        }
        size_t len = strlen(buf);
        memcpy(tmp, buf + (len - 255), 255);
        tmp[255] = '\0';
        free(buf);
        buf = tmp;
        dot = strrchr(buf, '.');
        ext = dot + 1;
    }
    *out_ext = ext;
    return buf;
}

static int ensure_upload_dir(void) {
#if defined(_WIN32)
    if (_mkdir(UPLOAD_DIR) != 0) {
        if (errno != EEXIST) return -1;
    }
    // Windows ACLs not set here; default ACLs apply.
    return 0;
#else
    if (mkdir(UPLOAD_DIR, 0700) != 0) {
        if (errno != EEXIST) return -1;
    }
    chmod(UPLOAD_DIR, 0700);
    return 0;
#endif
}

static int secure_random_bytes(unsigned char *out, size_t len) {
#if defined(_WIN32)
    if (BCryptGenRandom(NULL, out, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG) != 0) return -1;
    return 0;
#else
#ifdef __linux__
    // try getrandom syscall via getrandom() if available
    // Fallback to /dev/urandom if not available
#endif
    FILE *f = fopen("/dev/urandom", "rb");
    if (!f) return -1;
    size_t r = fread(out, 1, len, f);
    fclose(f);
    if (r != len) return -1;
    return 0;
#endif
}

static char *hex_encode(const unsigned char *in, size_t len) {
    static const char *hex = "0123456789abcdef";
    char *out = (char*)malloc(len * 2 + 1);
    if (!out) return NULL;
    for (size_t i = 0; i < len; i++) {
        out[i*2] = hex[(in[i] >> 4) & 0xF];
        out[i*2 + 1] = hex[in[i] & 0xF];
    }
    out[len*2] = '\0';
    return out;
}

char *upload_file(const char *original_name, const unsigned char *content, size_t content_len) {
    if (!original_name || !content) {
        return NULL;
    }
    if (content_len == 0 || content_len > MAX_SIZE_BYTES) {
        return NULL;
    }

    char *ext = NULL;
    char *san = sanitize_filename(original_name, &ext);
    if (!san) {
        return NULL;
    }

    if (ensure_upload_dir() != 0) {
        free(san);
        return NULL;
    }

    unsigned char rnd[16];
    if (secure_random_bytes(rnd, sizeof(rnd)) != 0) {
        free(san);
        return NULL;
    }
    char *hex = hex_encode(rnd, sizeof(rnd));
    if (!hex) {
        free(san);
        return NULL;
    }

    // build destination path: uploads/<hex>.<ext>
    size_t path_len = strlen(UPLOAD_DIR) + 1 + strlen(hex) + 1 + strlen(ext) + 1 + 4;
    char *dest = (char*)malloc(path_len);
    if (!dest) {
        free(san);
        free(hex);
        return NULL;
    }
    snprintf(dest, path_len, "%s" PATH_SEP "%s.%s", UPLOAD_DIR, hex, ext);

    // Write file
#if defined(_WIN32)
    // Create file only if not exists
    HANDLE hFile = CreateFileA(dest, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        free(san); free(hex); free(dest);
        return NULL;
    }
    DWORD written = 0;
    BOOL ok = WriteFile(hFile, content, (DWORD)content_len, &written, NULL);
    CloseHandle(hFile);
    if (!ok || written != content_len) {
        free(san); free(hex); free(dest);
        return NULL;
    }
#else
    int fd = open(dest, O_WRONLY | O_CREAT | O_EXCL, 0600);
    if (fd < 0) {
        free(san); free(hex); free(dest);
        return NULL;
    }
    size_t total = 0;
    while (total < content_len) {
        ssize_t w = write(fd, content + total, content_len - total);
        if (w <= 0) {
            close(fd);
            free(san); free(hex); free(dest);
            return NULL;
        }
        total += (size_t)w;
    }
    close(fd);
#endif

    free(san);
    free(hex);
    // Return allocated path; caller must free
    return dest;
}

int main(void) {
    // 5 test cases
    const char *names[5] = {
        "hello.txt",
        "image.jpg",
        "report.pdf",
        "evil/../secret.txt",
        "malware.exe"
    };
    unsigned char data0[] = "Hello, World!";
    unsigned char data1[] = {0xFF,0xD8,0xFF,0xD9};
    unsigned char data2[] = {'%', 'P','D','F','-','1','.','4','\n','%', 0xE2, 0xE3, 0xCF, 0xD3, '\n'};
    unsigned char data3[] = "top secret";
    unsigned char data4[] = {'M','Z',0,0};

    const unsigned char *datas[5] = {data0, data1, data2, data3, data4};
    size_t sizes[5] = {sizeof(data0)-1, sizeof(data1), sizeof(data2), sizeof(data3)-1, sizeof(data4)};

    for (int i = 0; i < 5; i++) {
        char *path = upload_file(names[i], datas[i], sizes[i]);
        if (path) {
            printf("Saved: %s\n", path);
            free(path);
        } else {
            printf("Failed: %s\n", names[i]);
        }
    }
    return 0;
}