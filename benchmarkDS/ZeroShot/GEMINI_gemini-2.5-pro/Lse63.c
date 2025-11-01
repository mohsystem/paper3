#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#if defined(__GNUC__) || defined(__clang__)
#define PRIu64 "llu"
#else
#include <inttypes.h>
#endif

// Define a reasonable maximum file size to prevent DoS attacks (e.g., 100 MB)
#define MAX_FILE_SIZE (100L * 1024 * 1024)

/**
 * @brief Reads a file, calculates a checksum, and writes the results to another file.
 *
 * @param input_path The path to the input file.
 * @param output_path The path to the output file.
 * @return 0 on success, -1 on failure.
 */
int process_file(const char* input_path, const char* output_path) {
    FILE* in_file = NULL;
    FILE* out_file = NULL;
    unsigned char* buffer = NULL;
    long file_size = 0;
    int return_code = -1;

    // 1. Open input file securely
    in_file = fopen(input_path, "rb");
    if (in_file == NULL) {
        perror("Error opening input file");
        return -1; // No resources to clean up yet
    }

    // 2. Securely determine file size
    if (fseek(in_file, 0, SEEK_END) != 0) {
        perror("Error seeking to end of input file");
        goto cleanup;
    }
    file_size = ftell(in_file);
    if (file_size < 0) {
        perror("Error determining input file size");
        goto cleanup;
    }
    if (fseek(in_file, 0, SEEK_SET) != 0) {
        perror("Error seeking to beginning of input file");
        goto cleanup;
    }

    // Check against max file size
    if (file_size > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: Input file is too large: %ld bytes. Max allowed: %ld bytes.\n", file_size, MAX_FILE_SIZE);
        goto cleanup;
    }

    // 3. Allocate buffer and read file contents
    size_t bytes_read = 0;
    if (file_size > 0) {
        buffer = (unsigned char*)malloc(file_size);
        if (buffer == NULL) {
            perror("Error: Failed to allocate memory for buffer");
            goto cleanup;
        }
        bytes_read = fread(buffer, 1, file_size, in_file);
        if (bytes_read != (size_t)file_size) {
            if (feof(in_file)) {
                 fprintf(stderr, "Error: Unexpected end of file while reading %s.\n", input_path);
            } else if (ferror(in_file)) {
                 perror("Error reading input file");
            }
            goto cleanup;
        }
    }
    
    // 4. Calculate checksum
    uint64_t checksum = 0;
    for (size_t i = 0; i < bytes_read; ++i) {
        checksum += buffer[i];
    }

    if (bytes_read == 0) {
        printf("Info: Buffer is empty for file %s\n", input_path);
    }
    
    // 5. Open output file and write results
    out_file = fopen(output_path, "w");
    if (out_file == NULL) {
        perror("Error opening output file");
        goto cleanup;
    }

    if (fprintf(out_file, "Bytes read: %zu\nChecksum: %" PRIu64 "\n", bytes_read, checksum) < 0) {
        perror("Error writing to output file");
        goto cleanup;
    }

    // If we reached here, all operations were successful
    return_code = 0;

cleanup:
    // Securely close all opened resources
    if (in_file != NULL) {
        fclose(in_file);
    }
    if (out_file != NULL) {
        fclose(out_file);
    }
    if (buffer != NULL) {
        free(buffer);
    }
    return return_code;
}

// Test harness
void run_test(const char* test_name, const char* in, const char* out, int expected) {
    int result = process_file(in, out);
    if (result == expected) {
        printf("  %s: PASSED\n", test_name);
    } else {
        printf("  %s: FAILED (Expected %d, got %d)\n", test_name, expected, result);
    }
}

void create_test_file(const char* path, const char* content, size_t size) {
    FILE* f = fopen(path, "wb");
    if (f) {
        if (size > 0) {
            fwrite(content, 1, size, f);
        }
        fclose(f);
    }
}

void cleanup_test_files(const char* files[], int count) {
    for (int i = 0; i < count; ++i) {
        remove(files[i]);
    }
}

int main() {
    printf("--- Running C Test Cases ---\n");

    // Test Case 1: Empty file
    const char* empty_in = "test_empty_in.txt";
    const char* empty_out = "test_empty_out.txt";
    create_test_file(empty_in, "", 0);
    printf("Test 1: Empty File\n");
    run_test("Test 1", empty_in, empty_out, 0);

    // Test Case 2: ASCII text file
    const char* ascii_in = "test_ascii_in.txt";
    const char* ascii_out = "test_ascii_out.txt";
    const char* ascii_content = "Hello World!";
    create_test_file(ascii_in, ascii_content, strlen(ascii_content));
    printf("Test 2: ASCII File\n");
    run_test("Test 2", ascii_in, ascii_out, 0);

    // Test Case 3: Binary file
    const char* binary_in = "test_binary_in.txt";
    const char* binary_out = "test_binary_out.txt";
    const char binary_content[] = {0x01, 0x02, 0x00, (char)0xFF, (char)0xFE};
    create_test_file(binary_in, binary_content, sizeof(binary_content));
    printf("Test 3: Binary File\n");
    run_test("Test 3", binary_in, binary_out, 0);

    // Test Case 4: Non-existent input file
    const char* non_existent_in = "non_existent_file.txt";
    const char* non_existent_out = "test_no_input_out.txt";
    printf("Test 4: Non-existent Input File\n");
    run_test("Test 4", non_existent_in, non_existent_out, -1);
    
    // Test Case 5: Unwritable output file (e.g., path is a directory, not portable, but /dev/full is on Linux)
    // A more portable failure is an invalid path. On most systems, opening a file in a non-existent dir fails.
    const char* good_in = ascii_in;
    const char* unwritable_out = "non_existent_dir/output.txt";
    printf("Test 5: Unwritable Output Path\n");
    run_test("Test 5", good_in, unwritable_out, -1);
    
    // Cleanup
    const char* files_to_clean[] = {
        empty_in, empty_out, ascii_in, ascii_out, 
        binary_in, binary_out, non_existent_out
    };
    cleanup_test_files(files_to_clean, sizeof(files_to_clean)/sizeof(files_to_clean[0]));

    printf("--- C Test Cases Finished ---\n\n");
    return 0;
}