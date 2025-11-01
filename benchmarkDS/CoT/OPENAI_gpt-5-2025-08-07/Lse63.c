#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

typedef struct {
    size_t bytes_read;
    uint32_t checksum;
    int is_empty;
    int error; /* 0 = ok, non-zero = error */
} Result;

Result process_file(const char* input_path, const char* output_path) {
    Result res;
    res.bytes_read = 0;
    res.checksum = 0;
    res.is_empty = 1;
    res.error = 0;

    FILE* in = fopen(input_path, "rb");
    if (!in) {
        res.error = 1;
        return res;
    }

    if (fseek(in, 0, SEEK_END) != 0) {
        fclose(in);
        res.error = 2;
        return res;
    }
    long fsize = ftell(in);
    if (fsize < 0) {
        fclose(in);
        res.error = 3;
        return res;
    }
    if (fseek(in, 0, SEEK_SET) != 0) {
        fclose(in);
        res.error = 4;
        return res;
    }

    size_t size = (size_t)fsize;
    unsigned char* buffer = NULL;
    if (size > 0) {
        buffer = (unsigned char*)malloc(size);
        if (!buffer) {
            fclose(in);
            res.error = 5;
            return res;
        }
        size_t readn = fread(buffer, 1, size, in);
        if (readn != size) {
            free(buffer);
            fclose(in);
            res.error = 6;
            return res;
        }
    }
    fclose(in);

    uint64_t sum = 0;
    for (size_t i = 0; i < size; ++i) {
        sum += (uint64_t)buffer[i];
        sum &= 0xFFFFFFFFULL;
    }
    uint32_t checksum = (uint32_t)(sum & 0xFFFFFFFFULL);
    int is_empty = (checksum == 0U) ? 1 : 0;

    FILE* out = fopen(output_path, "w");
    if (!out) {
        if (buffer) free(buffer);
        res.error = 7;
        return res;
    }
    fprintf(out, "bytes_read=%zu\n", size);
    fprintf(out, "checksum=%u\n", checksum);
    fclose(out);

    if (buffer) free(buffer);

    res.bytes_read = size;
    res.checksum = checksum;
    res.is_empty = is_empty;
    res.error = 0;
    return res;
}

static int write_file(const char* path, const unsigned char* data, size_t len) {
    FILE* f = fopen(path, "wb");
    if (!f) return -1;
    if (len > 0 && fwrite(data, 1, len, f) != len) {
        fclose(f);
        return -2;
    }
    fclose(f);
    return 0;
}

int main(void) {
    char base[128];
    time_t t = time(NULL);
    snprintf(base, sizeof(base), "lse63_c_demo_%ld_", (long)t);

    const char* names[5] = {
        "empty.bin",
        "hello.txt",
        "repeatA.bin",
        "binary.bin",
        "unicode.txt"
    };

    unsigned char hello[] = {'h','e','l','l','o'};
    unsigned char repeatA[1024];
    for (size_t i = 0; i < sizeof(repeatA); ++i) repeatA[i] = (unsigned char)'a';
    unsigned char binary[] = {0x01, 0x02, 0xFF, 0x10, 0x20, 0x7F};
    unsigned char unicode_hello[] = {0xE3,0x81,0x93,0xE3,0x82,0x93,0xE3,0x81,0xAB,0xE3,0x81,0xA1,0xE3,0x81,0xAF}; /* こんにちは in UTF-8 */

    const unsigned char* data[5] = {
        (const unsigned char*)"", hello, repeatA, binary, unicode_hello
    };
    size_t lens[5] = {
        0, sizeof(hello), sizeof(repeatA), sizeof(binary), sizeof(unicode_hello)
    };

    for (int i = 0; i < 5; ++i) {
        char in_path[256];
        char out_path[256];
        snprintf(in_path, sizeof(in_path), "%s%s", base, names[i]);
        snprintf(out_path, sizeof(out_path), "%s%s.out.txt", base, names[i]);

        if (write_file(in_path, data[i], lens[i]) != 0) {
            fprintf(stderr, "Failed to create input file: %s\n", in_path);
            continue;
        }

        Result r = process_file(in_path, out_path);
        if (r.error != 0) {
            fprintf(stderr, "Error processing %s (code %d)\n", in_path, r.error);
        } else {
            printf("Test %d: %s -> bytes=%zu, checksum=%u, empty=%s | out=%s\n",
                   i + 1, in_path, r.bytes_read, r.checksum, r.is_empty ? "true" : "false", out_path);
        }
    }

    return 0;
}