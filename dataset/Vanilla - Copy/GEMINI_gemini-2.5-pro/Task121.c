#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

// For cross-platform directory creation and path separators
#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#define RMDIR(path) _rmdir(path)
#define PATH_SEPARATOR '\\'
#else
#include <unistd.h>
#define MKDIR(path) mkdir(path, 0777)
#define RMDIR(path) rmdir(path)
#define PATH_SEPARATOR '/'
#endif

/**
 * @brief Creates a directory path recursively.
 * 
 * @param path The full directory path to create.
 * @return 0 on success, -1 on failure.
 */
int create_directory_recursively(const char* path) {
    char* p = NULL;
    char temp_path[512];
    snprintf(temp_path, sizeof(temp_path), "%s", path);
    size_t len = strlen(temp_path);
    if (len > 0 && temp_path[len - 1] == PATH_SEPARATOR) {
        temp_path[len - 1] = 0;
    }
    for (p = temp_path + 1; *p; p++) {
        if (*p == PATH_SEPARATOR) {
            *p = 0;
            if (MKDIR(temp_path) != 0 && errno != EEXIST) {
                return -1;
            }
            *p = PATH_SEPARATOR;
        }
    }
    if (MKDIR(temp_path) != 0 && errno != EEXIST) {
        return -1;
    }
    return 0;
}

/**
 * @brief Simulates file upload by copying a file from source to destination.
 * 
 * @param sourceFilePath Path of the file to copy.
 * @param destinationFilePath Full path for the new file.
 * @return 0 on success, -1 on failure.
 */
int uploadFile(const char* sourceFilePath, const char* destinationFilePath) {
    FILE *sourceFile, *destFile;
    char buffer[4096];
    size_t bytesRead;

    sourceFile = fopen(sourceFilePath, "rb");
    if (sourceFile == NULL) {
        fprintf(stderr, "Error: Cannot open source file '%s'\n", sourceFilePath);
        return -1;
    }

    char dest_dir[512];
    strncpy(dest_dir, destinationFilePath, sizeof(dest_dir) - 1);
    dest_dir[sizeof(dest_dir)-1] = '\0';
    char* last_slash = strrchr(dest_dir, PATH_SEPARATOR);
    if(last_slash != NULL) {
        *last_slash = '\0';
        if(create_directory_recursively(dest_dir) != 0){
             fprintf(stderr, "Error: Could not create directory '%s'\n", dest_dir);
             fclose(sourceFile);
             return -1;
        }
    }

    destFile = fopen(destinationFilePath, "wb");
    if (destFile == NULL) {
        fprintf(stderr, "Error: Cannot open destination file '%s'\n", destinationFilePath);
        fclose(sourceFile);
        return -1;
    }

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), sourceFile)) > 0) {
        if (fwrite(buffer, 1, bytesRead, destFile) != bytesRead) {
            fprintf(stderr, "Error: Failed to write to destination file.\n");
            fclose(sourceFile);
            fclose(destFile);
            return -1;
        }
    }
    
    if(ferror(sourceFile)){
        fprintf(stderr, "Error: Failed reading from source file.\n");
    }

    printf("File uploaded successfully from '%s' to '%s'\n", sourceFilePath, destinationFilePath);

    fclose(sourceFile);
    fclose(destFile);

    return 0;
}

// Helper function to create a dummy file for testing
void create_dummy_file(const char* filename, const char* content) {
    FILE* fp = fopen(filename, "w");
    if (fp) {
        fprintf(fp, "%s", content);
        fclose(fp);
    }
}

// Helper function to remove a directory and its contents (simple version)
void remove_directory(const char* path) {
    char filepath[512];

    snprintf(filepath, sizeof(filepath), "%s%c%s", path, PATH_SEPARATOR, "uploaded_file1.txt"); remove(filepath);
    snprintf(filepath, sizeof(filepath), "%s%c%s", path, PATH_SEPARATOR, "overwrite.txt"); remove(filepath);
    snprintf(filepath, sizeof(filepath), "%s%c%s", path, PATH_SEPARATOR, "renamed_file.txt"); remove(filepath);
    snprintf(filepath, sizeof(filepath), "%s%cnew%cnested%c%s", path, PATH_SEPARATOR, PATH_SEPARATOR, PATH_SEPARATOR, "uploaded_file4.txt"); remove(filepath);
    snprintf(filepath, sizeof(filepath), "%s%cnew%cnested", path, PATH_SEPARATOR, PATH_SEPARATOR); RMDIR(filepath);
    snprintf(filepath, sizeof(filepath), "%s%cnew", path, PATH_SEPARATOR); RMDIR(filepath);

    RMDIR(path);
}


int main() {
    const char* testDir = "c_upload_test_dir";
    const char* sourceFile = "c_source.txt";
    
    create_dummy_file(sourceFile, "This is a C test file for upload.");
    MKDIR(testDir);
    
    printf("--- Running Test Cases ---\n");
    char dest_path[512];

    // Test Case 1: Successful upload
    printf("\n--- Test Case 1: Successful Upload ---\n");
    snprintf(dest_path, sizeof(dest_path), "%s%c%s", testDir, PATH_SEPARATOR, "uploaded_file1.txt");
    uploadFile(sourceFile, dest_path);

    // Test Case 2: Uploading a non-existent source file
    printf("\n--- Test Case 2: Non-existent Source File ---\n");
    snprintf(dest_path, sizeof(dest_path), "%s%c%s", testDir, PATH_SEPARATOR, "uploaded_file2.txt");
    uploadFile("non_existent_file.txt", dest_path);

    // Test Case 3: Overwriting an existing file
    printf("\n--- Test Case 3: Overwriting an Existing File ---\n");
    snprintf(dest_path, sizeof(dest_path), "%s%c%s", testDir, PATH_SEPARATOR, "overwrite.txt");
    uploadFile(sourceFile, dest_path); // First time
    printf("Uploading again to the same destination...\n");
    uploadFile(sourceFile, dest_path); // Second time

    // Test Case 4: Uploading to a nested directory that doesn't exist
    printf("\n--- Test Case 4: Upload to a non-existent nested directory ---\n");
    snprintf(dest_path, sizeof(dest_path), "%s%cnew%cnested%c%s", testDir, PATH_SEPARATOR, PATH_SEPARATOR, PATH_SEPARATOR, "uploaded_file4.txt");
    uploadFile(sourceFile, dest_path);

    // Test Case 5: Uploading with a new filename
    printf("\n--- Test Case 5: Uploading with a new filename ---\n");
    snprintf(dest_path, sizeof(dest_path), "%s%c%s", testDir, PATH_SEPARATOR, "renamed_file.txt");
    uploadFile(sourceFile, dest_path);

    // Cleanup
    printf("\n--- Cleaning up test files and directories ---\n");
    remove(sourceFile);
    remove_directory(testDir);
    printf("Cleanup complete.\n");
    
    return 0;
}