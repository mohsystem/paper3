#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* read_file(const char* path) {
    FILE* fp = fopen(path, "rb");
    if (!fp) return NULL;
    if (fseek(fp, 0, SEEK_END) != 0) { fclose(fp); return NULL; }
    long size = ftell(fp);
    if (size < 0) { fclose(fp); return NULL; }
    if (fseek(fp, 0, SEEK_SET) != 0) { fclose(fp); return NULL; }

    size_t alloc_size = (size_t)size + 1;
    char* buffer = (char*)malloc(alloc_size);
    if (!buffer) { fclose(fp); return NULL; }

    size_t read = fread(buffer, 1, (size_t)size, fp);
    buffer[read] = '\0';
    fclose(fp);
    return buffer;
}

int main(int argc, char** argv) {
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            char* content = read_file(argv[i]);
            if (content) {
                fputs(content, stdout);
                size_t len = strlen(content);
                if (len == 0 || content[len - 1] != '\n') fputc('\n', stdout);
                free(content);
            } else {
                fprintf(stderr, "ERROR reading %s\n", argv[i]);
            }
        }
        return 0;
    }

    // 5 test cases using files in the current directory if no command-line args provided
    const char* names[5];
    char buffers[5][64];
    for (int i = 0; i < 5; ++i) {
        snprintf(buffers[i], sizeof(buffers[i]), "task70_test_c_%d.txt", i + 1);
        names[i] = buffers[i];
        FILE* fp = fopen(names[i], "wb");
        if (fp) {
            char line[64];
            snprintf(line, sizeof(line), "Test case %d content.\n", i + 1);
            fputs(line, fp);
            fclose(fp);
        }
    }

    for (int i = 0; i < 5; ++i) {
        char* content = read_file(names[i]);
        if (content) {
            fputs(content, stdout);
            free(content);
        }
        remove(names[i]);
    }

    return 0;
}