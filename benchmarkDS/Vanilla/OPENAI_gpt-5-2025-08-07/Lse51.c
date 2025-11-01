#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

size_t write_text_file(const char* path, const char* text) {
    // Create parent dir if needed is non-trivial portably; assume /tmp exists.
    FILE* f = fopen(path, "wb");
    if (!f) {
        perror("write_text_file fopen");
        return 0;
    }
    size_t len = strlen(text);
    size_t n = fwrite(text, 1, len, f);
    if (fclose(f) != 0) {
        perror("write_text_file fclose");
    }
    return n;
}

char* read_text_file(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) {
        // perror("read_text_file fopen");
        return NULL;
    }
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    long size = ftell(f);
    if (size < 0) { fclose(f); return NULL; }
    rewind(f);
    char* buf = (char*)malloc((size_t)size + 1);
    if (!buf) { fclose(f); return NULL; }
    size_t n = fread(buf, 1, (size_t)size, f);
    buf[n] = '\0';
    fclose(f);
    return buf;
}

int extract_tar_to_tmp(const char* tar_path) {
    // Not implemented in plain C without external libraries.
    // Return 0 to indicate not supported.
    (void)tar_path;
    return 0;
}

static void ensure_dir_tmp_unpack() {
    struct stat st;
    if (stat("/tmp/unpack", &st) != 0) {
        if (mkdir("/tmp/unpack", 0777) != 0 && errno != EEXIST) {
            perror("mkdir /tmp/unpack");
        }
    }
}

int main(void) {
    ensure_dir_tmp_unpack();

    // Test 1: write/read a text file
    size_t written = write_text_file("/tmp/c_sample.txt", "Hello from C stdio!");
    char* read = read_text_file("/tmp/c_sample.txt");
    printf("Test1 - written: %zu, read: %s\n", written, read ? read : "(null)");
    free(read);

    // Test 2: attempt to extract plain tar (stub)
    int ok_plain = extract_tar_to_tmp("/tmp/c_plain.tar");
    printf("Test2 - extract plain tar supported? %s\n", ok_plain ? "yes" : "no");

    // Test 3: attempt to extract gzip tar (stub)
    int ok_gz = extract_tar_to_tmp("/tmp/c_gzip.tar.gz");
    printf("Test3 - extract gzip tar supported? %s\n", ok_gz ? "yes" : "no");

    // Test 4: attempt to extract bzip2 tar (stub)
    int ok_bz2 = extract_tar_to_tmp("/tmp/c_bzip2.tar.bz2");
    printf("Test4 - extract bzip2 tar supported? %s\n", ok_bz2 ? "yes" : "no");

    // Test 5: read non-existent file
    char* missing = read_text_file("/tmp/c_missing.txt");
    printf("Test5 - missing read result: %s\n", missing ? missing : "(null)");
    free(missing);

    return 0;
}