
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char* data;
    size_t size;
} FileData;

FileData img(const char* file) {
    FileData result = {NULL, 0};
    
    if (file == NULL || strlen(file) == 0) {
        return result;
    }
    
    FILE* fp = fopen(file, "rb");
    if (fp == NULL) {
        printf("Error: Could not open file %s\\n", file);
        return result;
    }
    
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    result.data = (char*)malloc(size);
    if (result.data == NULL) {
        fclose(fp);
        return result;
    }
    
    result.size = fread(result.data, 1, size, fp);
    fclose(fp);
    
    return result;
}

int main() {
    // Test case 1: Valid image file
    FileData result1 = img("test1.jpg");
    printf("Test 1 - File: test1.jpg, Result: %s\\n", 
           result1.data != NULL ? "Success" : "Failed");
    if (result1.data) free(result1.data);
    
    // Test case 2: Valid text file
    FileData result2 = img("test2.txt");
    printf("Test 2 - File: test2.txt, Result: %s\\n", 
           result2.data != NULL ? "Success" : "Failed");
    if (result2.data) free(result2.data);
    
    // Test case 3: Empty file parameter
    FileData result3 = img("");
    printf("Test 3 - Empty file, Result: %s\\n", 
           result3.data != NULL ? "Success" : "Failed (Expected)");
    if (result3.data) free(result3.data);
    
    // Test case 4: Null file parameter
    FileData result4 = img(NULL);
    printf("Test 4 - Null file, Result: %s\\n", 
           result4.data != NULL ? "Success" : "Failed (Expected)");
    if (result4.data) free(result4.data);
    
    // Test case 5: Non-existent file
    FileData result5 = img("nonexistent.jpg");
    printf("Test 5 - Non-existent file, Result: %s\\n", 
           result5.data != NULL ? "Success" : "Failed (Expected)");
    if (result5.data) free(result5.data);
    
    return 0;
}
