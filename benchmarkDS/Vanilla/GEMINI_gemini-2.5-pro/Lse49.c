#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> // For stat
#include <stdbool.h>

// Note: This implementation requires the 'zip' and 'unzip' command-line utilities
// to be installed and available in the system's PATH. This is a common approach
// in C/C++ when a third-party library is not desired for simplicity.

/**
 * Unzips an archive using the system's 'unzip' command.
 * @param zip_path The path to the .zip file.
 * @param extract_to_dir The directory to extract the files into.
 */
void unzip(const char* zip_path, const char* extract_to_dir) {
    char command[512];

    // Create the destination directory if it doesn't exist
    snprintf(command, sizeof(command), "mkdir -p %s", extract_to_dir);
    system(command);
    
    // Build the unzip command
    // -o: overwrite files without prompting
    // -d: destination directory
    snprintf(command, sizeof(command), "unzip -o \"%s\" -d \"%s\"", zip_path, extract_to_dir);

    printf("Executing: %s\n", command);
    int result = system(command);
    if (result != 0) {
        fprintf(stderr, "Warning: system command exited with non-zero status: %d\n", result);
    }
}

// --- Test Case Helper Functions ---

/**
 * Creates a test zip file using the system's 'zip' command.
 */
void create_test_zip(const char* zip_path, int test_num) {
    char command[512];
    char file_content[100];
    char dir_name[50];
    char file1_name[100];
    char file2_name[100];

    // Create dummy files to be zipped
    snprintf(dir_name, sizeof(dir_name), "temp_zip_dir_%d", test_num);
    snprintf(command, sizeof(command), "mkdir -p %s/subdir", dir_name);
    system(command);
    
    snprintf(file1_name, sizeof(file1_name), "%s/test_file_%d.txt", dir_name, test_num);
    snprintf(file2_name, sizeof(file2_name), "%s/subdir/test_file_%d.txt", dir_name, test_num);
    snprintf(file_content, sizeof(file_content), "This is content for test case %d", test_num);
    
    FILE* fp1 = fopen(file1_name, "w");
    if (fp1) {
        fputs(file_content, fp1);
        fclose(fp1);
    }

    FILE* fp2 = fopen(file2_name, "w");
    if (fp2) {
        fputs(file_content, fp2);
        fclose(fp2);
    }

    // Create zip archive
    snprintf(command, sizeof(command), "zip -r %s %s", zip_path, dir_name);
    system(command);
    
    // Clean up temp files
    snprintf(command, sizeof(command), "rm -rf %s", dir_name);
    system(command);
}

/**
 * Checks if a file exists.
 */
bool file_exists(const char* path) {
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

int main() {
    const char* zip_file_path = "archive.zip";
    const char* extract_dir = "/tmp/unpack";
    char command[256];
    char expected_file1[256];
    char expected_file2[256];

    for (int i = 1; i <= 5; ++i) {
        printf("--- Running Test Case %d ---\n", i);
        bool success = true;

        // 1. Setup
        snprintf(command, sizeof(command), "rm -f %s", zip_file_path);
        system(command);
        snprintf(command, sizeof(command), "rm -rf %s", extract_dir);
        system(command);

        create_test_zip(zip_file_path, i);
        printf("Created %s for test %d\n", zip_file_path, i);

        // 2. Execute
        unzip(zip_file_path, extract_dir);

        // 3. Verify
        snprintf(expected_file1, sizeof(expected_file1), "%s/temp_zip_dir_%d/test_file_%d.txt", extract_dir, i, i);
        snprintf(expected_file2, sizeof(expected_file2), "%s/temp_zip_dir_%d/subdir/test_file_%d.txt", extract_dir, i, i);
        
        if (!file_exists(expected_file1) || !file_exists(expected_file2)) {
            fprintf(stderr, "Verification failed: Extracted files not found.\n");
            success = false;
        } else {
             FILE* f = fopen(expected_file1, "r");
             char content[100] = {0};
             char expected_content[100];
             snprintf(expected_content, sizeof(expected_content), "This is content for test case %d", i);
             
             if (f) {
                fread(content, 1, sizeof(content)-1, f);
                fclose(f);
                if (strcmp(content, expected_content) != 0) {
                    fprintf(stderr, "Verification failed: File content mismatch.\n");
                    success = false;
                }
             } else {
                fprintf(stderr, "Verification failed: Could not open extracted file for reading.\n");
                success = false;
             }
        }

        // 4. Cleanup
        snprintf(command, sizeof(command), "rm -f %s", zip_file_path);
        system(command);
        snprintf(command, sizeof(command), "rm -rf %s", extract_dir);
        system(command);

        printf("Test Case %d: %s\n", i, success ? "PASS" : "FAIL");
    }

    return 0;
}