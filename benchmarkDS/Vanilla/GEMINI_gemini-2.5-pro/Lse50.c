#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <unistd.h> // For access()

/**
 * Extracts a zip file to a specified directory using the system's 'unzip' command.
 * Note: This function requires the 'unzip' command-line tool to be installed and in the system's PATH.
 * The system calls are tailored for Linux/macOS.
 *
 * @param zip_file_path Path to the .zip file.
 * @param dest_directory Path to the destination directory.
 * @return 0 on success, -1 on failure.
 */
int extract_zip(const char* zip_file_path, const char* dest_directory) {
    // Check if zip file exists
    if (access(zip_file_path, F_OK) == -1) {
        fprintf(stderr, "Error: ZIP file not found at '%s'\n", zip_file_path);
        return -1;
    }

    // Create destination directory if it doesn't exist.
    char mkdir_command[512];
    snprintf(mkdir_command, sizeof(mkdir_command), "mkdir -p \"%s\"", dest_directory);
    system(mkdir_command);

    // Construct the command
    // -o: overwrite files without prompting. -q: quiet mode.
    char command[1024];
    snprintf(command, sizeof(command), "unzip -o -q \"%s\" -d \"%s\"", zip_file_path, dest_directory);

    // Execute the command
    int result = system(command);

    if (result == 0) {
        return 0;
    } else {
        fprintf(stderr, "Error: Failed to extract '%s'. Make sure 'unzip' is installed.\n", zip_file_path);
        return -1;
    }
}

// Helper to check if a file exists
bool file_exists(const char* path) {
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

int main() {
    // Note: This implementation requires 'zip' and 'unzip' command-line tools.
    // The system calls are tailored for Linux/macOS.
    printf("Running C Lse50 tests...\n");
    const char* dest_dir = "/tmp/unpack";

    // Test Case 1 & 2: Basic extraction and creating destination directory
    printf("\n--- Test Case 1 & 2: Basic Extraction ---\n");
    {
        const char* zip_file = "archive1.zip";
        system("rm -rf /tmp/unpack test_setup archive1.zip");
        system("mkdir -p test_setup/dir1 && echo 'hello' > test_setup/file1.txt && echo 'world' > test_setup/dir1/file2.txt");
        system("zip -r -q archive1.zip test_setup/*");
        
        if (extract_zip(zip_file, dest_dir) == 0) {
            if (file_exists("/tmp/unpack/test_setup/file1.txt") && file_exists("/tmp/unpack/test_setup/dir1/file2.txt")) {
                printf("PASS: Files extracted successfully.\n");
            } else {
                printf("FAIL: File extraction verification failed.\n");
            }
        } else {
            printf("FAIL: Extraction function returned -1.\n");
        }
        system("rm -rf /tmp/unpack test_setup archive1.zip");
    }

    // Test Case 3: Non-existent zip file
    printf("\n--- Test Case 3: Non-existent ZIP file ---\n");
    {
        if (extract_zip("nonexistent.zip", dest_dir) == -1) {
            printf("PASS: Correctly handled non-existent zip file.\n");
        } else {
            printf("FAIL: Should have returned -1 for non-existent file.\n");
        }
    }

    // Test Case 4: Extracting an empty zip file
    printf("\n--- Test Case 4: Empty ZIP file ---\n");
    {
        const char* zip_file = "archive_empty.zip";
        system("rm -rf /tmp/unpack test_setup archive_empty.zip");
        system("mkdir -p test_setup");
        system("zip -r -q archive_empty.zip test_setup/*");

        if (extract_zip(zip_file, dest_dir) == 0) {
            // Check if the directory exists but is empty. A simple way is to try listing it.
            int list_result = system("test -z \"$(ls -A /tmp/unpack/test_setup)\"");
            if (list_result == 0) {
                printf("PASS: Empty zip extracted correctly.\n");
            } else {
                printf("FAIL: Directory should be empty after extraction.\n");
            }
        } else {
             printf("FAIL: Extraction function returned -1.\n");
        }
        system("rm -rf /tmp/unpack test_setup archive_empty.zip");
    }
    
    // Test Case 5: Overwriting existing files
    printf("\n--- Test Case 5: Overwriting existing files ---\n");
    {
        const char* zip_file = "archive_overwrite.zip";
        system("rm -rf /tmp/unpack test_setup archive_overwrite.zip");
        system("mkdir -p /tmp/unpack/test_setup && echo 'old content' > /tmp/unpack/test_setup/file.txt");
        system("mkdir -p test_setup && echo 'new content' > test_setup/file.txt");
        system("zip -r -q archive_overwrite.zip test_setup/*");

        if (extract_zip(zip_file, dest_dir) == 0) {
            // Use grep to check content
            int grep_result = system("grep -q 'new content' /tmp/unpack/test_setup/file.txt");
            if (grep_result == 0) {
                printf("PASS: File was successfully overwritten.\n");
            } else {
                printf("FAIL: File content was not overwritten.\n");
            }
        } else {
            printf("FAIL: Extraction function returned -1.\n");
        }
        system("rm -rf /tmp/unpack test_setup archive_overwrite.zip");
    }

    return 0;
}