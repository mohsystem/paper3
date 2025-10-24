
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

char* deleteFile(const char* filePath, char* result) {
    if (filePath == NULL || strlen(filePath) == 0) {
        strcpy(result, "Error: File path cannot be null or empty");
        return result;
    }
    
    struct stat buffer;
    
    // Check if file exists
    if (stat(filePath, &buffer) != 0) {
        sprintf(result, "Error: File does not exist: %s", filePath);
        return result;
    }
    
    // Check if it's a regular file
    if (!S_ISREG(buffer.st_mode)) {
        sprintf(result, "Error: Path is not a file: %s", filePath);
        return result;
    }
    
    // Try to delete the file
    if (remove(filePath) == 0) {
        sprintf(result, "Success: File deleted successfully: %s", filePath);
    } else {
        sprintf(result, "Error: Failed to delete file: %s", filePath);
    }
    
    return result;
}

int main(int argc, char* argv[]) {
    char result[512];
    
    // Test cases
    printf("Test Case 1: Empty file path\\n");
    printf("%s\\n\\n", deleteFile("", result));
    
    printf("Test Case 2: NULL file path\\n");
    printf("%s\\n\\n", deleteFile(NULL, result));
    
    printf("Test Case 3: Non-existent file\\n");
    printf("%s\\n\\n", deleteFile("nonexistent_file.txt", result));
    
    printf("Test Case 4: Create and delete a test file\\n");
    const char* testFile = "test_file_to_delete.txt";
    FILE* file = fopen(testFile, "w");
    if (file != NULL) {
        fprintf(file, "test content");
        fclose(file);
        printf("%s\\n\\n", deleteFile(testFile, result));
    } else {
        printf("Error creating test file\\n\\n");
    }
    
    printf("Test Case 5: Command line argument\\n");
    if (argc > 1) {
        printf("%s\\n", deleteFile(argv[1], result));
    } else {
        printf("No command line argument provided\\n");
    }
    
    return 0;
}
