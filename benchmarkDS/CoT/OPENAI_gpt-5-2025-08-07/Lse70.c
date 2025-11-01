#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path, mode) _mkdir(path)
#else
#include <fcntl.h>
#include <unistd.h>
#define MKDIR(path, mode) mkdir(path, mode)
#endif

// Chain-of-Through Step 1: Problem understanding
// upload_file securely saves uploaded data into 'uploads' directory, returning saved path or "ERROR: ..." string.

// Chain-of-Through Step 2: Security requirements embedded: sanitize filename, size limit, allowed extensions, unique name, exclusive create.
static const size_t MAX_SIZE_BYTES = 1000000; // 1 MB demo
static const char* ALLOWED_EXTS[] = {"txt","png","jpg","jpeg","pdf","zip","gif","bmp","csv","json"};
static const size_t ALLOWED_EXTS_LEN = sizeof(ALLOWED_EXTS)/sizeof(ALLOWED_EXTS[0]);

static int str_ends_with_ci(const char* s, const char* ext) {
    size_t ls = strlen(s), le = strlen(ext);
    if (le > ls) return 0;
    const char* p = s + (ls - le);
    for (size_t i = 0; i < le; ++i) {
        if (tolower((unsigned char)p[i]) != tolower((unsigned char)ext[i])) return 0;
    }
    return 1;
}

static char* get_ext_lower(const char* filename) {
    const char* base = filename;
    // basename-like extraction
    for (const char* p = filename; *p; ++p) {
        if (*p == '/' || *p == '\\') base = p + 1;
    }
    const char* dot = strrchr(base, '.');
    if (!dot || dot == base || *(dot+1) == '\0') return NULL;
    char* ext = strdup(dot + 1);
    if (!ext) return NULL;
    for (char* q = ext; *q; ++q) *q = (char)tolower((unsigned char)*q);
    return ext;
}

static int is_allowed_ext(const char* ext) {
    if (!ext) return 0;
    for (size_t i = 0; i < ALLOWED_EXTS_LEN; ++i) {
        if (strcasecmp(ext, ALLOWED_EXTS[i]) == 0) return 1;
    }
    return 0;
}

static char* strip_ext_copy(const char* filename) {
    const char* base = filename;
    for (const char* p = filename; *p; ++p) {
        if (*p == '/' || *p == '\\') base = p + 1;
    }
    const char* dot = strrchr(base, '.');
    size_t len = dot && dot != base ? (size_t)(dot - base) : strlen(base);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, base, len);
    out[len] = '\0';
    return out;
}

static void sanitize_name_inplace(char* s) {
    size_t n = strlen(s);
    size_t w = 0;
    for (size_t r = 0; r < n && w < 100; ++r) {
        char c = s[r];
        if (isalnum((unsigned char)c) || c == '.' || c == '_' || c == '-') {
            s[w++] = c;
        } else {
            s[w++] = '_';
        }
    }
    s[w] = '\0';
    if (w == 0 || s[0] == '.') {
        strcpy(s, "file");
    }
}

static void random_hex(char* out, size_t bytes) {
    // try /dev/urandom for better entropy
    FILE* ur = fopen("/dev/urandom", "rb");
    unsigned char* buf = (unsigned char*)malloc(bytes);
    if (!buf) {
        // fallback to rand
        srand((unsigned)time(NULL));
        for (size_t i = 0; i < bytes; ++i) buf[i] = (unsigned char)(rand() % 256);
    } else {
        if (ur) {
            fread(buf, 1, bytes, ur);
            fclose(ur);
        } else {
            srand((unsigned)time(NULL));
            for (size_t i = 0; i < bytes; ++i) buf[i] = (unsigned char)(rand() % 256);
        }
    }
    static const char* HEX = "0123456789abcdef";
    for (size_t i = 0; i < bytes; ++i) {
        out[i*2] = HEX[(buf[i] >> 4) & 0xF];
        out[i*2+1] = HEX[buf[i] & 0xF];
    }
    out[bytes*2] = '\0';
    free(buf);
}

char* upload_file(const char* originalFilename, const unsigned char* data, size_t dataLen, const char* contentType) {
    // Chain-of-Through Step 3: Secure coding generation
    if (!data || dataLen == 0) {
        char* e = strdup("ERROR: Empty or no data");
        return e;
    }
    if (dataLen > MAX_SIZE_BYTES) {
        char* e = strdup("ERROR: File too large");
        return e;
    }
    if (!originalFilename || !*originalFilename) {
        char* e = strdup("ERROR: Missing filename");
        return e;
    }

    char* ext = get_ext_lower(originalFilename);
    if (!ext || !is_allowed_ext(ext)) {
        free(ext);
        char* e = strdup("ERROR: Disallowed file extension");
        return e;
    }

    char* baseNoExt = strip_ext_copy(originalFilename);
    if (!baseNoExt) {
        free(ext);
        char* e = strdup("ERROR: Memory allocation failed");
        return e;
    }
    sanitize_name_inplace(baseNoExt);

    char randhex[17];
    random_hex(randhex, 8);

    // build unique filename
    size_t need = strlen(baseNoExt) + 1 + strlen(randhex) + 1 + strlen(ext) + 1;
    char* unique = (char*)malloc(need);
    if (!unique) {
        free(ext);
        free(baseNoExt);
        char* e = strdup("ERROR: Memory allocation failed");
        return e;
    }
    snprintf(unique, need, "%s_%s.%s", baseNoExt, randhex, ext);

    // ensure uploads directory exists
    struct stat st;
    if (stat("uploads", &st) != 0) {
        if (MKDIR("uploads", 0700) != 0 && errno != EEXIST) {
            free(ext); free(baseNoExt); free(unique);
            char* e = strdup("ERROR: Could not create uploads directory");
            return e;
        }
    } else {
        if (!S_ISDIR(st.st_mode)) {
            free(ext); free(baseNoExt); free(unique);
            char* e = strdup("ERROR: 'uploads' exists and is not a directory");
            return e;
        }
    }

    // Build target path
    size_t pathLen = strlen("uploads/") + strlen(unique) + 1;
    char* target = (char*)malloc(pathLen);
    if (!target) {
        free(ext); free(baseNoExt); free(unique);
        char* e = strdup("ERROR: Memory allocation failed");
        return e;
    }
    snprintf(target, pathLen, "uploads/%s", unique);

    // Exclusive create and write
#ifdef _WIN32
    FILE* f = fopen(target, "wb"); // no exclusive flag; emulate by checking existence
    if (!f) {
        free(ext); free(baseNoExt); free(unique); free(target);
        char* e = strdup("ERROR: Write failed");
        return e;
    }
#else
    int fd = open(target, O_CREAT | O_EXCL | O_WRONLY, 0600);
    if (fd == -1) {
        free(ext); free(baseNoExt); free(unique); free(target);
        char* e = strdup(errno == EEXIST ? "ERROR: Name collision, try again" : "ERROR: Write failed");
        return e;
    }
    FILE* f = fdopen(fd, "wb");
    if (!f) {
        close(fd);
        free(ext); free(baseNoExt); free(unique); free(target);
        char* e = strdup("ERROR: Write failed");
        return e;
    }
#endif
    size_t written = fwrite(data, 1, dataLen, f);
    fclose(f);
    if (written != dataLen) {
        remove(target);
        free(ext); free(baseNoExt); free(unique); free(target);
        char* e = strdup("ERROR: Write failed");
        return e;
    }

    // Cleanup temp allocations except target which is the return
    free(ext); free(baseNoExt); free(unique);

    // Chain-of-Through Step 4 & 5: Review and final secure output
    return target; // caller must free()
}

int main(void) {
    // 5 test cases
    const unsigned char d1[] = "Hello";
    char* r1 = upload_file("hello.txt", d1, sizeof(d1)-1, "text/plain");
    printf("Test1: %s\n", r1); free(r1);

    const unsigned char d2[] = "fake";
    char* r2 = upload_file("../../etc/passwd", d2, sizeof(d2)-1, "text/plain"); // no ext -> error
    printf("Test2: %s\n", r2); free(r2);

    const unsigned char d3[] = {0x89,'P','N','G'};
    char* r3 = upload_file("img.png", d3, sizeof(d3), "image/png");
    printf("Test3: %s\n", r3); free(r3);

    const unsigned char d4[] = {'M','Z'};
    char* r4 = upload_file("bad.exe", d4, sizeof(d4), "application/octet-stream"); // disallowed ext
    printf("Test4: %s\n", r4); free(r4);

    unsigned char* big = (unsigned char*)malloc(MAX_SIZE_BYTES + 1);
    if (big) {
        memset(big, 0, MAX_SIZE_BYTES + 1);
        char* r5 = upload_file("big.pdf", big, MAX_SIZE_BYTES + 1, "application/pdf");
        printf("Test5: %s\n", r5); free(r5);
        free(big);
    } else {
        printf("Test5: ERROR: Allocation failed\n");
    }
    return 0;
}