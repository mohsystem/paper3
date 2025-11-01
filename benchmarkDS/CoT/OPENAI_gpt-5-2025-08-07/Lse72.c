#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#if defined(_WIN32)
  #include <direct.h>
  #include <io.h>
  #include <fcntl.h>
  #include <sys/stat.h>
  #define PATH_SEP '\\'
  #define MKDIR(p) _mkdir(p)
  #define ACCESS(p) _access(p, 0)
  #define CLOSEFD _close
  #define OPENFD _open
  #define WRITEFD _write
  #define O_EXCLUSIVE (_O_CREAT | _O_EXCL | _O_RDWR | _O_BINARY)
#else
  #include <sys/stat.h>
  #include <sys/types.h>
  #include <unistd.h>
  #include <fcntl.h>
  #define PATH_SEP '/'
  #define MKDIR(p) mkdir(p, 0700)
  #define ACCESS(p) access(p, F_OK)
  #define CLOSEFD close
  #define OPENFD open
  #define WRITEFD write
  #define O_EXCLUSIVE (O_CREAT | O_EXCL | O_RDWR)
#endif

// Chain-of-Through process:
// 1) Problem understanding: Implement upload_file that writes data to 'uploads' next to this source file.
// 2) Security requirements: Sanitize filename, whitelist extensions, size limit, avoid overwrite, normalize paths.
// 3) Secure coding generation: Implement below with careful checks and atomic create where possible.
// 4) Code review: Check pointer validity, buffer sizes, and error handling.
// 5) Secure code output: Final function below.

static int has_allowed_extension(const char* ext) {
    const char* allowed[] = {"txt","pdf","png","jpg","jpeg","gif","bin"};
    for (size_t i = 0; i < sizeof(allowed)/sizeof(allowed[0]); ++i) {
        if (strcmp(ext, allowed[i]) == 0) return 1;
    }
    return 0;
}

static void to_lower_str(char* s) {
    for (; *s; ++s) *s = (char)tolower((unsigned char)*s);
}

static char* sanitize_base(const char* base) {
    size_t n = strlen(base);
    char* out = (char*)malloc(n + 1);
    if (!out) return NULL;
    size_t j = 0;
    for (size_t i = 0; i < n; ++i) {
        unsigned char c = (unsigned char)base[i];
        if (isalnum(c) || c == '.' || c == '_' || c == '-') {
            out[j++] = (char)c;
        } else {
            out[j++] = '_';
        }
        if (j >= 200) break; // limit length
    }
    out[j] = '\0';
    // remove leading dots
    size_t start = 0;
    while (out[start] == '.') start++;
    if (start > 0) memmove(out, out + start, j - start + 1);
    if (out[0] == '\0') {
        free(out);
        out = (char*)malloc(5);
        if (!out) return NULL;
        strcpy(out, "file");
    }
    return out;
}

static char* join_path(const char* a, const char* b) {
    size_t la = strlen(a);
    size_t lb = strlen(b);
    int need_sep = (la > 0 && a[la-1] != PATH_SEP);
    size_t total = la + (need_sep ? 1 : 0) + lb + 1;
    char* out = (char*)malloc(total);
    if (!out) return NULL;
    strcpy(out, a);
    if (need_sep) {
        out[la] = PATH_SEP;
        out[la+1] = '\0';
    }
    strcat(out, b);
    return out;
}

static char* dir_of_source(void) {
    // Derive directory from __FILE__
    const char* file = __FILE__;
    const char* last_sep = strrchr(file, '/');
#if defined(_WIN32)
    const char* last_sep2 = strrchr(file, '\\');
    if (!last_sep || (last_sep2 && last_sep2 > last_sep)) last_sep = last_sep2;
#endif
    if (!last_sep) {
        // No separator, return "."
        char* d = (char*)malloc(2);
        if (!d) return NULL;
        strcpy(d, ".");
        return d;
    }
    size_t len = (size_t)(last_sep - file);
    char* dir = (char*)malloc(len + 1);
    if (!dir) return NULL;
    memcpy(dir, file, len);
    dir[len] = '\0';
    return dir;
}

char* upload_file(const char* originalFilename, const unsigned char* data, size_t len) {
    if (!originalFilename) {
        return NULL;
    }
    if (!data && len > 0) {
        return NULL;
    }

    const size_t MAX_SIZE = 5 * 1024 * 1024; // 5 MB
    if (len > MAX_SIZE) {
        return NULL;
    }

    // Strip directories
    const char* name = originalFilename;
    const char* slash1 = strrchr(name, '/');
#if defined(_WIN32)
    const char* slash2 = strrchr(name, '\\');
    if (!slash1 || (slash2 && slash2 > slash1)) slash1 = slash2;
#endif
    if (slash1) name = slash1 + 1;

    // Separate base and ext
    char* name_copy = strdup(name);
    if (!name_copy) return NULL;
    char* dot = strrchr(name_copy, '.');
    char* ext = NULL;
    if (dot && dot != name_copy && *(dot + 1) != '\0') {
        *dot = '\0';
        ext = dot + 1;
        to_lower_str(ext);
    } else {
        free(name_copy);
        return NULL; // missing extension
    }
    if (!has_allowed_extension(ext)) {
        free(name_copy);
        return NULL; // disallowed extension
    }

    char* safe_base = sanitize_base(name_copy);
    free(name_copy);
    if (!safe_base) return NULL;

    // Build safe filename
    size_t safe_len = strlen(safe_base) + 1 + strlen(ext) + 1;
    char* safe_name = (char*)malloc(safe_len);
    if (!safe_name) {
        free(safe_base);
        return NULL;
    }
    snprintf(safe_name, safe_len, "%s.%s", safe_base, ext);
    free(safe_base);

    // Determine upload directory relative to this source file
    char* baseDir = dir_of_source();
    if (!baseDir) {
        free(safe_name);
        return NULL;
    }
    char* uploadDir = join_path(baseDir, "uploads");
    free(baseDir);
    if (!uploadDir) {
        free(safe_name);
        return NULL;
    }

    // Create uploads directory if not exists
    if (ACCESS(uploadDir) != 0) {
        MKDIR(uploadDir);
    }

    // Build candidate path and ensure uniqueness using O_EXCL
    char* candidate = join_path(uploadDir, safe_name);
    if (!candidate) {
        free(uploadDir);
        free(safe_name);
        return NULL;
    }

    int counter = 1;
    int fd;
    while (1) {
#if defined(_WIN32)
        fd = OPENFD(candidate, O_EXCLUSIVE, _S_IREAD | _S_IWRITE);
#else
        fd = OPENFD(candidate, O_EXCLUSIVE, 0600);
#endif
        if (fd >= 0) {
            break; // created exclusively
        } else {
            // try new name
            free(candidate);
            size_t new_len = strlen(uploadDir) + 1 + strlen(safe_name) + 32;
            candidate = (char*)malloc(new_len);
            if (!candidate) {
                free(uploadDir);
                free(safe_name);
                return NULL;
            }
            // rebuild base_n.ext
            char* dot2 = strrchr(safe_name, '.');
            if (!dot2) { // fallback
                snprintf(candidate, new_len, "%s%c%s_%d", uploadDir, PATH_SEP, safe_name, counter);
            } else {
                size_t base_len = (size_t)(dot2 - safe_name);
                char tmp_base[256];
                if (base_len > 255) base_len = 255;
                memcpy(tmp_base, safe_name, base_len);
                tmp_base[base_len] = '\0';
                snprintf(candidate, new_len, "%s%c%s_%d.%s", uploadDir, PATH_SEP, tmp_base, counter, ext);
            }
            counter++;
        }
    }

    // Write data
    if (len > 0) {
        if (WRITEFD(fd, data, (unsigned int)len) < 0) {
            CLOSEFD(fd);
            remove(candidate);
            free(candidate);
            free(uploadDir);
            free(safe_name);
            return NULL;
        }
    }
    CLOSEFD(fd);

    // Cleanup
    free(uploadDir);
    free(safe_name);

    // Return absolute-like path if possible (here we return the candidate as is)
    return candidate; // caller must free
}

int main(void) {
    // Test 1: valid
    {
        const unsigned char d1[] = "Hello, World!";
        char* p1 = upload_file("report.txt", d1, sizeof(d1)-1);
        if (p1) {
            printf("Saved: %s\n", p1);
            free(p1);
        } else {
            printf("Error test1\n");
        }
    }

    // Test 2: traversal attempt
    {
        const unsigned char d2[] = "secret";
        char* p2 = upload_file("../../secret.txt", d2, sizeof(d2)-1);
        if (p2) {
            printf("Saved: %s\n", p2);
            free(p2);
        } else {
            printf("Blocked traversal or invalid: OK\n");
        }
    }

    // Test 3: disallowed extension
    {
        const unsigned char d3[] = "#!/bin/sh\necho hi";
        char* p3 = upload_file("script.sh", d3, sizeof(d3)-1);
        if (p3) {
            printf("Saved: %s\n", p3);
            free(p3);
        } else {
            printf("Blocked extension: OK\n");
        }
    }

    // Test 4: weird name with allowed ext
    {
        unsigned char d4[3] = {1,2,3};
        char* p4 = upload_file("inva|id:name?.png", d4, sizeof(d4));
        if (p4) {
            printf("Saved: %s\n", p4);
            free(p4);
        } else {
            printf("Error test4\n");
        }
    }

    // Test 5: duplicate filename should be unique
    {
        const unsigned char d5[] = "Another content";
        char* p5 = upload_file("report.txt", d5, sizeof(d5)-1);
        if (p5) {
            printf("Saved: %s\n", p5);
            free(p5);
        } else {
            printf("Error test5\n");
        }
    }

    return 0;
}