
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int bytesRead;
    int checksum;
} Result;

Result processFile(const char* inputFilePath, const char* outputFilePath) {
    Result result = {0, 0};
    FILE* inputFile = NULL;
    FILE* outputFile = NULL;
    unsigned char* buffer = NULL;
    long fileSize;
    
    // Open input file
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
    
    // Write results to output file
    outputFile = fopen(outputFilePath, "w");
    if (outputFile) {
        fprintf(outputFile, "Bytes Read: %d\\n", result.bytesRead);
        fprintf(outputFile, "Checksum: %d\\n", result.checksum);
        fclose(outputFile);
    }
    
    free(buffer);
    return result;
}

int main() {
    // Test case 1: Empty file
    {
        FILE* f = fopen("test1.txt", "wb");
        fclose(f);
        Result r1 = processFile("test1.txt", "output1.txt");
        printf("Test 1 - Bytes: %d, Checksum: %d\\n", r1.bytesRead, r1.checksum);
    }
    
    // Test case 2: Small text file
    {
        FILE* f = fopen("test2.txt", "wb");
        fprintf(f, "Hello");
        fclose(f);
        Result r2 = processFile("test2.txt", "output2.txt");
        printf("Test 2 - Bytes: %d, Checksum: %d\\n", r2.bytesRead, r2.checksum);
    }
    
    // Test case 3: Numbers
    {
        FILE* f = fopen("test3.txt", "wb");
        fprintf(f, "12345");
        fclose(f);
        Result r3 = processFile("test3.txt", "output3.txt");
        printf("Test 3 - Bytes: %d, Checksum: %d\\n", r3.bytesRead, r3.checksum);
    }
    
    // Test case 4: Special characters
    {
        FILE* f = fopen("test4.txt", "wb");
        fprintf(f, "!@#$%%");
        fclose(f);
        Result r4 = processFile("test4.txt", "output4.txt");
        printf("Test 4 - Bytes: %d, Checksum: %d\\n", r4.bytesRead, r4.checksum);
    }
    
    // Test case 5: Binary data
    {
        FILE* f = fopen("test5.txt", "wb");
        unsigned char data[] = {1, 2, 3, 4, 5, 255, 254, 253};
        fwrite(data, 1, sizeof(data), f);
        fclose(f);
        Result r5 = processFile("test5.txt", "output5.txt");
        printf("Test 5 - Bytes: %d, Checksum: %d\\n", r5.bytesRead, r5.checksum);
    }
    
    return 0;
}
