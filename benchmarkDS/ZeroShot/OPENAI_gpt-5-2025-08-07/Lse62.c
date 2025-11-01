#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#define mkstemp _mktemp_s_unsupported_on_windows
#else
#include <unistd.h>
#endif

struct Result {
    uint64_t bytes_read;
    uint64_t checksum;
};

// Writes "bytes_read=<num>\nchecksum=<num>\n" to output file atomically where possible.
static int write_results_atomic(const char* out_path, uint64_t bytes_read, uint64_t checksum) {
    char tmp_path[4096];
    size_t out_len = strlen(out_path);
    if (out_len + 5 >= sizeof(tmp_path)) {
        return -1;
    }
    snprintf(tmp_path, sizeof(tmp_path), "%s.tmp", out_path);

    FILE* wf = fopen(tmp_path, "wb");
    if (!wf) {
        return -1;
    }
    int ok = 1;
    if (fprintf(wf, "bytes_read=%" PRIu64 "\n", bytes_read) < 0) ok = 0;
    if (ok && fprintf(wf, "checksum=%" PRIu64 "\n", checksum) < 0) ok = 0;
    if (fflush(wf) != 0) ok = 0;
    if (fclose(wf) != 0) ok = 0;
    if (!ok) {
        remove(tmp_path);
        return -1;
    }
    // Replace target
    if (rename(tmp_path, out_path) != 0) {
        // Fallback: try to remove and rename again
        remove(out_path);
        if (rename(tmp_path, out_path) != 0) {
            remove(tmp_path);
            return -1;
        }
    }
    return 0;
}

// Reads a file, computes unsigned 64-bit sum of bytes and byte count, writes to output file.
// Returns 0 on success, non-zero on failure. Results stored into out_res when non-NULL.
int process_file(const char* input_path, const char* output_path, struct Result* out_res) {
    FILE* f = fopen(input_path, "rb");
    if (!f) {
        return -1;
    }

    uint64_t bytes = 0;
    uint64_t sum = 0;
    unsigned char buf[8192];

    for (;;) {
        size_t n = fread(buf, 1, sizeof(buf), f);
        if (n > 0) {
            bytes += (uint64_t)n;
            for (size_t i = 0; i < n; i++) {
                sum = (sum + (uint64_t)buf[i]) & 0xFFFFFFFFFFFFFFFFULL;
            }
        }
        if (n < sizeof(buf)) {
            if (feof(f)) break;
            if (ferror(f)) {
                fclose(f);
                return -2;
            }
        }
    }
    if (fclose(f) != 0) {
        return -3;
    }

    if (write_results_atomic(output_path, bytes, sum) != 0) {
        return -4;
    }

    if (out_res) {
        out_res->bytes_read = bytes;
        out_res->checksum = sum;
    }
    return 0;
}

static int write_bytes_to_file(const char* path, const unsigned char* data, size_t len) {
    FILE* f = fopen(path, "wb");
    if (!f) return -1;
    if (len > 0 && fwrite(data, 1, len, f) != len) {
        fclose(f);
        return -2;
    }
    if (fclose(f) != 0) return -3;
    return 0;
}

int main(void) {
    // Create temp directory path (simple approach: use /tmp; on Windows, current dir)
#ifdef _WIN32
    const char* base = ".";
#else
    const char* base = "/tmp";
#endif

    char in1[512], in2[512], in3[512], in4[512], in5[512];
    char out1[512], out2[512], out3[512], out4[512], out5[512];

    snprintf(in1, sizeof(in1), "%s/lse62_c_in1.bin", base);
    snprintf(in2, sizeof(in2), "%s/lse62_c_in2.bin", base);
    snprintf(in3, sizeof(in3), "%s/lse62_c_in3.bin", base);
    snprintf(in4, sizeof(in4), "%s/lse62_c_in4.bin", base);
    snprintf(in5, sizeof(in5), "%s/lse62_c_in5.bin", base);

    snprintf(out1, sizeof(out1), "%s/lse62_c_out1.txt", base);
    snprintf(out2, sizeof(out2), "%s/lse62_c_out2.txt", base);
    snprintf(out3, sizeof(out3), "%s/lse62_c_out3.txt", base);
    snprintf(out4, sizeof(out4), "%s/lse62_c_out4.txt", base);
    snprintf(out5, sizeof(out5), "%s/lse62_c_out5.txt", base);

    // Prepare test files
    unsigned char empty[] = {};
    unsigned char hello[] = {'h','e','l','l','o'};
    unsigned char zeros[3000]; memset(zeros, 0, sizeof(zeros));
    size_t pattern_len = 120000;
    unsigned char* pattern = (unsigned char*)malloc(pattern_len);
    if (!pattern) return 1;
    for (size_t i = 0; i < pattern_len; i++) pattern[i] = (unsigned char)(i & 0xFF);
    unsigned char mixed[] = {0xFFu, 0x01u, 0x02u, 0x7Fu, 0x80u, 0x55u, 0x00u, 0xAAu};

    if (write_bytes_to_file(in1, empty, sizeof(empty)) != 0) return 1;
    if (write_bytes_to_file(in2, hello, sizeof(hello)) != 0) return 1;
    if (write_bytes_to_file(in3, zeros, sizeof(zeros)) != 0) return 1;
    if (write_bytes_to_file(in4, pattern, pattern_len) != 0) return 1;
    if (write_bytes_to_file(in5, mixed, sizeof(mixed)) != 0) return 1;

    free(pattern);

    struct Result r;

    if (process_file(in1, out1, &r) == 0) {
        printf("Test1 bytes=%" PRIu64 " checksum=%" PRIu64 " -> %s\n", r.bytes_read, r.checksum, out1);
    } else {
        printf("Test1 failed\n");
    }
    if (process_file(in2, out2, &r) == 0) {
        printf("Test2 bytes=%" PRIu64 " checksum=%" PRIu64 " -> %s\n", r.bytes_read, r.checksum, out2);
    } else {
        printf("Test2 failed\n");
    }
    if (process_file(in3, out3, &r) == 0) {
        printf("Test3 bytes=%" PRIu64 " checksum=%" PRIu64 " -> %s\n", r.bytes_read, r.checksum, out3);
    } else {
        printf("Test3 failed\n");
    }
    if (process_file(in4, out4, &r) == 0) {
        printf("Test4 bytes=%" PRIu64 " checksum=%" PRIu64 " -> %s\n", r.bytes_read, r.checksum, out4);
    } else {
        printf("Test4 failed\n");
    }
    if (process_file(in5, out5, &r) == 0) {
        printf("Test5 bytes=%" PRIu64 " checksum=%" PRIu64 " -> %s\n", r.bytes_read, r.checksum, out5);
    } else {
        printf("Test5 failed\n");
    }

    return 0;
}