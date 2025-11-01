
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define MAX_BUFFER_SIZE (1024 * 1024) // 1MB limit

typedef struct {
    size_t bytesRead;
    unsigned long long checksum;
} ChecksumResult;

unsigned long long calculateChecksum(const unsigned char* buffer, size_t size) {
    unsigned long long checksum = 0;
    for (size_t i = 0; i < size; i++) {
        checksum += (unsigned long long)buffer[i];
    }
    return checksum;
}

ChecksumResult processFile(const char* inputFilePath, const char* outputFilePath) {
    ChecksumResult result = {0, 0};
    
    if (inputFilePath == NULL || outputFilePath == NULL) {
        fprintf(stderr, "Error: File paths cannot be NULL\\n");
        return result;
    }
    
    // Check if file exists and get size
    struct stat st;
    if (stat(inputFilePath, &st) != 0) {
        fprintf(stderr, "Error: Cannot access input file: %s\\n", inputFilePath);
        return result;
    }
    
    if (st.st_size > MAX_BUFFER_SIZE) {
        fprintf(stderr, "Error: File too large. Maximum size: %d\\n", MAX_BUFFER_SIZE);
        return result;
    }
    
    // Open input file
    FILE* inputFile = fopen(inputFilePath, "rb");
    if (inputFile == NULL) {
        fprintf(stderr, "Error: Cannot open input file: %s\\n", inputFilePath);
        return result;
    }
    
    // Allocate buffer
    size_t fileSize = (size_t)st.st_size;
    unsigned char* buffer = (unsigned char*)malloc(fileSize);
    if (buffer == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        fclose(inputFile);
        return result;
    }
    
    // Read file into buffer
    size_t bytesRead = fread(buffer, 1, fileSize, inputFile);
    fclose(inputFile);
    
    if (bytesRead != fileSize) {
        fprintf(stderr, "Error: Failed to read complete file\\n");
        free(buffer);
        return result;
    }
    
    // Calculate checksum
    unsigned long long checksum = calculateChecksum(buffer, bytesRead);
    free(buffer);
    
    // Save results to output file
    FILE* outputFile = fopen(outputFilePath, "w");
    if (outputFile == NULL) {
        fprintf(stderr, "Error: Cannot open output file: %s\\n", outputFilePath);
        result.bytesRead = bytesRead;
        result.checksum = checksum;
        return result;
    }
    
    fprintf(outputFile, "Bytes Read: %zu\\n", bytesRead);
    fprintf(outputFile, "Checksum: %llu\\n", checksum);
    if (checksum == 0) {
        fprintf(outputFile, "Status: Buffer is empty or checksum is zero\\n");
    }
    fclose(outputFile);
    
    result.bytesRead = bytesRead;
    result.checksum = checksum;
    return result;
}

void createTestFile(const char* filename, const char* content) {
    FILE* file = fopen(filename, "w");
    if (file != NULL) {
        fputs(content, file);
        fclose(file);
    } else {
        fprintf(stderr, "Error creating test file: %s\\n", filename);
    }
}

int main() {
    // Test case 1: Normal file
    createTestFile("test1.txt", "Hello World");
    processFile("test1.txt", "output1.txt");
    
    // Test case 2: Empty file
    createTestFile("test2.txt", "");
    processFile("test2.txt", "output2.txt");
    
    // Test case 3: File with numbers
    createTestFile("test3.txt", "1234567890");
    processFile("test3.txt", "output3.txt");
    
    // Test case 4: Binary-like content
    createTestFile("test4.txt", "\\0\\0\\0\\0");
    processFile("test4.txt", "output4.txt");
    
    // Test case 5: Larger text
    createTestFile("test5.txt", "The quick brown fox jumps over the lazy dog");
    processFile("test5.txt", "output5.txt");
    
    printf("All test cases executed. Check output files.\\n");
    
    return 0;
}
