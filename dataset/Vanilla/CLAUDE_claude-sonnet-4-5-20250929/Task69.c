
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

int deleteFile(const char* filePath) {
    if (filePath == NULL || strlen(filePath) == 0) {
        printf("Error: File path is empty or null\\n");
        return 0;
    }
    
    struct stat pathStat;
    if (stat(filePath, &pathStat) != 0) {
        printf("Error: File does not exist - %s\\n", filePath);
        return 0;
    }
    
    if (S_ISDIR(pathStat.st_mode)) {
        printf("Error: Path is a directory, not a file - %s\\n", filePath);
        return 0;
    }
    
    if (remove(filePath) == 0) {
        printf("File deleted successfully: %s\\n", filePath);
        return 1;
    } else {
        printf("Error: Failed to delete file - %s\\n", filePath);
        return 0;
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            deleteFile(argv[i]);
        }
    } else {
        /* Test cases */
        printf("Running test cases...\\n\\n");
        
        /* Test case 1: Create and delete a test file */
        printf("Test 1:\\n");
        FILE* testFile1 = fopen("test_file_1.txt", "w");
        if (testFile1) {
            fprintf(testFile1, "test content");
            fclose(testFile1);
            deleteFile("test_file_1.txt");
        }
        
        printf("\\n");
        
        /* Test case 2: Try to delete non-existent file */
        printf("Test 2:\\n");
        deleteFile("non_existent_file.txt");
        
        printf("\\n");
        
        /* Test case 3: Create and delete another test file */
        printf("Test 3:\\n");
        FILE* testFile3 = fopen("test_file_3.dat", "w");
        if (testFile3) {
            fprintf(testFile3, "test data");
            fclose(testFile3);
            deleteFile("test_file_3.dat");
        }
        
        printf("\\n");
        
        /* Test case 4: Try to delete with empty path */
        printf("Test 4:\\n");
        deleteFile("");
        
        printf("\\n");
        
        /* Test case 5: Try to delete with null path */
        printf("Test 5:\\n");
        deleteFile(NULL);
    }
    
    return 0;
}
