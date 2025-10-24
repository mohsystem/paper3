
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

int deleteFile(const char* filePath) {
    if (filePath == NULL || strlen(filePath) == 0) {
        printf("Error: File path cannot be null or empty\\n");
        return 0;
    }
    
    struct stat fileInfo;
    if (stat(filePath, &fileInfo) != 0) {
        printf("Error: File does not exist: %s\\n", filePath);
        return 0;
    }
    
    if (!S_ISREG(fileInfo.st_mode)) {
        printf("Error: Path is not a regular file: %s\\n", filePath);
        return 0;
    }
    
    if (remove(filePath) == 0) {
        printf("File deleted successfully: %s\\n", filePath);
        return 1;
    } else {
        printf("Error: Failed to delete file - %s\\n", strerror(errno));
        return 0;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <file_path>\\n", argv[0]);
        
        printf("\\n=== Running Test Cases ===\\n\\n");
        
        const char* testFile1 = "test1_temp.txt";
        FILE* file1 = fopen(testFile1, "w");
        if (file1) {
            fprintf(file1, "test");
            fclose(file1);
        }
        printf("Test 1: Delete existing file\\n");
        deleteFile(testFile1);
        
        printf("\\nTest 2: Delete non-existent file\\n");
        deleteFile("non_existent_file.txt");
        
        printf("\\nTest 3: Delete with NULL path\\n");
        deleteFile(NULL);
        
        printf("\\nTest 4: Delete with empty path\\n");
        deleteFile("");
        
        const char* testFile5 = "test5_temp.txt";
        FILE* file5 = fopen(testFile5, "w");
        if (file5) {
            fprintf(file5, "test");
            fclose(file5);
        }
        printf("\\nTest 5: Delete valid file\\n");
        deleteFile(testFile5);
        
        return 0;
    }
    
    deleteFile(argv[1]);
    return 0;
}
