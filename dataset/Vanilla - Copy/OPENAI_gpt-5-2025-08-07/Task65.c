#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>

static int is_valid_filename(const char* name) {
    if (name == NULL) return 0;
    const char* invalid = "<>\"|?*";
    for (const unsigned char* p = (const unsigned char*)name; *p; ++p) {
        if (*p < 32) return 0;
        if (strchr(invalid, (int)*p) != NULL) return 0;
    }
    // Basic component length check
    const char* start = name;
    const char* p = name;
    while (1) {
        if (*p == '/' || *p == '\\' || *p == '\0') {
            size_t len = (size_t)(p - start);
            if (len > 255) return 0;
            if (*p == '\0') break;
            start = p + 1;
        }
        if (*p == '\0') break;
        ++p;
    }
    return 1;
}

static char* alloc_copy(const char* s) {
    size_t n = strlen(s);
    char* out = (char*)malloc(n + 1);
    if (!out) return NULL;
    memcpy(out, s, n + 1);
    return out;
}

char* readFile(const char* filename) {
    if (filename == NULL || strlen(filename) == 0) {
        return alloc_copy("ERROR: Filename is empty.");
    }
    if (!is_valid_filename(filename)) {
        return alloc_copy("ERROR: Filename contains invalid characters.");
    }

    struct stat st;
    if (stat(filename, &st) != 0) {
        return alloc_copy("ERROR: File does not exist.");
    }
    if ((st.st_mode & S_IFMT) == S_IFDIR) {
        return alloc_copy("ERROR: Path is a directory, not a file.");
    }

    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        return alloc_copy("ERROR: File is not readable.");
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return alloc_copy("ERROR: I/O error while seeking.");
    }
    long size = ftell(fp);
    if (size < 0) {
        fclose(fp);
        return alloc_copy("ERROR: I/O error while telling position.");
    }
    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return alloc_copy("ERROR: I/O error while rewinding.");
    }

    char* buf = (char*)malloc((size_t)size + 1);
    if (!buf) {
        fclose(fp);
        return alloc_copy("ERROR: Memory allocation failed.");
    }
    size_t readn = fread(buf, 1, (size_t)size, fp);
    fclose(fp);
    buf[readn] = '\0';
    return buf;
}

static char* create_temp_file_with_content(const char* content) {
    char nameBuf[L_tmpnam];
    if (tmpnam(nameBuf) == NULL) {
        return NULL;
    }
    FILE* f = fopen(nameBuf, "wb");
    if (!f) return NULL;
    fwrite(content, 1, strlen(content), f);
    fclose(f);
    return alloc_copy(nameBuf);
}

int main(int argc, char** argv) {
    if (argc > 1) {
        char* res = readFile(argv[1]);
        printf("CLI arg result:\n%s\n", res ? res : "ERROR: Unknown error.");
        free(res);
    }

    char* tests[5];

    // 1) Valid temp file
    tests[0] = create_temp_file_with_content("Hello from Task65 test case 1.\nLine 2.");

    // 2) Valid temp file with larger content
    {
        // Build content
        char* big = NULL;
        size_t cap = 0, len = 0;
        for (int i = 0; i < 100; ++i) {
            char line[64];
            snprintf(line, sizeof(line), "Line %d: Sample content\n", i + 1);
            size_t add = strlen(line);
            if (len + add + 1 > cap) {
                cap = (cap == 0 ? 256 : cap * 2);
                if (cap < len + add + 1) cap = len + add + 1;
                big = (char*)realloc(big, cap);
                if (!big) break;
            }
            memcpy(big + len, line, add);
            len += add;
            big[len] = '\0';
        }
        if (big) {
            tests[1] = create_temp_file_with_content(big);
            free(big);
        } else {
            tests[1] = NULL;
        }
    }

    // 3) Non-existent file
    tests[2] = alloc_copy("this_file_should_not_exist_1234567890.txt");

    // 4) Empty filename
    tests[3] = alloc_copy("");

    // 5) Invalid filename
    tests[4] = alloc_copy("bad|name.txt");

    for (int i = 0; i < 5; ++i) {
        const char* fn = tests[i] ? tests[i] : "(null)";
        printf("Test %d (%s):\n", i + 1, fn);
        char* res = readFile(tests[i]);
        printf("%s\n", res ? res : "ERROR: Unknown error.");
        free(res);
        if (tests[i]) free(tests[i]);
    }

    return 0;
}