/* Chain-of-Through process:
   1) Problem understanding: Provide a function to read file content from a base directory and filename with validation,
      and a main that tests 5 cases and optionally reads a command-line argument filename.
   2) Security requirements: Validate filename to safe characters only, restrict to a base directory, ensure canonical
      path stays within base, enforce maximum file size, handle errors gracefully.
   3) Secure coding generation: Implement validation, realpath checks, stat-based size limit, safe dynamic allocation,
      and clear error reporting.
   4) Code review: Checked for buffer overflows by using snprintf and dynamic allocation, verified prefix checks after realpath,
      restricted filename chars, and ensured resources are closed and memory is freed.
   5) Secure code output: Final code implements safeguards accordingly.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define PATH_SEP '\\'
#else
#include <unistd.h>
#define PATH_SEP '/'
#endif

#define MAX_BYTES 1024

static int is_valid_filename(const char* name) {
    if (name == NULL) return 0;
    size_t len = strlen(name);
    if (len == 0 || len > 255) return 0;
    for (size_t i = 0; i < len; ++i) {
        char c = name[i];
        if (!(isalnum((unsigned char)c) || c == '.' || c == '_' || c == '-')) {
            return 0;
        }
    }
    return 1;
}

static char* make_error(const char* msg) {
    if (!msg) msg = "Unknown error";
    const char* prefix = "ERROR: ";
    size_t total = strlen(prefix) + strlen(msg) + 1;
    char* out = (char*)malloc(total);
    if (!out) return NULL;
    strcpy(out, prefix);
    strcat(out, msg);
    return out;
}

static char* join_path(const char* base, const char* name) {
    size_t blen = strlen(base);
    size_t nlen = strlen(name);
    int need_sep = (blen > 0 && base[blen - 1] != '/' && base[blen - 1] != '\\');
    size_t total = blen + (need_sep ? 1 : 0) + nlen + 1;
    char* path = (char*)malloc(total);
    if (!path) return NULL;
    strcpy(path, base);
    if (need_sep) {
#ifdef _WIN32
        path[blen] = '\\';
#else
        path[blen] = '/';
#endif
        path[blen + 1] = '\0';
    }
    strcat(path, name);
    return path;
}

static int starts_with_path(const char* base, const char* target) {
    // Ensure target is within base directory
#ifdef _WIN32
    // Case-insensitive comparison on Windows
    size_t b = strlen(base);
    size_t t = strlen(target);
    if (t < b) return 0;
    for (size_t i = 0; i < b; ++i) {
        char cb = tolower((unsigned char)base[i]);
        char ct = tolower((unsigned char)target[i]);
        if (cb != ct) return 0;
    }
    if (t == b) return 1;
    char sep = target[b];
    return (sep == '\\' || sep == '/');
#else
    size_t b = strlen(base);
    if (strncmp(base, target, b) != 0) return 0;
    if (target[b] == '\0') return 1;
    return target[b] == '/';
#endif
}

/* Secure file read: returns a heap-allocated string with file content or "ERROR: ..." message.
   Caller must free the returned string. */
char* safe_read_file(const char* baseDir, const char* filename) {
    if (!baseDir || strlen(baseDir) == 0) baseDir = ".";
    if (!is_valid_filename(filename)) {
        return make_error("Invalid filename format. Allowed: letters, digits, . _ - (1..255 chars)");
    }

    // Build target path
    char* path = join_path(baseDir, filename);
    if (!path) return make_error("Memory allocation failed.");

    // Stat the file
    struct stat st;
    if (stat(path, &st) != 0) {
        free(path);
        return make_error("File does not exist or cannot be accessed.");
    }
#ifndef _WIN32
    if (!S_ISREG(st.st_mode)) {
        free(path);
        return make_error("Not a regular file.");
    }
#else
    // On Windows, stat for regular file is less clear; proceed to open later
#endif
    if ((unsigned long long)st.st_size > (unsigned long long)MAX_BYTES) {
        free(path);
        char buf[128];
        snprintf(buf, sizeof(buf), "File too large (%lld bytes). Max allowed: %d bytes.",
                 (long long)st.st_size, (int)MAX_BYTES);
        return make_error(buf);
    }

    // Resolve canonical paths to ensure target stays within base
    char base_real_buf[4096];
    char target_real_buf[4096];
#ifdef _WIN32
    // _fullpath provides canonical absolute path on Windows
    if (_fullpath(base_real_buf, baseDir, sizeof(base_real_buf)) == NULL) {
        free(path);
        return make_error("Failed to resolve base directory.");
    }
    if (_fullpath(target_real_buf, path, sizeof(target_real_buf)) == NULL) {
        free(path);
        return make_error("Failed to resolve target path.");
    }
#else
    if (realpath(baseDir, base_real_buf) == NULL) {
        free(path);
        return make_error("Failed to resolve base directory.");
    }
    if (realpath(path, target_real_buf) == NULL) {
        free(path);
        return make_error("Failed to resolve target path.");
    }
#endif

    if (!starts_with_path(base_real_buf, target_real_buf)) {
        free(path);
        return make_error("Unsafe path detected.");
    }

    // Read the file
    FILE* f = fopen(target_real_buf, "rb");
    if (!f) {
        free(path);
        return make_error("Unable to open file for reading.");
    }

    size_t size = (size_t)st.st_size;
    char* content = (char*)malloc(size + 1);
    if (!content) {
        fclose(f);
        free(path);
        return make_error("Memory allocation failed.");
    }
    size_t readn = 0;
    if (size > 0) {
        readn = fread(content, 1, size, f);
        if (readn != size) {
            fclose(f);
            free(path);
            free(content);
            return make_error("Failed while reading the file.");
        }
    }
    content[size] = '\0';
    fclose(f);
    free(path);
    return content;
}

int main(int argc, char** argv) {
    const char* base = "task65_c_dir";
#ifdef _WIN32
    _mkdir(base);
#else
    mkdir(base, 0700);
#endif

    // Create test files
    {
        char* p = join_path(base, "sample1.txt");
        FILE* f = fopen(p, "wb");
        if (f) { fputs("Hello, Task65!", f); fclose(f); }
        free(p);
    }
    {
        char* p = join_path(base, "sample-2.txt");
        FILE* f = fopen(p, "wb");
        if (f) { fputs("Second sample content.", f); fclose(f); }
        free(p);
    }
    {
        char* p = join_path(base, "big.txt");
        FILE* f = fopen(p, "wb");
        if (f) {
            for (int i = 0; i < 1500; ++i) fputc('A', f);
            fclose(f);
        }
        free(p);
    }

    const char* tests[5] = {
        "sample1.txt",     // valid
        "sample-2.txt",    // valid
        "../etc/passwd",   // invalid (format disallowed)
        "missing.txt",     // missing
        "big.txt"          // too large
    };

    for (int i = 0; i < 5; ++i) {
        char* result = safe_read_file(base, tests[i]);
        printf("Test %d (%s):\n", i + 1, tests[i]);
        if (result) {
            printf("%s\n", result);
            free(result);
        } else {
            printf("ERROR: Allocation failure.\n");
        }
        printf("-----\n");
    }

    if (argc > 1) {
        char* res = safe_read_file(".", argv[1]);
        printf("Command-line read:\n");
        if (res) {
            printf("%s\n", res);
            free(res);
        } else {
            printf("ERROR: Allocation failure.\n");
        }
    }

    return 0;
}