
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define MAX_FILE_SIZE (10 * 1024 * 1024) // 10MB limit

typedef struct {
    size_t bytesRead;
    unsigned long checksum;
} ChecksumResult;

int fileExists(const char* path) {
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

size_t getFileSize(const char* path) {
    struct stat buffer;
    if (stat(path, &buffer) != 0) {
        return 0;
    }
    return (size_t)buffer.st_size;
}

ChecksumResult processFile(const char* inputPath, const char* outputPath) {
    ChecksumResult result = {0, 0};
    FILE* inputFile = NULL;
    FILE* outputFile = NULL;
    unsigned char* buffer = NULL;
    
    if (inputPath == NULL || outputPath == NULL || 
        strlen(inputPath) == 0 || strlen(outputPath) == 0) {
        fprintf(stderr, "Error: File paths cannot be NULL or empty\\n");
        return result;
    }
    
    // Validate input file
    if (!fileExists(inputPath)) {
        fprintf(stderr, "Error: Input file does not exist\\n");
        return result;
    }
    
    // Check file size
    size_t fileSize = getFileSize(inputPath);
    if (fileSize > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: File size exceeds maximum allowed size\\n");
        return result;
    }
    
    // Open input file
    inputFile = fopen(inputPath, "rb");
    if (inputFile == NULL) {
        fprintf(stderr, "Error: Cannot open input file\\n");
        return result;
    }
    
    // Allocate buffer
    buffer = (unsigned char*)malloc(fileSize);
    if (buffer == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        fclose(inputFile);
        return result;
    }
    
    // Read file contents
    size_t bytesRead = fread(buffer, 1, fileSize, inputFile);
    fclose(inputFile);
    
    // Calculate checksum
    unsigned long checksum = 0;
    for (size_t i = 0; i < bytesRead; i++) {
        checksum = (checksum + buffer[i]) & 0xFFFFFFFFUL; // Prevent overflow
    }
    
    free(buffer);
    
    // Write results to output file
    outputFile = fopen(outputPath, "w");
    if (outputFile == NULL) {
        fprintf(stderr, "Error: Cannot open output file\\n");
        return result;
    }
    
    fprintf(outputFile, "Bytes Read: %zu\\n", bytesRead);
    fprintf(outputFile, "Checksum: %lu\\n", checksum);
    fprintf(outputFile, "Buffer Empty: %s\\n", (checksum == 0) ? "true" : "false");
    fclose(outputFile);
    
    result.bytesRead = bytesRead;
    result.checksum = checksum;
    return result;
}

int main() {
    // Test case 1: Normal file with content
    {
        FILE* f = fopen("test1.txt", "w");
        if (f) {
            fprintf(f, "Hello World");
            fclose(f);
        }
        ChecksumResult result = processFile("test1.txt", "output1.txt");
        printf("Test 1 - Bytes: %zu, Checksum: %lu\\n", result.bytesRead, result.checksum);
    }
    
    // Test case 2: Empty file
    {
        FILE* f = fopen("test2.txt", "w");
        if (f) {
            fclose(f);
        }
        ChecksumResult result = processFile("test2.txt", "output2.txt");
        printf("Test 2 - Bytes: %zu, Checksum: %lu\\n", result.bytesRead, result.checksum);
    }
    
    // Test case 3: File with numbers
    {
        FILE* f = fopen("test3.txt", "w");
        if (f) {
            fprintf(f, "1234567890");
            fclose(f);
        }
        ChecksumResult result = processFile("test3.txt", "output3.txt");
        printf("Test 3 - Bytes: %zu, Checksum: %lu\\n", result.bytesRead, result.checksum);
    }
    
    // Test case 4: Binary content
    {
        FILE* f = fopen("test4.txt", "wb");
        if (f) {
            unsigned char data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
            fwrite(data, 1, sizeof(data), f);
            fclose(f);
        }
        ChecksumResult result = processFile("test4.txt", "output4.txt");
        printf("Test 4 - Bytes: %zu, Checksum: %lu\\n", result.bytesRead, result.checksum);
    }
    
    // Test case 5: Non-existent file
    {
        ChecksumResult result = processFile("nonexistent.txt", "output5.txt");
        printf("Test 5 - Bytes: %zu, Checksum: %lu\\n", result.bytesRead, result.checksum);
    }
    
    return 0;
}
