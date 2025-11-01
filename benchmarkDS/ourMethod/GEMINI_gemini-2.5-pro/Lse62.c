#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

// Define a reasonable maximum file size to prevent resource exhaustion.
#define MAX_FILE_SIZE (16 * 1024 * 1024) // 16 MB

/**
 * Reads a file, calculates a checksum, and writes results to an output file.
 * Follows secure file handling practices: open-then-validate.
 * @param input_path Path to the input file.
 * @param output_path Path to the output file (must not exist).
 * @return 0 on success, -1 on failure.
 */
int process_file(const char* input_path, const char* output_path) {
    int input_fd = -1;
    int output_fd = -1;
    unsigned char* buffer = NULL;
    int result = -1;

    // --- Read Input File ---
    // Open the resource first, prevent symlink following with O_NOFOLLOW.
    input_fd = open(input_path, O_RDONLY | O_NOFOLLOW);
    if (input_fd < 0) {
        perror("Error opening input file");
        return -1;
    }

    struct stat st;
    // Validate the already-opened handle.
    if (fstat(input_fd, &st) != 0) {
        perror("Error getting file stats");
        goto cleanup;
    }

    // Reject if not a regular file or if size exceeds the policy limit.
    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "Error: Input is not a regular file.\n");
        goto cleanup;
    }
    if (st.st_size > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: Input file size exceeds the limit of %d bytes.\n", MAX_FILE_SIZE);
        goto cleanup;
    }
    
    long file_size = st.st_size;
    long bytes_read_total = 0;
    unsigned long checksum = 0;

    if (file_size > 0) {
        // Allocate buffer based on validated file size.
        buffer = (unsigned char*)malloc(file_size);
        if (buffer == NULL) {
            perror("Error allocating memory for buffer");
            goto cleanup;
        }

        // Read file content into buffer, ensuring read respects buffer boundaries.
        ssize_t bytes_read = read(input_fd, buffer, file_size);
        if (bytes_read < 0) {
            perror("Error reading from input file");
            goto cleanup;
        }
        if (bytes_read != file_size) {
            fprintf(stderr, "Error: Incomplete read. Expected %ld, got %zd bytes.\n", file_size, bytes_read);
            goto cleanup;
        }
        bytes_read_total = bytes_read;
        
        // --- Calculate Checksum ---
        for (long i = 0; i < bytes_read_total; ++i) {
            checksum += buffer[i];
        }
    }

    // --- Write Output File ---
    // Create output file securely with restrictive permissions (0600).
    // O_EXCL fails if the file already exists, preventing overwrites.
    output_fd = open(output_path, O_WRONLY | O_CREAT | O_EXCL, 0600);
    if (output_fd < 0) {
        perror("Error creating output file");
        goto cleanup;
    }

    char output_buffer[128];
    // Use snprintf for safe string formatting to prevent buffer overflows.
    int len = snprintf(output_buffer, sizeof(output_buffer), "Bytes read: %ld\nChecksum: %lu\n", bytes_read_total, checksum);
    if (len < 0 || (size_t)len >= sizeof(output_buffer)) {
        fprintf(stderr, "Error formatting output string.\n");
        goto cleanup;
    }

    if (write(output_fd, output_buffer, len) != len) {
        perror("Error writing to output file");
        goto cleanup;
    }

    printf("Successfully processed '%s', results in '%s'\n", input_path, output_path);
    result = 0; // Success

cleanup:
    if (buffer != NULL) free(buffer);
    if (input_fd >= 0) close(input_fd);
    if (output_fd >= 0) close(output_fd);
    return result;
}

void run_test(const char* name, const char* content, size_t content_len, int should_succeed) {
    printf("--- Test Case: %s ---\n", name);
    const char* input_file = "test_input.txt";
    const char* output_file = "test_output.txt";
    
    remove(input_file);
    remove(output_file);
    
    if (content != NULL) {
        FILE* f = fopen(input_file, "wb");
        if (f) {
            if (content_len > 0) fwrite(content, 1, content_len, f);
            fclose(f);
        } else {
            perror("Failed to create test input file");
            return;
        }
    } else {
        printf("Testing with non-existent input file.\n");
    }
    
    int result = process_file(input_file, output_file);
    if (should_succeed) {
        if (result == 0) printf("PASS: Process succeeded as expected.\n");
        else printf("FAIL: Process failed unexpectedly.\n");
    } else {
        if (result != 0) printf("PASS: Process failed as expected.\n");
        else printf("FAIL: Process succeeded unexpectedly.\n");
    }
    
    remove(input_file);
    remove(output_file);
    printf("\n");
}

int main() {
    run_test("Normal file", "Hello World!", 12, 1);
    run_test("Empty file", "", 0, 1);
    const char null_content[] = {'a', '\0', 'b', '\0', 'c'};
    run_test("File with null bytes", null_content, sizeof(null_content), 1);
    run_test("Non-existent file", NULL, 0, 0);

    printf("--- Test Case: Output file exists ---\n");
    const char* input_file = "test_input.txt";
    const char* output_file = "test_output.txt";
    remove(input_file);
    remove(output_file);
    FILE* f_in = fopen(input_file, "w"); if(f_in) fclose(f_in);
    FILE* f_out = fopen(output_file, "w"); if(f_out) fclose(f_out);
    int result = process_file(input_file, output_file);
    if (result != 0) {
        printf("PASS: Process failed as expected because output file exists.\n");
    } else {
        printf("FAIL: Process succeeded but should have failed.\n");
    }
    remove(input_file);
    remove(output_file);
    printf("\n");
    
    return 0;
}