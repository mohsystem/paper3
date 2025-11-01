#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    unsigned char* data;
    size_t size;
} Buffer;

Buffer img(const char* filePath) {
    Buffer buf = {NULL, 0};
    FILE* f = fopen(filePath, "rb");
    if (!f) return buf;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return buf; }
    long sz = ftell(f);
    if (sz < 0) { fclose(f); return buf; }
    if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return buf; }
    if (sz == 0) {
        buf.data = (unsigned char*)malloc(1);
        if (buf.data) buf.data[0] = 0;
        buf.size = 0;
        fclose(f);
        return buf;
    }
    buf.data = (unsigned char*)malloc((size_t)sz);
    if (!buf.data) { fclose(f); return buf; }
    size_t rd = fread(buf.data, 1, (size_t)sz, f);
    buf.size = rd;
    fclose(f);
    return buf;
}

void free_buffer(Buffer* b) {
    if (b && b->data) {
        free(b->data);
        b->data = NULL;
        b->size = 0;
    }
}

int main(void) {
    const char* files[5] = {
        "c_test1.bin", "c_test2.bin", "c_test3.bin", "c_test4.bin", "c_test5.bin"
    };

    FILE* f = fopen(files[0], "wb");
    const char* s = "Hello C";
    fwrite(s, 1, strlen(s), f);
    fclose(f);

    f = fopen(files[1], "wb");
    unsigned char b2[] = {0x00, 0x01, 0x02, 0xFF, 0x10};
    fwrite(b2, 1, sizeof(b2), f);
    fclose(f);

    f = fopen(files[2], "wb");
    const char* rep = "rep";
    for (int i = 0; i < 10; ++i) fwrite(rep, 1, 3, f);
    fclose(f);

    f = fopen(files[3], "wb");
    // empty file
    fclose(f);

    f = fopen(files[4], "wb");
    srand(42);
    for (int i = 0; i < 1024; ++i) {
        unsigned char v = (unsigned char)(rand() % 256);
        fwrite(&v, 1, 1, f);
    }
    fclose(f);

    for (int i = 0; i < 5; ++i) {
        Buffer buf = img(files[i]);
        printf("Read %s -> length=%zu", files[i], buf.size);
        if (buf.size > 0) {
            printf(", first8=[");
            size_t n = buf.size < 8 ? buf.size : 8;
            for (size_t j = 0; j < n; ++j) {
                if (j) printf(", ");
                printf("%u", (unsigned int)buf.data[j]);
            }
            printf("]");
        }
        printf("\n");
        free_buffer(&buf);
    }

    return 0;
}