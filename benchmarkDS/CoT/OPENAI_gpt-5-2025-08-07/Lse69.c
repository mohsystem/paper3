/* Steps (Chain-of-Through):
 * 1) Problem understanding: upload_image() reads an image, base64-encodes it, writes encoded data to a file.
 * 2) Security requirements: Validate file size, extension, signature; constrain output to CWD.
 * 3) Secure coding generation: Safe I/O, bounds checks, error handling.
 * 4) Code review: Prevent path traversal, oversized input, incorrect use of memory.
 * 5) Secure code output: Final implementation with tests.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <errno.h>

#if defined(_WIN32)
  #include <direct.h>
  #include <io.h>
  #define MKDIR(p) _mkdir(p)
  #define PATH_SEP '\\'
  #include <windows.h>
#else
  #include <unistd.h>
  #include <limits.h>
  #define MKDIR(p) mkdir(p, 0700)
  #define PATH_SEP '/'
#endif

#define MAX_SIZE (10UL * 1024UL * 1024UL) /* 10 MB */

static int has_allowed_extension(const char* name) {
    const char* dot = strrchr(name, '.');
    if (!dot) return 0;
    char ext[16];
    size_t n = strlen(dot);
    if (n >= sizeof(ext)) return 0;
    for (size_t i = 0; i < n; i++) ext[i] = (char)tolower((unsigned char)dot[i]);
    ext[n] = '\0';
    const char* allowed[] = {".png",".jpg",".jpeg",".gif",".bmp",".webp"};
    for (size_t i = 0; i < sizeof(allowed)/sizeof(allowed[0]); i++) {
        if (strcmp(ext, allowed[i]) == 0) return 1;
    }
    return 0;
}

static int allowed_signature(const unsigned char* head, size_t len) {
    if (len < 12) return 0;
    /* PNG */
    const unsigned char png[] = {0x89,'P','N','G',0x0D,0x0A,0x1A,0x0A};
    int is_png = 1;
    for (int i = 0; i < 8; i++) if (head[i] != png[i]) { is_png = 0; break; }
    if (is_png) return 1;
    /* JPEG */
    if (head[0]==0xFF && head[1]==0xD8 && head[2]==0xFF) return 1;
    /* GIF */
    if ((memcmp(head, "GIF87a", 6) == 0) || (memcmp(head, "GIF89a", 6) == 0)) return 1;
    /* BMP */
    if (head[0]=='B' && head[1]=='M') return 1;
    /* WEBP: "RIFF" ... "WEBP" */
    if (memcmp(head, "RIFF", 4) == 0 && memcmp(head+8, "WEBP", 4) == 0) return 1;
    return 0;
}

static char* base64_encode(const unsigned char* data, size_t len) {
    static const char* b64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t out_len = 4 * ((len + 2) / 3);
    char* out = (char*)malloc(out_len + 1);
    if (!out) return NULL;
    size_t i = 0, j = 0;
    while (i + 3 <= len) {
        unsigned int n = (data[i] << 16) | (data[i+1] << 8) | (data[i+2]);
        out[j++] = b64[(n >> 18) & 63];
        out[j++] = b64[(n >> 12) & 63];
        out[j++] = b64[(n >> 6) & 63];
        out[j++] = b64[n & 63];
        i += 3;
    }
    if (i < len) {
        unsigned int n = data[i] << 16;
        out[j++] = b64[(n >> 18) & 63];
        if (i + 1 < len) {
            n |= data[i+1] << 8;
            out[j++] = b64[(n >> 12) & 63];
            out[j++] = b64[(n >> 6) & 63];
            out[j++] = '=';
        } else {
            out[j++] = b64[(n >> 12) & 63];
            out[j++] = '=';
            out[j++] = '=';
        }
    }
    out[j] = '\0';
    return out;
}

/* Base64 decode for test image creation */
static unsigned char* base64_decode(const char* s, size_t* out_len) {
    int T[256];
    for (int i = 0; i < 256; ++i) T[i] = -1;
    const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    for (int i = 0; chars[i]; ++i) T[(unsigned char)chars[i]] = i;
    T[(unsigned char)'='] = -2;

    size_t cap = strlen(s) * 3 / 4 + 4;
    unsigned char* out = (unsigned char*)malloc(cap);
    if (!out) return NULL;
    int val = 0, valb = -8;
    size_t j = 0;
    for (const unsigned char* p = (const unsigned char*)s; *p; ++p) {
        int t = T[*p];
        if (t == -1) continue;
        if (t == -2) break;
        val = (val << 6) | t;
        valb += 6;
        if (valb >= 0) {
            if (j >= cap) {
                cap *= 2;
                unsigned char* tmp = (unsigned char*)realloc(out, cap);
                if (!tmp) { free(out); return NULL; }
                out = tmp;
            }
            out[j++] = (unsigned char)((val >> valb) & 0xFF);
            valb -= 8;
        }
    }
    *out_len = j;
    return out;
}

static int ensure_dirs(const char* path) {
    // Create parent directories (very basic, split by separator)
    char* dup = strdup(path);
    if (!dup) return -1;
    size_t len = strlen(dup);
    for (size_t i = 0; i < len; ++i) {
        if (dup[i] == '/' || dup[i] == '\\') {
            char c = dup[i];
            dup[i] = '\0';
            if (dup[0] != '\0') {
                MKDIR(dup); /* ignore errors if exist */
            }
            dup[i] = c;
        }
    }
    free(dup);
    return 0;
}

static int secure_output_within_cwd(const char* output_path) {
    char cwd_buf[4096];
#if defined(_WIN32)
    if (!_getcwd(cwd_buf, sizeof(cwd_buf))) return 0;
    char out_buf[4096];
    if (!_fullpath(out_buf, output_path, sizeof(out_buf))) return 0;
    /* Case-insensitive compare start */
    size_t cwd_len = strlen(cwd_buf);
    for (size_t i = 0; i < cwd_len; ++i) {
        char a = (char)tolower((unsigned char)cwd_buf[i]);
        char b = (char)tolower((unsigned char)out_buf[i]);
        if (a != b) return 0;
    }
#else
    if (!getcwd(cwd_buf, sizeof(cwd_buf))) return 0;
    char out_buf[4096];
    if (!realpath(output_path, out_buf)) {
        /* Try to resolve parent directory only */
        char tmp[4096];
        strncpy(tmp, output_path, sizeof(tmp)-1); tmp[sizeof(tmp)-1] = '\0';
        char* last_sep = strrchr(tmp, PATH_SEP);
        if (last_sep) {
            *last_sep = '\0';
            char par_buf[4096];
            if (!realpath(tmp, par_buf)) return 0;
            /* ok parent is inside cwd? */
            if (strncmp(par_buf, cwd_buf, strlen(cwd_buf)) != 0) return 0;
            return 1;
        }
        return 0;
    }
    if (strncmp(out_buf, cwd_buf, strlen(cwd_buf)) != 0) return 0;
#endif
    return 1;
}

char* upload_image(const char* input_path, const char* output_path) {
    if (!input_path || !output_path) { errno = EINVAL; return NULL; }

    struct stat st;
    if (stat(input_path, &st) != 0 || !S_ISREG(st.st_mode)) { errno = EINVAL; return NULL; }
    if (st.st_size <= 0 || (unsigned long)st.st_size > MAX_SIZE) { errno = EINVAL; return NULL; }

    const char* base = strrchr(input_path, PATH_SEP);
    base = base ? base + 1 : input_path;
    if (!has_allowed_extension(base)) { errno = EINVAL; return NULL; }

    FILE* f = fopen(input_path, "rb");
    if (!f) return NULL;
    unsigned char head[12];
    size_t r = fread(head, 1, sizeof(head), f);
    if (r < sizeof(head)) { fclose(f); errno = EINVAL; return NULL; }
    if (!allowed_signature(head, sizeof(head))) { fclose(f); errno = EINVAL; return NULL; }

    unsigned char* data = (unsigned char*)malloc((size_t)st.st_size);
    if (!data) { fclose(f); errno = ENOMEM; return NULL; }
    fseek(f, 0, SEEK_SET);
    size_t nread = fread(data, 1, (size_t)st.st_size, f);
    fclose(f);
    if (nread != (size_t)st.st_size) { free(data); errno = EIO; return NULL; }

    char* encoded = base64_encode(data, nread);
    free(data);
    if (!encoded) { errno = ENOMEM; return NULL; }

    if (!secure_output_within_cwd(output_path)) {
        free(encoded);
        errno = EACCES;
        return NULL;
    }

    ensure_dirs(output_path);
    char tmp_path[4096];
    snprintf(tmp_path, sizeof(tmp_path), "%s.tmp", output_path);
    FILE* out = fopen(tmp_path, "wb");
    if (!out) { free(encoded); return NULL; }
    size_t to_write = strlen(encoded);
    if (fwrite(encoded, 1, to_write, out) != to_write) {
        fclose(out);
        remove(tmp_path);
        free(encoded);
        errno = EIO;
        return NULL;
    }
    fclose(out);
    /* Atomic-ish replace */
    remove(output_path);
    if (rename(tmp_path, output_path) != 0) {
        remove(tmp_path);
        free(encoded);
        return NULL;
    }

    return encoded; /* caller must free */
}

static int write_bytes(const char* path, const unsigned char* buf, size_t len) {
    ensure_dirs(path);
    FILE* f = fopen(path, "wb");
    if (!f) return -1;
    size_t w = fwrite(buf, 1, len, f);
    fclose(f);
    return (w == len) ? 0 : -1;
}

int main(void) {
    /* Prepare 1x1 PNG */
    const char* png_b64 = "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mP8/x8AAwMBAe0BqZcAAAAASUVORK5CYII=";
    size_t png_len = 0;
    unsigned char* png = base64_decode(png_b64, &png_len);
    if (!png) {
        fprintf(stderr, "Failed to decode test PNG\n");
        return 1;
    }

    const char* in_dir = "test_inputs_c";
    const char* out_dir = "encoded_outputs_c";
    MKDIR(in_dir);
    MKDIR(out_dir);

    const char* in_names[5]  = {"img1.png","img2.PNG","img3.pNg","img4.png","img5.png"};
    const char* out_names[5] = {"img1.b64","img2.b64","img3.b64","img4.b64","img5.b64"};

    char in_path[512], out_path[512];
    for (int i = 0; i < 5; ++i) {
        snprintf(in_path, sizeof(in_path), "%s%c%s", in_dir, PATH_SEP, in_names[i]);
        if (write_bytes(in_path, png, png_len) != 0) {
            fprintf(stderr, "Failed to write test file %s\n", in_path);
            free(png);
            return 1;
        }
    }

    for (int i = 0; i < 5; ++i) {
        snprintf(in_path, sizeof(in_path), "%s%c%s", in_dir, PATH_SEP, in_names[i]);
        snprintf(out_path, sizeof(out_path), "%s%c%s", out_dir, PATH_SEP, out_names[i]);
        char* enc = upload_image(in_path, out_path);
        if (!enc) {
            fprintf(stderr, "Test %d failed: %s\n", i+1, strerror(errno));
            free(png);
            return 1;
        }
        printf("Test %d -> output: %s (encoded length=%zu)\n", i+1, out_path, strlen(enc));
        free(enc);
    }

    free(png);
    return 0;
}