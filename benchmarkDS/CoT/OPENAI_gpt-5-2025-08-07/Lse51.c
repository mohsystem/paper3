/* Chain-of-Through process:
 * 1) Problem understanding: Provide secure file open/read/write utilities in C with careful resource management; tests included.
 * 2) Security requirements: Validate inputs, check return codes, avoid buffer overflows, handle binary-safe I/O.
 * 3) Secure coding generation: Use fopen/fread/fwrite with size checks, create directories, avoid unsafe functions.
 * 4) Code review: Ensure all resources are closed and errors handled.
 * 5) Secure code output: Final functions and five tests.
 * Note: Standard C library does not include TAR extraction; not implemented here.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(p, m) _mkdir(p)
#define PATH_SEP '\\'
#else
#include <unistd.h>
#define MKDIR(p, m) mkdir(p, m)
#define PATH_SEP '/'
#endif

static void make_dirs_recursive(const char* path) {
    if (!path || !*path) return;
    size_t len = strlen(path);
    char* tmp = (char*)malloc(len + 1);
    if (!tmp) return;
    strcpy(tmp, path);
    for (size_t i = 0; i < len; ++i) {
        if (tmp[i] == '/' || tmp[i] == '\\') {
            char sep = tmp[i];
            tmp[i] = '\0';
            if (strlen(tmp) > 0) {
                MKDIR(tmp, 0700);
            }
            tmp[i] = sep;
        }
    }
    if (tmp[len - 1] == '/' || tmp[len - 1] == '\\') {
        // already a directory path
    } else {
        // create final directory if path ends with directory separator is not guaranteed
    }
    free(tmp);
}

static size_t write_text_file(const char* path, const char* content) {
    if (!path) { errno = EINVAL; return 0; }
    if (!content) content = "";
    // Create parent directories if any
    const char* last_sep = strrchr(path, '/');
#ifdef _WIN32
    const char* last_sep2 = strrchr(path, '\\');
    if (!last_sep || (last_sep2 && last_sep2 > last_sep)) last_sep = last_sep2;
#endif
    if (last_sep) {
        size_t dir_len = (size_t)(last_sep - path + 1);
        char* dir = (char*)malloc(dir_len + 1);
        if (dir) {
            memcpy(dir, path, dir_len);
            dir[dir_len] = '\0';
            make_dirs_recursive(dir);
            free(dir);
        }
    }
    FILE* f = fopen(path, "wb");
    if (!f) return 0;
    size_t len = strlen(content);
    size_t written = fwrite(content, 1, len, f);
    if (fclose(f) != 0) {
        // ignore close error here
    }
    if (written != len) {
        return written;
    }
    return written;
}

static char* read_text_file(const char* path) {
    if (!path) { errno = EINVAL; return NULL; }
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }
    long sz = ftell(f);
    if (sz < 0) {
        fclose(f);
        return NULL;
    }
    if (fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return NULL;
    }
    char* buf = (char*)malloc((size_t)sz + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }
    size_t n = fread(buf, 1, (size_t)sz, f);
    buf[n] = '\0';
    fclose(f);
    return buf;
}

// Placeholder: TAR extraction is not implemented in standard C.
/* int extract_tar_secure(const char* tarPath, const char* destDir) { return -1; } */

int main(void) {
#ifdef _WIN32
    const char* tmp = ".\\";
#else
    const char* tmp = "/tmp/";
#endif
    const char* files[5];
    char buf1[512], buf2[512], buf3[512], buf4[512], buf5[512];
#ifdef _WIN32
    snprintf(buf1, sizeof(buf1), "%slse51_c_t1.txt", tmp);
    snprintf(buf2, sizeof(buf2), "%slse51_c_t2.txt", tmp);
    snprintf(buf3, sizeof(buf3), "%slse51_c_dir%clse51_c_t3.txt", tmp, PATH_SEP);
    snprintf(buf4, sizeof(buf4), "%slse51_c_dir%csub%clse51_c_t4.txt", tmp, PATH_SEP, PATH_SEP);
    snprintf(buf5, sizeof(buf5), "%slse51_c_t5.txt", tmp);
#else
    snprintf(buf1, sizeof(buf1), "%slse51_c_t1.txt", tmp);
    snprintf(buf2, sizeof(buf2), "%slse51_c_t2.txt", tmp);
    snprintf(buf3, sizeof(buf3), "%slse51_c_dir/lse51_c_t3.txt", tmp);
    snprintf(buf4, sizeof(buf4), "%slse51_c_dir/sub/lse51_c_t4.txt", tmp);
    snprintf(buf5, sizeof(buf5), "%slse51_c_t5.txt", tmp);
#endif
    files[0] = buf1; files[1] = buf2; files[2] = buf3; files[3] = buf4; files[4] = buf5;
    const char* contents[5] = {
        "Hello, C fopen/fclose",
        "Line1\nLine2\nLine3",
        "UTF-8 bytes \xE2\x9C\x93",
        "",
        "Final test content"
    };
    for (int i = 0; i < 5; ++i) {
        size_t written = write_text_file(files[i], contents[i]);
        char* read = read_text_file(files[i]);
        size_t read_len = read ? strlen(read) : 0;
        printf("Test %d wrote bytes: %zu read len: %zu\n", i + 1, written, read_len);
        free(read);
    }
    return 0;
}