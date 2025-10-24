
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* readFileContents(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        char* error = (char*)malloc(100 * sizeof(char));
        sprintf(error, "Error reading file: Unable to open file");
        return error;
    }
    
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* content = (char*)malloc((fileSize + 1) * sizeof(char));
    if (content == NULL) {
        fclose(file);
        char* error = (char*)malloc(100 * sizeof(char));
        sprintf(error, "Error: Memory allocation failed");
        return error;
    }
    
    size_t bytesRead = fread(content, 1, fileSize, file);
    content[bytesRead] = '\\0';
    
    fclose(file);
    return content;
}

int main() {
    // Test case 1: Create and read a simple text file
    {
        FILE* file = fopen("test1.txt", "w");
        fprintf(file, "Hello, World!\\n");
        fprintf(file, "This is test file 1.\\n");
        fclose(file);
        printf("Test Case 1:\\n");
        char* content = readFileContents("test1.txt");
        printf("%s\\n", content);
        free(content);
    }
    
    // Test case 2: Create and read a file with multiple lines
    {
        FILE* file = fopen("test2.txt", "w");
        fprintf(file, "Line 1\\n");
        fprintf(file, "Line 2\\n");
        fprintf(file, "Line 3\\n");
        fclose(file);
        printf("Test Case 2:\\n");
        char* content = readFileContents("test2.txt");
        printf("%s\\n", content);
        free(content);
    }
    
    // Test case 3: Empty file
    {
        FILE* file = fopen("test3.txt", "w");
        fclose(file);
        printf("Test Case 3 (Empty file):\\n");
        char* content = readFileContents("test3.txt");
        printf("%s\\n", content);
        free(content);
    }
    
    // Test case 4: File with special characters
    {
        FILE* file = fopen("test4.txt", "w");
        fprintf(file, "Special chars: @#$%%^&*()\\n");
        fprintf(file, "Numbers: 123456789\\n");
        fclose(file);
        printf("Test Case 4:\\n");
        char* content = readFileContents("test4.txt");
        printf("%s\\n", content);
        free(content);
    }
    
    // Test case 5: Non-existent file
    printf("Test Case 5 (Non-existent file):\\n");
    char* content = readFileContents("nonexistent.txt");
    printf("%s\\n", content);
    free(content);
    
    return 0;
}
