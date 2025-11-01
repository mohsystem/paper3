#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Reads the contents of a file into a buffer, calculates the checksum of the buffer,
 * and saves the number of bytes read and the checksum to another file.
 *
 * @param inputFilePath  The path to the file to be read.
 * @param outputFilePath The path to the file where results will be saved.
 */
void processFile(const char* inputFilePath, const char* outputFilePath) {
    long long checksum = 0;
    size_t bytesRead = 0;
    unsigned char* buffer = NULL;

    FILE* inputFile = fopen(inputFilePath, "rb");
    if (inputFile != NULL) {
        // Seek to the end of the file to determine its size
        fseek(inputFile, 0, SEEK_END);
        long fileSize = ftell(inputFile);
        rewind(inputFile);

        if (fileSize > 0) {
            buffer = (unsigned char*)malloc(fileSize);
            if (buffer == NULL) {
                fprintf(stderr, "Error: Memory allocation failed.\n");
                fclose(inputFile);
                return;
            }

            bytesRead = fread(buffer, 1, fileSize, inputFile);
            if (bytesRead != (size_t)fileSize) {
                fprintf(stderr, "Warning: Could not read the entire file.\n");
            }
            
            // Calculate checksum by summing all bytes read
            for (size_t i = 0; i < bytesRead; ++i) {
                checksum += buffer[i];
            }
        }
        fclose(inputFile);
    }
    // If inputFile is NULL (file not found) or file is empty,
    // bytesRead and checksum will correctly be 0.

    FILE* outputFile = fopen(outputFilePath, "w");
    if (outputFile == NULL) {
        fprintf(stderr, "Error: Could not open output file %s.\n", outputFilePath);
        free(buffer);
        return;
    }

    fprintf(outputFile, "Bytes read: %zu\n", bytesRead);
    fprintf(outputFile, "Checksum: %lld\n", checksum);
    fclose(outputFile);

    free(buffer);
}

// Helper function to create a test file
void createTestFile(const char* path, const unsigned char* content, size_t size) {
    FILE* file = fopen(path, "wb");
    if (file) {
        if (size > 0 && content != NULL) {
            fwrite(content, 1, size, file);
        }
        fclose(file);
    }
}

// Helper function to read and print the content of an output file for verification
void printOutputFile(const char* path) {
    FILE* file = fopen(path, "r");
    if (file) {
        printf("--- Contents of %s ---\n", path);
        char c;
        while ((c = fgetc(file)) != EOF) {
            putchar(c);
        }
        printf("------------------------------------\n");
        fclose(file);
    } else {
        fprintf(stderr, "Could not read output file %s\n", path);
    }
}

int main(void) {
    // Test Case 1: A non-empty text file
    const char* test1_in = "test1_in.txt";
    const char* test1_out = "test1_out.txt";
    const unsigned char content1[] = "Hello World!";
    createTestFile(test1_in, content1, strlen((const char*)content1));
    processFile(test1_in, test1_out);
    printOutputFile(test1_out);

    // Test Case 2: An empty file
    const char* test2_in = "test2_in.txt";
    const char* test2_out = "test2_out.txt";
    createTestFile(test2_in, NULL, 0);
    processFile(test2_in, test2_out);
    printOutputFile(test2_out);

    // Test Case 3: A file with binary data
    const char* test3_in = "test3_in.bin";
    const char* test3_out = "test3_out.bin";
    const unsigned char binaryData[] = {0xDE, 0xAD, 0xBE, 0xEF};
    createTestFile(test3_in, binaryData, sizeof(binaryData));
    processFile(test3_in, test3_out);
    printOutputFile(test3_out);

    // Test Case 4: A file with a single byte
    const char* test4_in = "test4_in.txt";
    const char* test4_out = "test4_out.txt";
    const unsigned char content4[] = "A";
    createTestFile(test4_in, content4, 1);
    processFile(test4_in, test4_out);
    printOutputFile(test4_out);

    // Test Case 5: A file that doesn't exist
    const char* test5_in = "non_existent_file.txt";
    const char* test5_out = "test5_out.txt";
    remove(test5_in);
    processFile(test5_in, test5_out);
    printOutputFile(test5_out);

    return 0;
}