#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// For creating/removing directories in test cases (platform-specific)
#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#define RMDIR(path) _rmdir(path)
#else
#include <sys/stat.h>
#include <unistd.h>
#define MKDIR(path) mkdir(path, 0755)
#define RMDIR(path) rmdir(path)
#endif

#define BUFFER_SIZE 4096

/**
 * @brief Reads a file, calculates its checksum, and writes results to an output file.
 *
 * This function handles file resources carefully, checking all return codes and
 * using a goto-based cleanup to ensure files are always closed.
 *
 * @param inputFilePath Path to the input file.
 * @param outputFilePath Path to the output file.
 * @return 0 on success, -1 on failure.
 */
int processFile(const char* inputFilePath, const char* outputFilePath) {
    FILE *inFile = NULL;
    FILE *outFile = NULL;
    // Use unsigned char buffer to avoid implementation-defined behavior with signed char.
    unsigned char buffer[BUFFER_SIZE];
    unsigned long long numberOfBytesRead = 0;
    unsigned long long checksum = 0;
    int ret_val = -1; // Default to failure

    inFile = fopen(inputFilePath, "rb");
    if (inFile == NULL) {
        perror("Error opening input file");
        return -1; // No resources to clean up, just exit.
    }

    size_t bytesReadInChunk;
    while ((bytesReadInChunk = fread(buffer, 1, BUFFER_SIZE, inFile)) > 0) {
        numberOfBytesRead += bytesReadInChunk;
        for (size_t i = 0; i < bytesReadInChunk; ++i) {
            checksum += buffer[i];
        }
    }

    if (ferror(inFile)) {
        perror("Error reading from input file");
        goto cleanup; // Use goto for centralized cleanup
    }

    if (checksum == 0) {
        printf("Buffer is empty for file: %s\n", inputFilePath);
    }
    
    outFile = fopen(outputFilePath, "w");
    if (outFile == NULL) {
        perror("Error opening output file");
        goto cleanup;
    }

    if (fprintf(outFile, "Bytes read: %llu\n", numberOfBytesRead) < 0) {
        perror("Error writing bytes read to output file");
        goto cleanup;
    }
    if (fprintf(outFile, "Checksum: %llu\n", checksum) < 0) {
        perror("Error writing checksum to output file");
        goto cleanup;
    }

    ret_val = 0; // Set to success only if all operations succeed

cleanup:
    if (inFile != NULL && fclose(inFile) != 0) {
        perror("Error closing input file");
        ret_val = -1; // An error on close should be considered a failure
    }
    if (outFile != NULL && fclose(outFile) != 0) {
        perror("Error closing output file");
        ret_val = -1;
    }
    return ret_val;
}

void setupTestFiles() {
    FILE* f = NULL;
    f = fopen("test1.txt", "wb"); if (f) fclose(f); // Empty
    f = fopen("test2.txt", "w"); if (f) { fputs("Hello", f); fclose(f); } // Checksum: 500
    f = fopen("test3.bin", "wb"); if (f) { fwrite("\x01\x02\xFF", 1, 3, f); fclose(f); } // Checksum: 258
}

void runTestCase(const char* inputFile, const char* outputFile) {
    printf("Processing %s -> %s\n", inputFile, outputFile);
    int result = processFile(inputFile, outputFile);
    printf("Result: %s\n", (result == 0) ? "SUCCESS" : "FAILURE");
    if (result == 0) {
        FILE* f = fopen(outputFile, "r");
        if (f) {
            printf("Output content:\n---\n");
            int c;
            while ((c = fgetc(f)) != EOF) {
                putchar((char)c);
            }
            printf("---\n");
            fclose(f);
        } else {
            perror("Could not read output file for verification");
        }
    }
}

void cleanupTestFiles() {
    remove("test1.txt");
    remove("test2.txt");
    remove("test3.bin");
    remove("output1.txt");
    remove("output2.txt");
    remove("output3.txt");
    RMDIR("output5.txt"); // Remove directory created for test case 5
}

int main() {
    setupTestFiles();

    printf("--- Running C Test Cases ---\n");

    // Test Case 1: Empty file
    printf("\nTest Case 1: Empty File\n");
    runTestCase("test1.txt", "output1.txt");

    // Test Case 2: Small text file
    printf("\nTest Case 2: Small Text File\n");
    runTestCase("test2.txt", "output2.txt");
    
    // Test Case 3: Small binary file
    printf("\nTest Case 3: Small Binary File\n");
    runTestCase("test3.bin", "output3.txt");
    
    // Test Case 4: Non-existent input file
    printf("\nTest Case 4: Non-existent Input File\n");
    runTestCase("nonexistent.txt", "output4.txt");

    // Test Case 5: Unwritable output file (simulated by using a directory)
    printf("\nTest Case 5: Unwritable Output File\n");
    if (MKDIR("output5.txt") == 0) {
        runTestCase("test1.txt", "output5.txt");
    } else {
        perror("Could not create directory for Test Case 5");
    }
    
    printf("\n--- Cleaning up test files ---\n");
    cleanupTestFiles();
    printf("Cleanup complete.\n");

    return 0;
}