#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if defined(_WIN32)
#include <sys/stat.h>
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#define STAT _stat
#else
#include <sys/stat.h>
#include <unistd.h>
#define MKDIR(path) mkdir(path, 0700)
#define STAT stat
#endif

#define MAX_SIZE (5U * 1024U * 1024U) // 5MB

static int is_valid_filename(const char* filename) {
    if (filename == NULL) return 0;
    size_t len = 0;
    for (const unsigned char* p = (const unsigned char*)filename; *p; ++p) {
        unsigned char ch = *p;
        if (ch < 32 || ch == 127) return 0; // control chars
        switch (ch) {
            case '<': case '>': case ':': case '"':
            case '/': case '\\': case '|': case '?': case '*':
                return 0;
            default:
                break;
        }
        len++;
        if (len > 255) return 0;
    }
    if (len == 0) return 0;
    if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0) return 0;
    return 1;
}

char* read_file_secure(const char* filename) {
    if (!is_valid_filename(filename)) {
        fprintf(stderr, "Invalid filename format.\n");
        return NULL;
    }

    struct STAT st;
    if (STAT(filename, &st) != 0) {
        fprintf(stderr, "File does not exist.\n");
        return NULL;
    }
#if defined(_WIN32)
    if ((st.st_mode & _S_IFREG) == 0) {
        fprintf(stderr, "Not a regular file.\n");
        return NULL;
    }
#else
    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "Not a regular file.\n");
        return NULL;
    }
#endif

    if ((unsigned long)st.st_size > MAX_SIZE) {
        fprintf(stderr, "File too large.\n");
        return NULL;
    }

    FILE* f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "File is not readable.\n");
        return NULL;
    }

    size_t sz = (size_t)st.st_size;
    char* buf = (char*)malloc(sz + 1);
    if (!buf) {
        fprintf(stderr, "Memory allocation failure.\n");
        fclose(f);
        return NULL;
    }

    size_t read_total = 0;
    while (read_total < sz) {
        size_t n = fread(buf + read_total, 1, sz - read_total, f);
        if (n == 0) {
            if (ferror(f)) {
                fprintf(stderr, "I/O error while reading.\n");
                free(buf);
                fclose(f);
                return NULL;
            }
            break;
        }
        read_total += n;
    }
    buf[read_total] = '\0';
    fclose(f);
    return buf;
}

static void write_sample(const char* name, const char* content) {
    FILE* f = fopen(name, "wb");
    if (!f) {
        fprintf(stderr, "Failed to write sample file: %s\n", name);
        return;
    }
    size_t len = strlen(content);
    if (len > 0) {
        if (fwrite(content, 1, len, f) != len) {
            fprintf(stderr, "Failed to write complete sample content.\n");
        }
    }
    fclose(f);
}

int main(int argc, char** argv) {
    // Prepare sample resources
    write_sample("testfile.txt", "Hello from Task65 (C)!\nThis is a secure read test.\n");
    MKDIR("testdir");

    const char* tests[5] = {
        "testfile.txt",     // valid
        "nonexistent.txt",  // not exist
        "bad|name.txt",     // invalid char
        "../secret.txt",    // traversal attempt (invalid)
        "testdir"           // directory
    };

    for (int i = 0; i < 5; ++i) {
        printf("=== C Test %d: %s ===\n", i + 1, tests[i]);
        char* data = read_file_secure(tests[i]);
        if (data) {
            printf("Content length: %zu\n", strlen(data));
            printf("%s\n", data);
            free(data);
        } else {
            printf("Read failed.\n");
        }
    }

    // If an argument is provided, try to read it
    if (argc > 1) {
        char* res = read_file_secure(argv[1]);
        if (res) {
            printf("=== C Arg Read ===\n");
            printf("%s\n", res);
            free(res);
        } else {
            printf("Could not read file from argument.\n");
        }
    }

    return 0;
}