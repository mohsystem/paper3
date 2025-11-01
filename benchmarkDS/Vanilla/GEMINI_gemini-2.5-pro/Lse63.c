#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Reads the contents of a file into a buffer, calculates a simple checksum,
 * and saves the number of bytes read and the checksum to another file.
 *
 * @param inputFilePath  The path to the input file.
 * @param outputFilePath The path where the results will be saved.
 */
void processFile(const char* inputFilePath, const char* outputFilePath) {
    FILE* inputFile = fopen(inputFilePath, "rb");
    if (inputFile == NULL) {
        perror("Error opening input file");
        return;
    }

    // Determine file size
    fseek(inputFile, 0, SEEK_END);
    long fileSize = ftell(inputFile);
    if (fileSize == -1) {
        perror("Error getting file size");
        fclose(inputFile);
        return;
    }
    rewind(inputFile);

    // Allocate memory for buffer only if file is not empty
    unsigned char* buffer = NULL;
    if (fileSize > 0) {
        buffer = (unsigned char*)malloc(fileSize);
        if (buffer == NULL) {
            fprintf(stderr, "Error allocating memory for file buffer\n");
            fclose(inputFile);
            return;
        }
    }

    // Read file into buffer
    size_t bytesRead = 0;
    if (fileSize > 0) {
        bytesRead = fread(buffer, 1, fileSize, inputFile);
        if (bytesRead != (size_t)fileSize) {
            fprintf(stderr, "Error reading file content\n");
            free(buffer);
            fclose(inputFile);
            return;
        }
    }
    fclose(inputFile);

    int checksum = 0;
    // If the buffer is empty, bytesRead is 0, and the checksum remains 0.
    if (bytesRead > 0) {
        for (size_t i = 0; i < bytesRead; ++i) {
            checksum += buffer[i];
        }
        checksum %= 256;
    }
    
    if (buffer != NULL) {
        free(buffer);
    }

    // Write results to output file
    FILE* outputFile = fopen(outputFilePath, "w");
    if (outputFile == NULL) {
        perror("Error opening output file");
        return;
    }
    fprintf(outputFile, "Bytes read: %ld\nChecksum: %d", (long)bytesRead, checksum);
    fclose(outputFile);
}

// C doesn't have classes. The main function acts as the entry point.
// Lse63 would be the name of the project or source file.
typedef struct {
    const char* filename;
    const unsigned char* content;
    size_t size;
} TestCase;

void run_test_case(int testNum, const TestCase* test) {
    char outputFileName[20];
    sprintf(outputFileName, "test%d_out.txt", testNum);

    printf("--- Test Case %d ---\n", testNum);
    printf("Input File: %s\n", test->filename);

    // 1. Create input file with specified content
    FILE* inFile = fopen(test->filename, "wb");
    if (inFile) {
        if (test->size > 0) {
            fwrite(test->content, 1, test->size, inFile);
        }
        fclose(inFile);
    } else {
        perror("Failed to create input file");
        return;
    }

    // 2. Process the file
    processFile(test->filename, outputFileName);

    // 3. Read and display the output file content for verification
    FILE* outFile = fopen(outputFileName, "r");
    if (outFile) {
        printf("Output from %s:\n", outputFileName);
        char line[100];
        while (fgets(line, sizeof(line), outFile)) {
            printf("%s", line);
        }
        fclose(outFile);
    } else {
        printf("Output file %s was not created or couldn't be opened.\n", outputFileName);
    }

    // 4. Clean up created files
    remove(test->filename);
    remove(outputFileName);
    
    printf("\n---------------------\n\n");
}


int main(void) {
    // Test cases setup
    unsigned char content2[] = {'A'};
    unsigned char content3[] = {'H', 'e', 'l', 'l', 'o'};
    unsigned char content4[] = {0x80, 0x80};
    unsigned char content5[] = {0x00};

    TestCase testCases[] = {
        {"test1_in.txt", NULL, 0},                           // Empty file
        {"test2_in.txt", content2, sizeof(content2)},        // Single character (ASCII 65)
        {"test3_in.txt", content3, sizeof(content3)},        // Simple string
        {"test4_in.txt", content4, sizeof(content4)},        // Non-empty, checksum 0 (128+128=256)
        {"test5_in.txt", content5, sizeof(content5)}         // Null byte
    };

    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);
    for (int i = 0; i < numTestCases; ++i) {
        run_test_case(i + 1, &testCases[i]);
    }

    return 0;
}