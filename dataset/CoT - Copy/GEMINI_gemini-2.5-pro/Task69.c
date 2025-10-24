#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h> // For _mkdir
#define MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h> // For mkdir
#include <sys/types.h>
#define MKDIR(path) mkdir(path, 0777)
#endif


/**
 * Deletes a file specified by the file path.
 * @param filePath The path to the file to be deleted.
 */
void deleteFile(const char* filePath) {
    if (filePath == NULL || filePath[0] == '\0') {
        fprintf(stderr, "Error: File path cannot be null or empty.\n");
        return;
    }

    // remove() returns 0 on success, non-zero on failure
    if (remove(filePath) == 0) {
        printf("Success: File '%s' was deleted successfully.\n", filePath);
    } else {
        // perror() prints a descriptive error message to stderr.
        // It uses the global 'errno' variable set by remove() on failure.
        fprintf(stderr, "Error: Failed to delete file '%s'. ", filePath);
        perror(""); // Passing "" prints only the system error message
    }
}

/**
 * Helper function to create an empty file for testing.
 */
void createFile(const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (fp != NULL) {
        fclose(fp);
    }
}

/**
 * Sets up and runs a series of test cases.
 */
void runTestCases() {
    printf("--- Running Test Cases ---\n");
    const char* testFile1 = "test1.tmp";
    const char* testFile2 = "test2.tmp";
    const char* nonExistentFile = "nonexistent.tmp";
    const char* alreadyDeletedFile = "test1.tmp";
    const char* testDir = "testdir.tmp";
    
    // Setup test files and directory
    createFile(testFile1);
    createFile(testFile2);
    MKDIR(testDir);
    
    // Test Case 1: Delete an existing file
    printf("\n[Test Case 1: Delete an existing file]\n");
    deleteFile(testFile1);

    // Test Case 2: Delete another existing file
    printf("\n[Test Case 2: Delete another existing file]\n");
    deleteFile(testFile2);

    // Test Case 3: Attempt to delete a non-existent file
    printf("\n[Test Case 3: Attempt to delete a non-existent file]\n");
    deleteFile(nonExistentFile);

    // Test Case 4: Attempt to delete a file that was already deleted
    printf("\n[Test Case 4: Attempt to delete an already deleted file]\n");
    deleteFile(alreadyDeletedFile);

    // Test Case 5: Attempt to delete a directory
    printf("\n[Test Case 5: Attempt to delete a directory]\n");
    deleteFile(testDir);

    // Cleanup the test directory
    remove(testDir);

    printf("\n--- Test Cases Finished ---\n");
}

int main(int argc, char* argv[]) {
    runTestCases();

    // --- Main Logic for Command Line ---
    if (argc > 1) {
        printf("\n--- Deleting file from command line argument ---\n");
        deleteFile(argv[1]);
    } else {
        printf("\nUsage: %s <file_path_to_delete>\n", argv[0]);
        printf("Example: %s my_file.txt\n", argv[0]);
    }
    
    return 0;
}