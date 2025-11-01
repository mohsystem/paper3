
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int bytesRead;
    long checksum;
} ChecksumResult;

long calculateChecksum(unsigned char* buffer, int size) {
    long checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += buffer[i];
    }
    return checksum;
}

void saveResults(const char* outputPath, int bytesRead, long checksum) {
    FILE* outFile = fopen(outputPath, "w");
    if (outFile != NULL) {
        fprintf(outFile, "Bytes Read: %d\\n", bytesRead);
        fprintf(outFile, "Checksum: %ld\\n", checksum);
        fclose(outFile);
    }
}

ChecksumResult processFile(const char* inputPath, const char* outputPath) {
    ChecksumResult result = {0, 0};
    
    FILE* inFile = fopen(inputPath, "rb");
    if (inFile == NULL) {
        fprintf(stderr, "Error opening file: %s\\n", inputPath);
        return result;
    }
    
    // Get file size
    fseek(inFile, 0, SEEK_END);
    long fileSize = ftell(inFile);
    fseek(inFile, 0, SEEK_SET);
    
    // Allocate buffer
    unsigned char* buffer = (unsigned char*)malloc(fileSize);
    if (buffer == NULL) {
        fprintf(stderr, "Memory allocation failed\\n");
        fclose(inFile);
        return result;
    }
    
    // Read file contents into buffer
    size_t bytesRead = fread(buffer, 1, fileSize, inFile);
    fclose(inFile);
    
    result.bytesRead = bytesRead;
    
    // Calculate checksum
    result.checksum = calculateChecksum(buffer, bytesRead);
    
    // Check if buffer is empty (checksum is 0)
    if (result.checksum == 0) {
        printf("Buffer is empty or checksum is 0\\n");
    }
    
    // Save results to output file
    saveResults(outputPath, result.bytesRead, result.checksum);
    
    free(buffer);
    return result;
}

int main() {
    // Test case 1: File with some content
    {
        FILE* testFile = fopen("test1.txt", "w");
        fprintf(testFile, "Hello World");
        fclose(testFile);
        ChecksumResult result1 = processFile("test1.txt", "output1.txt");
        printf("Test 1 - Bytes: %d, Checksum: %ld\\n", result1.bytesRead, result1.checksum);
    }
    
    // Test case 2: Empty file
    {
        FILE* testFile = fopen("test2.txt", "w");
        fclose(testFile);
        ChecksumResult result2 = processFile("test2.txt", "output2.txt");
        printf("Test 2 - Bytes: %d, Checksum: %ld\\n", result2.bytesRead, result2.checksum);
    }
    
    // Test case 3: File with numbers
    {
        FILE* testFile = fopen("test3.txt", "w");
        fprintf(testFile, "1234567890");
        fclose(testFile);
        ChecksumResult result3 = processFile("test3.txt", "output3.txt");
        printf("Test 3 - Bytes: %d, Checksum: %ld\\n", result3.bytesRead, result3.checksum);
    }
    
    // Test case 4: File with special characters
    {
        FILE* testFile = fopen("test4.txt", "w");
        fprintf(testFile, "!@#$%%^&*()");
        fclose(testFile);
        ChecksumResult result4 = processFile("test4.txt", "output4.txt");
        printf("Test 4 - Bytes: %d, Checksum: %ld\\n", result4.bytesRead, result4.checksum);
    }
    
    // Test case 5: File with longer text
    {
        FILE* testFile = fopen("test5.txt", "w");
        fprintf(testFile, "The quick brown fox jumps over the lazy dog");
        fclose(testFile);
        ChecksumResult result5 = processFile("test5.txt", "output5.txt");
        printf("Test 5 - Bytes: %d, Checksum: %ld\\n", result5.bytesRead, result5.checksum);
    }
    
    return 0;
}
