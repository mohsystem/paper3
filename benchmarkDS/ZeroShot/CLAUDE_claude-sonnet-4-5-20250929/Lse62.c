
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    long bytesRead;
    long checksum;
} ChecksumResult;

ChecksumResult processFile(const char* inputFilePath, const char* outputFilePath) {
    ChecksumResult result = {0, 0};
    FILE* inputFile = NULL;
    FILE* outputFile = NULL;
    unsigned char* buffer = NULL;
    long fileSize = 0;
    
    // Open input file
    inputFile = fopen(inputFilePath, "rb");
    if (inputFile == NULL) {
        fprintf(stderr, "Error: Cannot open input file\\n");
        return result;
    }
    
    // Get file size
    fseek(inputFile, 0, SEEK_END);
    fileSize = ftell(inputFile);
    fseek(inputFile, 0, SEEK_SET);
    
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
    
    result.bytesRead = bytesRead;
    
    // Calculate checksum
    long checksum = 0;
    for (size_t i = 0; i < bytesRead; i++) {
        checksum += buffer[i];
    }
    result.checksum = checksum;
    
    // Free buffer
    free(buffer);
    
    // Save results to output file
    outputFile = fopen(outputFilePath, "w");
    if (outputFile == NULL) {
        fprintf(stderr, "Error: Cannot open output file\\n");
        return result;
    }
    
    fprintf(outputFile, "Bytes Read: %ld\\n", result.bytesRead);
    fprintf(outputFile, "Checksum: %ld\\n", result.checksum);
    if (checksum == 0) {
        fprintf(outputFile, "Status: Buffer is empty or all zeros\\n");
    } else {
        fprintf(outputFile, "Status: Buffer contains data\\n");
    }
    fclose(outputFile);
    
    return result;
}

int main() {
    printf("File Checksum Calculator - Test Cases\\n\\n");
    
    // Test Case 1: Empty file
    {
        FILE* test1 = fopen("test1_empty.txt", "w");
        fclose(test1);
        ChecksumResult result1 = processFile("test1_empty.txt", "output1.txt");
        printf("Test 1 - Empty file: Bytes=%ld, Checksum=%ld\\n", 
               result1.bytesRead, result1.checksum);
        remove("test1_empty.txt");
    }
    
    // Test Case 2: Small text file
    {
        FILE* test2 = fopen("test2_small.txt", "w");
        fprintf(test2, "Hello");
        fclose(test2);
        ChecksumResult result2 = processFile("test2_small.txt", "output2.txt");
        printf("Test 2 - Small file: Bytes=%ld, Checksum=%ld\\n", 
               result2.bytesRead, result2.checksum);
        remove("test2_small.txt");
    }
    
    // Test Case 3: File with numbers
    {
        FILE* test3 = fopen("test3_numbers.txt", "w");
        fprintf(test3, "12345");
        fclose(test3);
        ChecksumResult result3 = processFile("test3_numbers.txt", "output3.txt");
        printf("Test 3 - Numbers file: Bytes=%ld, Checksum=%ld\\n", 
               result3.bytesRead, result3.checksum);
        remove("test3_numbers.txt");
    }
    
    // Test Case 4: File with special characters
    {
        FILE* test4 = fopen("test4_special.txt", "w");
        fprintf(test4, "@#$%%^");
        fclose(test4);
        ChecksumResult result4 = processFile("test4_special.txt", "output4.txt");
        printf("Test 4 - Special chars: Bytes=%ld, Checksum=%ld\\n", 
               result4.bytesRead, result4.checksum);
        remove("test4_special.txt");
    }
    
    // Test Case 5: Larger text file
    {
        FILE* test5 = fopen("test5_large.txt", "w");
        fprintf(test5, "The quick brown fox jumps over the lazy dog.");
        fclose(test5);
        ChecksumResult result5 = processFile("test5_large.txt", "output5.txt");
        printf("Test 5 - Larger file: Bytes=%ld, Checksum=%ld\\n", 
               result5.bytesRead, result5.checksum);
        remove("test5_large.txt");
    }
    
    return 0;
}
