#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

// POSIX headers
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

static const off_t MAX_FILE_SIZE = 16 * 1024 * 1024; // 16 MB limit

int process_file(const char* input_path, const char* output_path) {
    int ret = -1; // Default to error
    int fd = -1;
    int out_fd = -1;
    char* buffer = NULL;
    char* temp_output_path = NULL;

    fd = open(input_path, O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
    if (fd < 0) {
        perror("Error opening input file");
        goto cleanup;
    }

    struct stat st;
    if (fstat(fd, &st) != 0) {
        perror("Error getting file stats");
        goto cleanup;
    }

    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "Error: Input path is not a regular file.\n");
        goto cleanup;
    }

    if (st.st_size > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: File size %ld exceeds maximum limit of %ld bytes.\n", (long)st.st_size, (long)MAX_FILE_SIZE);
        goto cleanup;
    }

    ssize_t bytes_read = 0;
    if (st.st_size > 0) {
        buffer = malloc(st.st_size);
        if (!buffer) {
            perror("Error allocating memory for file content");
            goto cleanup;
        }

        char* p = buffer;
        ssize_t remaining = st.st_size;
        while(remaining > 0) {
            ssize_t current_read = read(fd, p, remaining);
            if(current_read < 0) {
                perror("Error reading from file");
                goto cleanup;
            }
            if(current_read == 0) break; // EOF
            bytes_read += current_read;
            p += current_read;
            remaining -= current_read;
        }

        if (bytes_read != st.st_size) {
            fprintf(stderr, "Error: Could not read the entire file.\n");
            goto cleanup;
        }
    }
    
    uint32_t checksum = 0;
    for (ssize_t i = 0; i < bytes_read; ++i) {
        checksum += (unsigned char)buffer[i];
    }
    
    size_t temp_path_len = strlen(output_path) + 5; // ".tmp\0"
    temp_output_path = malloc(temp_path_len);
    if (!temp_output_path) {
        perror("Error allocating memory for temp path");
        goto cleanup;
    }
    snprintf(temp_output_path, temp_path_len, "%s.tmp", output_path);

    out_fd = open(temp_output_path, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (out_fd < 0) {
        perror("Error creating temporary output file");
        goto cleanup;
    }
    
    char out_buffer[256];
    int len = snprintf(out_buffer, sizeof(out_buffer), "Bytes read: %zd\nChecksum: %u\n", bytes_read, checksum);

    if (write(out_fd, out_buffer, len) != len) {
        perror("Error writing to temporary file");
        goto cleanup;
    }
    
    if (fsync(out_fd) != 0) {
        perror("Error syncing temporary file to disk");
        goto cleanup;
    }
    
    close(out_fd);
    out_fd = -1; // Prevent double close in cleanup

    if (rename(temp_output_path, output_path) != 0) {
        perror("Error renaming temporary file");
        unlink(temp_output_path); // remove temp file on rename failure
        goto cleanup;
    }
    
    ret = 0; // Success

cleanup:
    if (fd >= 0) close(fd);
    if (buffer) free(buffer);
    if (out_fd >= 0) {
        close(out_fd);
        if(temp_output_path) unlink(temp_output_path); // If we failed, remove temp file
    }
    if (temp_output_path) free(temp_output_path);
    return ret;
}

void create_test_file(const char* path, const char* content, size_t len) {
    FILE* f = fopen(path, "wb");
    if (f) {
        if (len > 0) {
            fwrite(content, 1, len, f);
        }
        fclose(f);
    }
}

void run_test_case(int test_num, const char* name, const char* content, size_t len, bool create, bool should_succeed) {
    printf("\n--- Test Case %d: %s ---\n", test_num, name);
    char input_path[256];
    char output_path[256];
    snprintf(input_path, sizeof(input_path), "%s.txt", name);
    snprintf(output_path, sizeof(output_path), "%s.out", name);

    remove(input_path);
    remove(output_path);
    if (create) {
        create_test_file(input_path, content, len);
    }

    printf("Input: %s, Output: %s\n", input_path, output_path);
    int result_code = process_file(input_path, output_path);
    bool success = (result_code == 0);

    printf("Execution result: %s\n", success ? "SUCCESS" : "FAILURE");
    printf("Expected result:  %s\n", should_succeed ? "SUCCESS" : "FAILURE");

    if (success == should_succeed) {
        printf("Test PASSED.\n");
    } else {
        printf("Test FAILED.\n");
    }

    remove(input_path);
    remove(output_path);
}

int main() {
    run_test_case(1, "c_empty_file", "", 0, true, true);
    run_test_case(2, "c_hello_file", "hello world", 11, true, true);
    char binary_content[] = {(char)0xff, (char)0xfe, (char)0x01, (char)0x00};
    run_test_case(3, "c_binary_file", binary_content, sizeof(binary_content), true, true);
    char nulls_content[] = "\0\0\0";
    run_test_case(4, "c_nulls_file", nulls_content, 3, true, true);
    run_test_case(5, "c_non_existent_file", "", 0, false, false);

    return 0;
}