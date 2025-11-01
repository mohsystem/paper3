
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int bytesRead;
    long long checksum;
} ChecksumResult;

ChecksumResult processFile(const char* inputFilePath, const char* outputFilePath) {
    ChecksumResult result = {0, 0};
    FILE *inputFile, *outputFile;
    unsigned char *buffer;
    long fileSize;
    
    // Open and read input file
    inputFile = fopen(inputFilePath, "rb");
    if (!inputFile) {
        fprintf(stderr, "Error opening input file\\n");
        return result;
    }
    
    // Get file size
    fseek(inputFile, 0, SEEK_END);
    fileSize = ftell(inputFile);
    fseek(inputFile, 0, SEEK_SET);
    
    // Allocate buffer and read file
    buffer = (unsigned char*)malloc(fileSize);
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed\\n");
        fclose(inputFile);
        return result;
    }
    
    result.bytesRead = fread(buffer, 1, fileSize, inputFile);
    fclose(inputFile);
    
    // Calculate checksum
    result.checksum = 0;
    for (int i = 0; i < result.bytesRead; i++) {
        result.checksum += buffer[i];
    }
    
    // Check if checksum is 0 (buffer is empty)
    if (result.checksum == 0) {
        printf("Buffer is empty or checksum is 0\\n");
    }
    
    // Save results to output file
    outputFile = fopen(outputFilePath, "w");
    if (outputFile) {
        fprintf(outputFile, "Bytes Read: %d\\n", result.bytesRead);
        fprintf(outputFile, "Checksum: %lld\\n", result.checksum);
        fclose(outputFile);
    }
    
    free(buffer);
    return result;
}

int main() {
    // Test case 1: File with text content
    {
        FILE *test = fopen("test1.txt", "w");
        fprintf(test, "Hello World");
        fclose(test);
        ChecksumResult result = processFile("test1.txt", "output1.txt");
        printf("Test 1 - Bytes: %d, Checksum: %lld\\n", result.bytesRead, result.checksum);
    }
    
    // Test case 2: Empty file
    {
        FILE *test = fopen("test2.txt", "w");
        fclose(test);
        ChecksumResult result = processFile("test2.txt", "output2.txt");
        printf("Test 2 - Bytes: %d, Checksum: %lld\\n", result.bytesRead, result.checksum);
    }
    
    // Test case 3: File with numbers
    {
        FILE *test = fopen("test3.txt", "w");
        fprintf(test, "1234567890");
        fclose(test);
        ChecksumResult result = processFile("test3.txt", "output3.txt");
        printf("Test 3 - Bytes: %d, Checksum: %lld\\n", result.bytesRead, result.checksum);
    }
    
    // Test case 4: File with special characters
    {
        FILE *test = fopen("test4.txt", "w");
        fprintf(test, "!@#$%%^&*()");
        fclose(test);
        ChecksumResult result = processFile("test4.txt", "output4.txt");
        printf("Test 4 - Bytes: %d, Checksum: %lld\\n", result.bytesRead, result.checksum);
    }
    
    // Test case 5: Larger text file
    {
        FILE *test = fopen("test5.txt", "w");
        fprintf(test, "The quick brown fox jumps over the lazy dog");
        fclose(test);
        ChecksumResult result = processFile("test5.txt", "output5.txt");
        printf("Test 5 - Bytes: %d, Checksum: %lld\\n", result.bytesRead, result.checksum);
    }
    
    return 0;
}
