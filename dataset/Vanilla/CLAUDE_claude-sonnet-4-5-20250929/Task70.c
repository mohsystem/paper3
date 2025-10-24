
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* readFileContent(const char* filePath) {
    FILE* file = fopen(filePath, "r");
    
    if (file == NULL) {
        char* error = (char*)malloc(256 * sizeof(char));
        sprintf(error, "Error reading file: File not found - %s", filePath);
        return error;
    }
    
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* content = (char*)malloc((fileSize + 1) * sizeof(char));
    if (content == NULL) {
        fclose(file);
        char* error = (char*)malloc(256 * sizeof(char));
        sprintf(error, "Error: Memory allocation failed");
        return error;
    }
    
    size_t readSize = fread(content, 1, fileSize, file);
    content[readSize] = '\\0';
    
    fclose(file);
    return content;
}

int main(int argc, char* argv[]) {
    // Test cases
    const char* testFiles[] = {
        "test1.txt",
        "test2.txt",
        "test3.txt",
        "nonexistent.txt",
        "test5.txt"
    };
    
    // Create test files
    FILE* file1 = fopen("test1.txt", "w");
    fprintf(file1, "Hello, World!");
    fclose(file1);
    
    FILE* file2 = fopen("test2.txt", "w");
    fprintf(file2, "C Programming");
    fclose(file2);
    
    FILE* file3 = fopen("test3.txt", "w");
    fprintf(file3, "File I/O Operations");
    fclose(file3);
    
    FILE* file5 = fopen("test5.txt", "w");
    fprintf(file5, "Test Case 5");
    fclose(file5);
    
    // If command line argument provided, use it
    if (argc > 1) {
        char* content = readFileContent(argv[1]);
        printf("File Content:\\n%s\\n", content);
        free(content);
    } else {
        // Run test cases
        printf("Running test cases:\\n\\n");
        for (int i = 0; i < 5; i++) {
            printf("Test Case %d: %s\\n", i + 1, testFiles[i]);
            char* content = readFileContent(testFiles[i]);
            printf("Content: %s\\n", content);
            printf("---\\n");
            free(content);
        }
    }
    
    return 0;
}
