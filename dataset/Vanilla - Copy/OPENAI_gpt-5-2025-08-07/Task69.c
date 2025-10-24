#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int delete_file(const char* path) {
    if (path == NULL || *path == '\0') return 0;
    return remove(path) == 0 ? 1 : 0;
}

static void create_temp_file(char* outPath, size_t outSize) {
    // Generate a temporary filename and create the file
    char buf[L_tmpnam];
    if (tmpnam(buf) == NULL) {
        snprintf(outPath, outSize, "Task69_tmp_%ld.tmp", (long)time(NULL));
    } else {
        snprintf(outPath, outSize, "%s", buf);
    }
    FILE* f = fopen(outPath, "wb");
    if (f) {
        const char* data = "data";
        fwrite(data, 1, strlen(data), f);
        fclose(f);
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            int res = delete_file(argv[i]);
            printf("%s -> %s\n", argv[i], res ? "true" : "false");
        }
        return 0;
    }

    // 5 test cases
    char f1[512], f2[512], f3[512], nonExistent[512];
    create_temp_file(f1, sizeof(f1)); // 1: existing file
    create_temp_file(f2, sizeof(f2)); // 2: existing file
    create_temp_file(f3, sizeof(f3)); // 3: existing file

    snprintf(nonExistent, sizeof(nonExistent), "Task69_nonexistent_%ld.tmp", (long)time(NULL));

    const char* tests[5];
    tests[0] = f1;
    tests[1] = f2;
    tests[2] = f3;
    tests[3] = nonExistent; // non-existent
    tests[4] = f1;          // attempt to delete again

    for (int i = 0; i < 5; ++i) {
        int res = delete_file(tests[i]);
        printf("%s -> %s\n", tests[i], res ? "true" : "false");
    }

    return 0;
}