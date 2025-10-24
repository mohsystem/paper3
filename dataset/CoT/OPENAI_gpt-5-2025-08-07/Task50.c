#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#include <io.h>
#include <direct.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <windows.h>
#define MKDIR(path,mode) _mkdir(path)
#define OPEN _open
#define WRITE _write
#define CLOSE _close
#else
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#define MKDIR(path,mode) mkdir(path, mode)
#define OPEN open
#define WRITE write
#define CLOSE close
#endif

// Chain-of-Through process:
// 1) Understand: Store uploaded file securely and return confirmation.
// 2) Security: Validate file name, prevent traversal, restrict size and extension, unique file naming, strict permissions.
// 3) Implement: Safe sanitization, controlled directory creation, exclusive create.
// 4) Review: Check all error paths, avoid buffer overflows, handle memory carefully.
// 5) Output: Final hardened C implementation with tests.

#define MAX_SIZE (10 * 1024 * 1024) // 10 MB
static const char* UPLOAD_DIR = "uploads";
static const char* ALLOWED_EXT[] = {"txt","pdf","png","jpg","jpeg","gif","bin","dat"};
static const size_t ALLOWED_EXT_COUNT = sizeof(ALLOWED_EXT)/sizeof(ALLOWED_EXT[0]);

static int is_allowed_ext(const char* ext) {
    if (!ext || !*ext) return 0;
    for (size_t i = 0; i < ALLOWED_EXT_COUNT; ++i) {
        if (strcmp(ext, ALLOWED_EXT[i]) == 0) return 1;
    }
    return 0;
}

static void tolower_str(char* s) {
    for (; *s; ++s) *s = (char)tolower((unsigned char)*s);
}

static char* sanitize_filename(const char* name) {
    // Returns malloc'd sanitized filename or NULL if invalid
    if (!name) return NULL;
    // Extract basename: find last slash or backslash
    const char* base = name;
    for (const char* p = name; *p; ++p) {
        if (*p == '/' || *p == '\\') base = p + 1;
    }
    if (strstr(base, "..")) return NULL;
    size_t n = strlen(base);
    if (n == 0 || n > 100) return NULL;
    char* out = (char*)malloc(n + 1);
    if (!out) return NULL;
    for (size_t i = 0; i < n; ++i) {
        char c = base[i];
        if (isalnum((unsigned char)c) || c=='.' || c=='_' || c=='-') out[i] = c;
        else out[i] = '_';
    }
    out[n] = '\0';
    if (out[0] == '\0') { free(out); return NULL; }
    return out;
}

static char* get_extension_lower(const char* name) {
    const char* dot = strrchr(name, '.');
    if (!dot || *(dot+1) == '\0') return NULL;
    char* ext = strdup(dot + 1);
    if (!ext) return NULL;
    tolower_str(ext);
    return ext;
}

static int ensure_upload_dir(void) {
#ifdef _WIN32
    struct _stat st;
    if (_stat(UPLOAD_DIR, &st) == 0 && (st.st_mode & _S_IFDIR)) return 1;
    if (MKDIR(UPLOAD_DIR, 0700) == 0) return 1;
    // If failed but exists, consider success
    if (_stat(UPLOAD_DIR, &st) == 0 && (st.st_mode & _S_IFDIR)) return 1;
    return 0;
#else
    struct stat st;
    if (stat(UPLOAD_DIR, &st) == 0 && S_ISDIR(st.st_mode)) return 1;
    if (MKDIR(UPLOAD_DIR, 0700) == 0) return 1;
    if (stat(UPLOAD_DIR, &st) == 0 && S_ISDIR(st.st_mode)) return 1;
    return 0;
#endif
}

static void random_hex(char* buf, size_t len) {
    // Fill buf with len hex chars (len must be even)
    size_t need = len / 2;
    unsigned char tmp[32];
    size_t generated = 0;
    FILE* ur = fopen("/dev/urandom", "rb");
    if (ur) {
        while (generated < need) {
            size_t toread = (need - generated > sizeof(tmp)) ? sizeof(tmp) : (need - generated);
            size_t r = fread(tmp, 1, toread, ur);
            if (r == 0) break;
            for (size_t i = 0; i < r; ++i) {
                sprintf(buf + (generated + i)*2, "%02x", tmp[i]);
            }
            generated += r;
        }
        fclose(ur);
    }
    if (generated < need) {
        // Fallback to rand()
        #ifdef _WIN32
        unsigned int seed = (unsigned int)GetTickCount();
        srand(seed);
        #else
        srand((unsigned int)time(NULL));
        #endif
        for (; generated < need; ++generated) {
            unsigned char b = (unsigned char)(rand() & 0xFF);
            sprintf(buf + generated*2, "%02x", b);
        }
    }
    buf[len] = '\0';
}

char* upload_file(const char* original_file_name, const unsigned char* content, size_t content_len) {
    if (!original_file_name || !*original_file_name) {
        char* msg = strdup("ERROR: Invalid file name.");
        return msg ? msg : NULL;
    }
    if (!content && content_len > 0) {
        char* msg = strdup("ERROR: No content provided.");
        return msg ? msg : NULL;
    }
    if (content_len > MAX_SIZE) {
        char buf[128];
        snprintf(buf, sizeof(buf), "ERROR: File too large. Max size is %d bytes.", (int)MAX_SIZE);
        return strdup(buf);
    }
    if (!ensure_upload_dir()) {
        return strdup("ERROR: Could not create upload directory.");
    }

    char* safe = sanitize_filename(original_file_name);
    if (!safe) {
        return strdup("ERROR: Invalid file name.");
    }

    char* ext = get_extension_lower(safe);
    if (!ext || !is_allowed_ext(ext)) {
        free(safe);
        if (ext) free(ext);
        return strdup("ERROR: Disallowed or missing file extension.");
    }

    // Separate base and ext
    char* dot = strrchr(safe, '.');
    size_t base_len = (size_t)(dot - safe);
    if (base_len == 0) base_len = 4; // "file"
    if (base_len > 64) base_len = 64;

    char base[65];
    if (dot && (dot - safe) > 0) {
        size_t copy_len = base_len;
        if ((size_t)(dot - safe) < copy_len) copy_len = (size_t)(dot - safe);
        strncpy(base, safe, copy_len);
        base[copy_len] = '\0';
    } else {
        strncpy(base, "file", sizeof(base));
        base[sizeof(base)-1] = '\0';
    }

    char suffix[33]; // 16 bytes hex -> 32 chars
    random_hex(suffix, 32);

    // Build final file name and path
    char final_name[256];
    snprintf(final_name, sizeof(final_name), "%s-%s.%s", base, suffix, ext);
    char path[512];
    snprintf(path, sizeof(path), "%s/%s", UPLOAD_DIR, final_name);

    // Write with O_CREAT|O_EXCL to avoid overwriting
#ifdef _WIN32
    int fd = OPEN(path, _O_WRONLY | _O_CREAT | _O_EXCL | _O_BINARY, _S_IREAD | _S_IWRITE);
#else
    int fd = OPEN(path, O_WRONLY | O_CREAT | O_EXCL, 0600);
#endif
    if (fd < 0) {
        free(safe);
        free(ext);
        return strdup("ERROR: Failed to store file.");
    }

    size_t total = 0;
    while (total < content_len) {
        int w = WRITE(fd, content + total, (unsigned int)(content_len - total));
        if (w <= 0) {
            CLOSE(fd);
            remove(path);
            free(safe);
            free(ext);
            return strdup("ERROR: Failed to write file.");
        }
        total += (size_t)w;
    }
    CLOSE(fd);

    char* msg = (char*)malloc(128 + strlen(final_name));
    if (!msg) {
        free(safe);
        free(ext);
        return NULL;
    }
    snprintf(msg, 128 + strlen(final_name), "OK: Stored as %s/%s (size %zu bytes).", UPLOAD_DIR, final_name, content_len);

    free(safe);
    free(ext);
    return msg;
}

int main(void) {
    // 5 test cases
    {
        const unsigned char data[] = "Hello, world!";
        char* res = upload_file("hello.txt", data, sizeof(data)-1);
        if (res) { printf("%s\n", res); free(res); }
    }
    {
        const unsigned char data[] = "malicious";
        char* res = upload_file("../evil.txt", data, sizeof(data)-1);
        if (res) { printf("%s\n", res); free(res); }
    }
    {
        const unsigned char data[] = {0x25,0x50,0x44,0x46}; // %PDF
        char* res = upload_file("report.pdf", data, sizeof(data));
        if (res) { printf("%s\n", res); free(res); }
    }
    {
        size_t big_len = MAX_SIZE + 1;
        unsigned char* big = (unsigned char*)malloc(big_len);
        if (big) {
            memset(big, 0, big_len);
            char* res = upload_file("image.png", big, big_len);
            if (res) { printf("%s\n", res); free(res); }
            free(big);
        } else {
            printf("ERROR: Allocation failed for big test.\n");
        }
    }
    {
        const unsigned char data[] = "content";
        char* res = upload_file("bad?.txt", data, sizeof(data)-1);
        if (res) { printf("%s\n", res); free(res); }
    }
    return 0;
}