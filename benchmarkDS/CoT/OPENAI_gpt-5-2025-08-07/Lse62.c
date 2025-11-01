/* Chain-of-Through process:
 * 1) Problem understanding: Stream-read a file, compute 32-bit checksum (sum of bytes mod 2^32), treat checksum==0 as empty, save stats.
 * 2) Security requirements: Avoid reading entire file; check I/O return values; avoid overflows; close resources properly.
 * 3) Secure coding generation: Use buffered fread; size-checked loops; use uint32_t for checksum.
 * 4) Code review: Validate error branches and ensure consistent output formatting.
 * 5) Secure code output: Final code uses safe patterns and deterministic behavior.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    unsigned long long bytesRead;
    uint32_t checksum; /* 32-bit sum of bytes */
} ResultC;

ResultC process_file(const char* input_path, const char* output_path) {
    const size_t BUF_SIZE = 8192;
    ResultC res;
    res.bytesRead = 0ULL;
    res.checksum = 0u;

    FILE* in = fopen(input_path, "rb");
    if (!in) {
        /* Still write an output file with zeros for consistency */
        FILE* out_fail = fopen(output_path, "wb");
        if (out_fail) {
            fprintf(out_fail, "bytesRead=%llu\nchecksum=%u\n", 0ULL, 0u);
            fclose(out_fail);
        }
        return res;
    }

    unsigned char* buf = (unsigned char*)malloc(BUF_SIZE);
    if (!buf) {
        fclose(in);
        FILE* out_fail = fopen(output_path, "wb");
        if (out_fail) {
            fprintf(out_fail, "bytesRead=%llu\nchecksum=%u\n", 0ULL, 0u);
            fclose(out_fail);
        }
        return res;
    }

    for (;;) {
        size_t n = fread(buf, 1, BUF_SIZE, in);
        if (n == 0) {
            if (feof(in)) break;
            /* read error */
            break;
        }
        res.bytesRead += (unsigned long long)n;
        for (size_t i = 0; i < n; ++i) {
            res.checksum = (uint32_t)(res.checksum + (uint32_t)buf[i]);
        }
    }

    free(buf);
    fclose(in);

    /* checksum == 0 => buffer considered empty (per spec) */
    FILE* out = fopen(output_path, "wb");
    if (out) {
        /* Ensure consistent formatting across platforms */
        fprintf(out, "bytesRead=%llu\nchecksum=%u\n", res.bytesRead, res.checksum);
        fclose(out);
    }

    return res;
}

static int write_file(const char* path, const unsigned char* data, size_t len) {
    FILE* f = fopen(path, "wb");
    if (!f) return -1;
    if (len > 0) {
        size_t w = fwrite(data, 1, len, f);
        if (w != len) {
            fclose(f);
            return -2;
        }
    }
    fclose(f);
    return 0;
}

int main(void) {
    const char* in_files[5] = {
        "c_in_1_empty.bin",
        "c_in_2_hello.txt",
        "c_in_3_zeros.bin",
        "c_in_4_seq.bin",
        "c_in_5_large.bin"
    };
    const char* out_files[5] = {
        "c_out_1.txt",
        "c_out_2.txt",
        "c_out_3.txt",
        "c_out_4.txt",
        "c_out_5.txt"
    };

    /* Prepare inputs */
    write_file(in_files[0], (const unsigned char*)"", 0);

    const unsigned char hello[] = "Hello, world!";
    write_file(in_files[1], hello, sizeof(hello) - 1);

    unsigned char zeros[1024];
    memset(zeros, 0, sizeof(zeros));
    write_file(in_files[2], zeros, sizeof(zeros));

    unsigned char seq[4096];
    for (size_t i = 0; i < sizeof(seq); ++i) seq[i] = (unsigned char)(i & 0xFF);
    write_file(in_files[3], seq, sizeof(seq));

    unsigned char large[10000];
    for (size_t i = 0; i < sizeof(large); ++i) large[i] = (unsigned char)('A' + (i % 26));
    write_file(in_files[4], large, sizeof(large));

    for (int i = 0; i < 5; ++i) {
        ResultC r = process_file(in_files[i], out_files[i]);
        printf("Test %d -> bytesRead=%llu, checksum=%u\n", i + 1, r.bytesRead, r.checksum);
    }
    return 0;
}