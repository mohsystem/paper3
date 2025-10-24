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
    char* buffer = (char*)malloc((size_t)size + 1);
    if (!buffer) { fclose(fp); return NULL; }
    size_t read = fread(buffer, 1, (size_t)size, fp);
    fclose(fp);
    buffer[read] = '\0';
    return buffer;
}

int main(void) {
    const char* filenames[5] = {
        "task36_c_test_1.txt",
        "task36_c_test_2.txt",
        "task36_c_test_3.txt",
        "task36_c_test_4.txt",
        "task36_c_test_5.txt"
    };
    const char* contents[5] = {
        "C Test File 1\nLine A\nLine B",
        "C Test File 2\nHello World",
        "C Test File 3\nSample Content",
        "C Test File 4\nLorem Ipsum",
        "C Test File 5\nEnd of File"
    };

    for (int i = 0; i < 5; ++i) {
        FILE* fp = fopen(filenames[i], "wb");
        if (fp) {
            fwrite(contents[i], 1, strlen(contents[i]), fp);
            fclose(fp);
        }
    }

    for (int i = 0; i < 5; ++i) {
        printf("== C Test %d ==\n", i + 1);
        char* data = read_file(filenames[i]);
        if (data) {
            printf("%s\n", data);
            free(data);
        } else {
            printf("Error reading file: %s\n", filenames[i]);
        }
        remove(filenames[i]);
    }

    return 0;
}