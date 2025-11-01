#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Reads a file into a buffer, calculates an 8-bit checksum,
 * and writes the number of bytes read and the checksum to an output file.
 *
 * @param inputFilePath The path to the input file.
 * @param outputFilePath The path to the output file.
 * @return 0 on success, -1 on failure.
 */
int processFileAndGetChecksum(const char* inputFilePath, const char* outputFilePath) {
    FILE* inputFile = NULL;
    FILE* outputFile = NULL;
    unsigned char* buffer = NULL;
    long bytesRead = 0;
    int checksum = 0;
    int return_code = -1;

    // 1. Read the contents of a file into a buffer.
    inputFile = fopen(inputFilePath, "rb");
    if (inputFile == NULL) {
        perror("Error: Could not open input file");
        return -1;
    }

    // Determine file size
    fseek(inputFile, 0, SEEK_END);
    bytesRead = ftell(inputFile);
    if (bytesRead < 0) {
        perror("Error: Could not determine file size");
        goto cleanup;
    }
    rewind(inputFile);

    if (bytesRead > 0) {
        // Allocate buffer
        buffer = (unsigned char*)malloc(bytesRead);
        if (buffer == NULL) {
            fprintf(stderr, "Error: Could not allocate memory for buffer.\n");
            goto cleanup;
        }

        // Read file into buffer
        size_t read_result = fread(buffer, 1, bytesRead, inputFile);
        if (read_result != (size_t)bytesRead) {
            fprintf(stderr, "Error: Could not read entire file into buffer.\n");
            goto cleanup;
        }
    }
    
    // 2. Calculate the checksum of the buffer.
    for (long i = 0; i < bytesRead; ++i) {
        checksum = (checksum + buffer[i]) % 256;
    }

    // 3. Save the number of bytes read and the checksum to another file.
    outputFile = fopen(outputFilePath, "w");
    if (outputFile == NULL) {
        perror("Error: Could not open output file");
        goto cleanup;
    }

    if (fprintf(outputFile, "Bytes read: %ld, Checksum: %d", bytesRead, checksum) < 0) {
        fprintf(stderr, "Error: Could not write to output file.\n");
        goto cleanup;
    }

    return_code = 0; // Success

cleanup:
    if (inputFile != NULL) fclose(inputFile);
    if (outputFile != NULL) fclose(outputFile);
    if (buffer != NULL) free(buffer);
    return return_code;
}

// Helper function to create a test file
void createTestFile(const char* filename, const char* content, size_t len) {
    FILE* file = fopen(filename, "wb");
    if (file) {
        fwrite(content, 1, len, file);
        fclose(file);
    }
}

// Helper function to read a file for verification
void readAndPrintTestFile(const char* filename) {
    FILE* file = fopen(filename, "r");
    char buffer[100];
    if (file && fgets(buffer, sizeof(buffer), file)) {
        // Remove trailing newline if present
        buffer[strcspn(buffer, "\n")] = 0;
        printf("%s\n", buffer);
        fclose(file);
    } else {
        printf("Could not read file\n");
    }
}

// Main function with 5 test cases
int main() {
    printf("--- C Test Cases ---\n");

    // Test Case 1: Standard text file
    createTestFile("test1_in.txt", "Hello, World!", 13);
    if (processFileAndGetChecksum("test1_in.txt", "test1_out.txt") == 0) {
        printf("Test Case 1 (Text): ");
        readAndPrintTestFile("test1_out.txt");
    }
    remove("test1_in.txt");
    remove("test1_out.txt");

    // Test Case 2: Binary data
    createTestFile("test2_in.bin", "\xDE\xAD\xBE\xEF", 4);
    if (processFileAndGetChecksum("test2_in.bin", "test2_out.txt") == 0) {
        printf("Test Case 2 (Binary): ");
        readAndPrintTestFile("test2_out.txt");
    }
    remove("test2_in.bin");
    remove("test2_out.txt");

    // Test Case 3: Empty file
    createTestFile("test3_in.txt", "", 0);
    if (processFileAndGetChecksum("test3_in.txt", "test3_out.txt") == 0) {
        printf("Test Case 3 (Empty): ");
        readAndPrintTestFile("test3_out.txt");
    }
    remove("test3_in.txt");
    remove("test3_out.txt");

    // Test Case 4: Non-existent file
    printf("Test Case 4 (Not Found): ");
    processFileAndGetChecksum("non_existent_file.txt", "test4_out.txt");

    // Test Case 5: File with content that results in a zero checksum
    createTestFile("test5_in.bin", "\x01\xFF", 2);
    if (processFileAndGetChecksum("test5_in.bin", "test5_out.txt") == 0) {
        printf("Test Case 5 (Zero Checksum): ");
        readAndPrintTestFile("test5_out.txt");
    }
    remove("test5_in.bin");
    remove("test5_out.txt");

    printf("--- C Tests Complete ---\n");
    return 0;
}