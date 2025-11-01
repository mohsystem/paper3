#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct {
    unsigned long long bytes_read;
    uint32_t checksum;
} Result;

static int write_bytes(const char* path, const unsigned char* data, size_t len) {
    FILE* f = fopen(path, "wb");
    if (!f) return -1;
    if (len > 0 && fwrite(data, 1, len, f) != len) {
        fclose(f);
        return -1;
    }
    fclose(f);
    return 0;
}

Result process_file(const char* input_path, const char* output_path) {
    Result res;
    res.bytes_read = 0;
    res.checksum = 0;

    FILE* f = fopen(input_path, "rb");
    if (!f) {
        // Write zero results to output as well
        FILE* out = fopen(output_path, "wb");
        if (out) {
            fprintf(out, "bytes=%llu\nchecksum=%u\n", res.bytes_read, res.checksum);
            fclose(out);
        }
        return res;
    }

    if (fseek(f, 0, SEEK_END) == 0) {
        long sz = ftell(f);
        if (sz < 0) sz = 0;
        res.bytes_read = (unsigned long long) (unsigned long) sz;
        (void) fseek(f, 0, SEEK_SET);

        unsigned char* buf = NULL;
        if (res.bytes_read > 0) {
            buf = (unsigned char*) malloc((size_t)res.bytes_read);
            if (!buf) {
                fclose(f);
                FILE* out = fopen(output_path, "wb");
                if (out) {
                    fprintf(out, "bytes=%llu\nchecksum=%u\n", res.bytes_read, res.checksum);
                    fclose(out);
                }
                return res;
            }
            size_t n = fread(buf, 1, (size_t)res.bytes_read, f);
            res.bytes_read = n;
            uint32_t sum = 0;
            for (size_t i = 0; i < n; ++i) sum += buf[i];
            res.checksum = sum & 0xFFFFFFFFu;
            free(buf);
        } else {
            res.bytes_read = 0;
            res.checksum = 0;
        }
    }
    fclose(f);

    FILE* out = fopen(output_path, "wb");
    if (out) {
        fprintf(out, "bytes=%llu\nchecksum=%u\n", res.bytes_read, res.checksum);
        fclose(out);
    }
    return res;
}

int main(void) {
    // Test case 1: Empty file
    const char* in1 = "c_in1.bin";
    const char* out1 = "c_out1.txt";
    write_bytes(in1, (const unsigned char*)"", 0);
    Result r1 = process_file(in1, out1);
    printf("Test 1 -> bytes=%llu, checksum=%u, saved: %s\n", r1.bytes_read, r1.checksum, out1);

    // Test case 2: "Hello, world!"
    const char* in2 = "c_in2.bin";
    const char* out2 = "c_out2.txt";
    const unsigned char msg2[] = "Hello, world!";
    write_bytes(in2, msg2, sizeof(msg2) - 1);
    Result r2 = process_file(in2, out2);
    printf("Test 2 -> bytes=%llu, checksum=%u, saved: %s\n", r2.bytes_read, r2.checksum, out2);

    // Test case 3: 1024 bytes 0..255 repeating
    const char* in3 = "c_in3.bin";
    const char* out3 = "c_out3.txt";
    unsigned char* data3 = (unsigned char*) malloc(1024);
    for (size_t i = 0; i < 1024; ++i) data3[i] = (unsigned char)(i & 0xFF);
    write_bytes(in3, data3, 1024);
    free(data3);
    Result r3 = process_file(in3, out3);
    printf("Test 3 -> bytes=%llu, checksum=%u, saved: %s\n", r3.bytes_read, r3.checksum, out3);

    // Test case 4: Non-ASCII pattern repeated
    const char* in4 = "c_in4.bin";
    const char* out4 = "c_out4.txt";
    unsigned char pattern4[] = {0x00, 0xFF, 0x10, 0x7F, 0x80};
    unsigned char data4[100];
    for (size_t i = 0; i < 100; ++i) data4[i] = pattern4[i % (sizeof(pattern4)/sizeof(pattern4[0]))];
    write_bytes(in4, data4, 100);
    Result r4 = process_file(in4, out4);
    printf("Test 4 -> bytes=%llu, checksum=%u, saved: %s\n", r4.bytes_read, r4.checksum, out4);

    // Test case 5: 4096 bytes deterministic generator
    const char* in5 = "c_in5.bin";
    const char* out5 = "c_out5.txt";
    unsigned char* data5 = (unsigned char*) malloc(4096);
    for (size_t i = 0; i < 4096; ++i) data5[i] = (unsigned char)(((i * 31) + 7) & 0xFF);
    write_bytes(in5, data5, 4096);
    free(data5);
    Result r5 = process_file(in5, out5);
    printf("Test 5 -> bytes=%llu, checksum=%u, saved: %s\n", r5.bytes_read, r5.checksum, out5);

    return 0;
}