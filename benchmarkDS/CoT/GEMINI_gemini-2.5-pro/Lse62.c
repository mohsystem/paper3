#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Reads a file into a buffer, calculates a checksum, and saves the byte count
 * and checksum to an output file.
 *
 * @param input_file_path The path to the input file.
 * @param output_file_path The path to the output file where results will be saved.
 * @return 0 on success, -1 on failure.
 */
int process_file_and_calculate_checksum(const char* input_file_path, const char* output_file_path) {
    FILE *f_in = NULL;
    FILE *f_out = NULL;
    unsigned char *buffer = NULL;
    long file_size = -1;
    int return_code = -1;

    // 1. Read the contents of the file into a buffer.
    // Open in binary read mode "rb" for portability.
    f_in = fopen(input_file_path, "rb");
    if (f_in == NULL) {
        perror("Error opening input file");
        return -1;
    }

    // Determine file size
    fseek(f_in, 0, SEEK_END);
    file_size = ftell(f_in);
    if (file_size == -1) {
        perror("Error determining file size");
        goto cleanup;
    }
    rewind(f_in);

    // Handle empty file case
    if (file_size == 0) {
        buffer = NULL; // No need to allocate
    } else {
        // Allocate memory for the buffer
        buffer = (unsigned char *)malloc(file_size);
        if (buffer == NULL) {
            fprintf(stderr, "Error: Failed to allocate memory for the buffer.\n");
            goto cleanup;
        }

        // Read the file into the buffer
        size_t bytes_read = fread(buffer, 1, file_size, f_in);
        if (bytes_read != (size_t)file_size) {
            fprintf(stderr, "Error: Failed to read the entire file.\n");
            goto cleanup;
        }
    }

    // 2. Calculate the checksum of the buffer.
    // Using long long to prevent overflow.
    long long checksum = 0;
    for (long i = 0; i < file_size; ++i) {
        checksum += buffer[i];
    }

    // 3. Save the number of bytes read and the checksum to a file.
    f_out = fopen(output_file_path, "w");
    if (f_out == NULL) {
        perror("Error opening output file");
        goto cleanup;
    }

    fprintf(f_out, "Bytes: %ld, Checksum: %lld", file_size, checksum);
    
    // If we reached here, everything was successful.
    return_code = 0;

cleanup:
    // This block ensures all resources are freed/closed regardless of success or failure.
    if (f_in != NULL) {
        fclose(f_in);
    }
    if (f_out != NULL) {
        fclose(f_out);
    }
    if (buffer != NULL) {
        free(buffer);
    }
    return return_code;
}

// Helper to create a test file
void create_test_file(const char* filename, const char* content, size_t len) {
    FILE* f = fopen(filename, "wb");
    if (f) {
        if (len > 0) {
            fwrite(content, 1, len, f);
        }
        fclose(f);
    }
}

// Main function for test cases
int main() {
    // --- Test Cases ---

    // Test Case 1: A normal text file.
    printf("--- Test Case 1: Normal File ---\n");
    const char* input_file_1 = "test_input_1.txt";
    const char* output_file_1 = "test_output_1.txt";
    create_test_file(input_file_1, "Hello", 5);
    int success_1 = process_file_and_calculate_checksum(input_file_1, output_file_1);
    printf("Test 1 Success: %s\n", success_1 == 0 ? "true" : "false"); // Expected: true
    if (success_1 == 0) {
        // Optional: print output file content for verification
        FILE* f = fopen(output_file_1, "r");
        if (f) { char buf[100]; fgets(buf, 100, f); printf("Output: %s\n", buf); fclose(f); } // Expected: Bytes: 5, Checksum: 500
    }
    remove(input_file_1);
    remove(output_file_1);
    printf("\n");

    // Test Case 2: An empty file.
    printf("--- Test Case 2: Empty File ---\n");
    const char* input_file_2 = "test_input_2.txt";
    const char* output_file_2 = "test_output_2.txt";
    create_test_file(input_file_2, "", 0);
    int success_2 = process_file_and_calculate_checksum(input_file_2, output_file_2);
    printf("Test 2 Success: %s\n", success_2 == 0 ? "true" : "false"); // Expected: true
    if (success_2 == 0) {
        FILE* f = fopen(output_file_2, "r");
        if (f) { char buf[100]; fgets(buf, 100, f); printf("Output: %s\n", buf); fclose(f); } // Expected: Bytes: 0, Checksum: 0
    }
    remove(input_file_2);
    remove(output_file_2);
    printf("\n");

    // Test Case 3: A binary file.
    printf("--- Test Case 3: Binary File ---\n");
    const char* input_file_3 = "test_input_3.bin";
    const char* output_file_3 = "test_output_3.txt";
    char binary_data[] = {0x01, (char)0x80, (char)0xFF};
    create_test_file(input_file_3, binary_data, 3);
    int success_3 = process_file_and_calculate_checksum(input_file_3, output_file_3);
    printf("Test 3 Success: %s\n", success_3 == 0 ? "true" : "false"); // Expected: true
    if (success_3 == 0) {
        FILE* f = fopen(output_file_3, "r");
        if (f) { char buf[100]; fgets(buf, 100, f); printf("Output: %s\n", buf); fclose(f); } // Expected: Bytes: 3, Checksum: 384
    }
    remove(input_file_3);
    remove(output_file_3);
    printf("\n");

    // Test Case 4: Non-existent input file.
    printf("--- Test Case 4: Non-existent Input File ---\n");
    int success_4 = process_file_and_calculate_checksum("non_existent_file.txt", "test_output_4.txt");
    printf("Test 4 Success: %s\n", success_4 == 0 ? "true" : "false"); // Expected: false
    remove("test_output_4.txt");
    printf("\n");

    // Test Case 5: Unwritable output file path.
    printf("--- Test Case 5: Unwritable Output Path ---\n");
    const char* input_file_5 = "test_input_5.txt";
    create_test_file(input_file_5, "test", 4);
    int success_5 = process_file_and_calculate_checksum(input_file_5, "non_existent_dir/test_output_5.txt");
    printf("Test 5 Success: %s\n", success_5 == 0 ? "true" : "false"); // Expected: false
    remove(input_file_5);
    printf("\n");

    return 0;
}