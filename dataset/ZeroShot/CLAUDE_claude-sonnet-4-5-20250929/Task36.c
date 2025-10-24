
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define MAX_CONTENT_SIZE 10000

char* readFileContents(const char* filePath) {
    static char content[MAX_CONTENT_SIZE];
    memset(content, 0, MAX_CONTENT_SIZE);
    
    // Check if file exists and is a regular file
    struct stat fileInfo;
    if (stat(filePath, &fileInfo) != 0) {
        strcpy(content, "Error: File does not exist");
        return content;
    }
    if (!S_ISREG(fileInfo.st_mode)) {
        strcpy(content, "Error: Path is not a file");
        return content;
    }
    
    // Try to open the file
    FILE* file = fopen(filePath, "r");
    if (file == NULL) {
        strcpy(content, "Error: Cannot open file");
        return content;
    }
    
    // Read file contents
    char line[1024];
    int offset = 0;
    while (fgets(line, sizeof(line), file) != NULL && offset < MAX_CONTENT_SIZE - 1) {
        int len = strlen(line);
        if (offset + len < MAX_CONTENT_SIZE) {
            strcpy(content + offset, line);
            offset += len;
        }
    }
    
    fclose(file);
    return content;
}

int main() {
    // Create test files
    FILE* f1 = fopen("test1.txt", "w");
    fprintf(f1, "Hello World!\\nThis is test file 1.");
    fclose(f1);
    
    FILE* f2 = fopen("test2.txt", "w");
    fprintf(f2, "Line 1\\nLine 2\\nLine 3");
    fclose(f2);
    
    FILE* f3 = fopen("test3.txt", "w");
    fprintf(f3, "Single line file");
    fclose(f3);
    
    FILE* f4 = fopen("test4.txt", "w");
    fclose(f4);
    
    FILE* f5 = fopen("test5.txt", "w");
    fprintf(f5, "Test file 5\\nWith multiple\\nLines of text");
    fclose(f5);
    
    // Test cases
    printf("Test Case 1:\\n%s\\n", readFileContents("test1.txt"));
    printf("Test Case 2:\\n%s\\n", readFileContents("test2.txt"));
    printf("Test Case 3:\\n%s\\n", readFileContents("test3.txt"));
    printf("Test Case 4 (empty file):\\n%s\\n", readFileContents("test4.txt"));
    printf("Test Case 5:\\n%s\\n", readFileContents("test5.txt"));
    
    // Cleanup
    remove("test1.txt");
    remove("test2.txt");
    remove("test3.txt");
    remove("test4.txt");
    remove("test5.txt");
    
    return 0;
}
