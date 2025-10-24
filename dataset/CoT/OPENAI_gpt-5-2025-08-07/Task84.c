/* Step 1-5: Secure session ID generator in C */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef _WIN32
  #include <windows.h>
  #include <bcrypt.h>
  #pragma comment(lib, "Bcrypt.lib")
#else
  #include <errno.h>
  #include <fcntl.h>
  #include <unistd.h>
#endif

#define MIN_BYTES 16
#define MAX_BYTES 1024
#define MAX_ATTEMPTS 1000

static int secure_random_bytes(uint8_t* buf, size_t len) {
#ifdef _WIN32
    if (BCryptGenRandom(NULL, buf, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG) != 0) {
        return -1;
    }
    return 0;
#else
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return -1;
    size_t total = 0;
    while (total < len) {
        ssize_t r = read(fd, buf + total, len - total);
        if (r < 0) { close(fd); return -1; }
        if (r == 0) { close(fd); return -1; }
        total += (size_t)r;
    }
    close(fd);
    return 0;
#endif
}

static char* base64url_encode(const uint8_t* data, size_t len) {
    static const char tbl[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    size_t out_cap = ((len + 2) / 3) * 4;
    char* out = (char*)malloc(out_cap + 1);
    if (!out) return NULL;
    size_t o = 0;
    size_t i = 0;
    while (i + 3 <= len) {
        unsigned int v = (data[i] << 16) | (data[i+1] << 8) | data[i+2];
        out[o++] = tbl[(v >> 18) & 0x3F];
        out[o++] = tbl[(v >> 12) & 0x3F];
        out[o++] = tbl[(v >> 6) & 0x3F];
        out[o++] = tbl[v & 0x3F];
        i += 3;
    }
    size_t rem = len - i;
    if (rem == 1) {
        unsigned int v = (data[i] << 16);
        out[o++] = tbl[(v >> 18) & 0x3F];
        out[o++] = tbl[(v >> 12) & 0x3F];
        /* omit padding */
    } else if (rem == 2) {
        unsigned int v = (data[i] << 16) | (data[i+1] << 8);
        out[o++] = tbl[(v >> 18) & 0x3F];
        out[o++] = tbl[(v >> 12) & 0x3F];
        out[o++] = tbl[(v >> 6) & 0x3F];
        /* omit padding */
    }
    out[o] = '\0';
    return out;
}

/* Simple dynamic list to track issued IDs for uniqueness */
static char** issued = NULL;
static size_t issued_count = 0;
static size_t issued_cap = 0;

static int is_unique_and_add(const char* id) {
    for (size_t i = 0; i < issued_count; ++i) {
        if (strcmp(issued[i], id) == 0) return 0;
    }
    if (issued_count == issued_cap) {
        size_t new_cap = issued_cap == 0 ? 16 : issued_cap * 2;
        char** tmp = (char**)realloc(issued, new_cap * sizeof(char*));
        if (!tmp) return 0;
        issued = tmp;
        issued_cap = new_cap;
    }
    size_t len = strlen(id);
    char* copy = (char*)malloc(len + 1);
    if (!copy) return 0;
    memcpy(copy, id, len + 1);
    issued[issued_count++] = copy;
    return 1;
}

char* generate_session_id(size_t num_bytes) {
    if (num_bytes < MIN_BYTES || num_bytes > MAX_BYTES) {
        return NULL;
    }
    uint8_t* buf = (uint8_t*)malloc(num_bytes);
    if (!buf) return NULL;

    for (int attempt = 0; attempt < MAX_ATTEMPTS; ++attempt) {
        if (secure_random_bytes(buf, num_bytes) != 0) {
            free(buf);
            return NULL;
        }
        char* id = base64url_encode(buf, num_bytes);
        if (!id) {
            free(buf);
            return NULL;
        }
        if (is_unique_and_add(id)) {
            free(buf);
            return id;
        }
        free(id);
    }
    free(buf);
    return NULL;
}

int main(void) {
    /* 5 test cases */
    char* a = generate_session_id(16);
    char* b = generate_session_id(24);
    char* c = generate_session_id(32);
    char* d = generate_session_id(48);
    char* e = generate_session_id(64);
    if (a) { printf("%s\n", a); }
    if (b) { printf("%s\n", b); }
    if (c) { printf("%s\n", c); }
    if (d) { printf("%s\n", d); }
    if (e) { printf("%s\n", e); }

    /* Free allocated IDs (optional cleanup) */
    free(a); free(b); free(c); free(d); free(e);
    for (size_t i = 0; i < issued_count; ++i) free(issued[i]);
    free(issued);
    return 0;
}