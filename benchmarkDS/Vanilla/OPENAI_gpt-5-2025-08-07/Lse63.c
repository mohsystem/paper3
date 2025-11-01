#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

struct Result {
    size_t bytesRead;
    uint32_t checksum;
};

struct Result process_file(const char* input_path, const char* output_path) {
    struct Result res;
    res.bytesRead = 0;
    res.checksum = 0;

    FILE* fin = fopen(input_path, "rb");
    if (!fin) {
        FILE* fout = fopen(output_path, "w");
        if (fout) {
            fprintf(fout, "bytes=%zu\nchecksum=%u\n", res.bytesRead, res.checksum);
            fclose(fout);
        }
        return res;
    }

    if (fseek(fin, 0, SEEK_END) != 0) {
        fclose(fin);
        FILE* fout = fopen(output_path, "w");
        if (fout) {
            fprintf(fout, "bytes=%zu\nchecksum=%u\n", res.bytesRead, res.checksum);
            fclose(fout);
        }
        return res;
    }
    long sz = ftell(fin);
    if (sz < 0) {
        fclose(fin);
        FILE* fout = fopen(output_path, "w");
        if (fout) {
            fprintf(fout, "bytes=%zu\nchecksum=%u\n", res.bytesRead, res.checksum);
            fclose(fout);
        }
        return res;
    }
    rewind(fin);

    unsigned char* buffer = NULL;
    if (sz > 0) {
        buffer = (unsigned char*)malloc((size_t)sz);
        if (!buffer) {
            fclose(fin);
            FILE* fout = fopen(output_path, "w");
            if (fout) {
                fprintf(fout, "bytes=%zu\nchecksum=%u\n", res.bytesRead, res.checksum);
                fclose(fout);
            }
            return res;
        }
        size_t readCount = fread(buffer, 1, (size_t)sz, fin);
        res.bytesRead = readCount;
        uint32_t sum = 0;
        for (size_t i = 0; i < readCount; ++i) {
            sum += (uint32_t)buffer[i];
        }
        res.checksum = sum;
        free(buffer);
    } else {
        res.bytesRead = 0;
        res.checksum = 0;
    }
    fclose(fin);

    FILE* fout = fopen(output_path, "w");
    if (fout) {
        fprintf(fout, "bytes=%zu\nchecksum=%u\n", res.bytesRead, res.checksum);
        fclose(fout);
    }
    return res;
}

static void write_bytes(const char* path, const unsigned char* data, size_t len) {
    FILE* f = fopen(path, "wb");
    if (!f) return;
    if (len > 0 && data) fwrite(data, 1, len, f);
    fclose(f);
}

int main(void) {
    // Test case 1: Empty file
    const char* in1 = "c_in1.bin";
    const char* out1 = "c_out1.txt";
    write_bytes(in1, NULL, 0);
    struct Result r1 = process_file(in1, out1);
    printf("Test1 -> bytes=%zu checksum=%u\n", r1.bytesRead, r1.checksum);

    // Test case 2: "Hello, World!"
    const char* in2 = "c_in2.bin";
    const char* out2 = "c_out2.txt";
    const unsigned char msg2[] = "Hello, World!";
    write_bytes(in2, msg2, sizeof(msg2) - 1);
    struct Result r2 = process_file(in2, out2);
    printf("Test2 -> bytes=%zu checksum=%u\n", r2.bytesRead, r2.checksum);

    // Test case 3: bytes 1..100
    const char* in3 = "c_in3.bin";
    const char* out3 = "c_out3.txt";
    unsigned char seq[100];
    for (int i = 0; i < 100; ++i) seq[i] = (unsigned char)(i + 1);
    write_bytes(in3, seq, 100);
    struct Result r3 = process_file(in3, out3);
    printf("Test3 -> bytes=%zu checksum=%u\n", r3.bytesRead, r3.checksum);

    // Test case 4: 10 zero bytes
    const char* in4 = "c_in4.bin";
    const char* out4 = "c_out4.txt";
    unsigned char zeros[10] = {0};
    write_bytes(in4, zeros, 10);
    struct Result r4 = process_file(in4, out4);
    printf("Test4 -> bytes=%zu checksum=%u\n", r4.bytesRead, r4.checksum);

    // Test case 5: "abc" repeated 1000 times
    const char* in5 = "c_in5.bin";
    const char* out5 = "c_out5.txt";
    size_t repCount = 1000;
    size_t len5 = repCount * 3;
    unsigned char* rep = (unsigned char*)malloc(len5);
    if (rep) {
        for (size_t i = 0; i < repCount; ++i) {
            rep[i*3 + 0] = 'a';
            rep[i*3 + 1] = 'b';
            rep[i*3 + 2] = 'c';
        }
        write_bytes(in5, rep, len5);
        free(rep);
    } else {
        write_bytes(in5, (const unsigned char*)"abc", 3);
    }
    struct Result r5 = process_file(in5, out5);
    printf("Test5 -> bytes=%zu checksum=%u\n", r5.bytesRead, r5.checksum);

    return 0;
}