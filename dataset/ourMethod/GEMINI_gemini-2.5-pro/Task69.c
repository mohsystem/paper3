#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <direct.h>
#define rmdir _rmdir
#define mkdir(path, mode) _mkdir(path)
#define lstat(path, buf) _stat(path, buf)
#define S_ISLNK(m) 0 // No native, simple symlink support for this example
#else
#include <unistd.h>
#include <sys/stat.h>
#endif

/**
 * @brief Deletes a file at the given path after performing security checks.
 *        It will not delete directories or symbolic links.
 *
 * @param file_path The path to the file to be deleted.
 * @return 0 on success, -1 on failure.
 */
int deleteFile(const char* file_path) {
    if (file_path == NULL || file_path[0] == '\0') {
        fprintf(stderr, "Error: File path cannot be null or empty.\n");
        return -1;
    }

    struct stat statbuf;

    // Use lstat to check file type without following symlinks (on POSIX).
    // This is a key part of mitigating TOCTOU vulnerabilities.
    if (lstat(file_path, &statbuf) != 0) {
        fprintf(stderr, "Error: Cannot access path: %s. %s\n", file_path, strerror(errno));
        return -1;
    }

#if !defined(_WIN32) && !defined(_WIN64)
    if (S_ISLNK(statbuf.st_mode)) {
        fprintf(stderr, "Error: Deleting symbolic links is not allowed. Path: %s\n", file_path);
        return -1;
    }
#endif

    if (S_ISDIR(statbuf.st_mode)) {
        fprintf(stderr, "Error: Path is a directory, not a file. Path: %s\n", file_path);
        return -1;
    }
    
    if (!S_ISREG(statbuf.st_mode)) {
        fprintf(stderr, "Error: Path is not a regular file. Path: %s\n", file_path);
        return -1;
    }

    if (remove(file_path) == 0) {
        printf("Successfully deleted file: %s\n", file_path);
        return 0;
    } else {
        fprintf(stderr, "Error: Failed to delete file %s. %s\n", file_path, strerror(errno));
        return -1;
    }
}

void runTest(const char* testName, const char* path) {
    printf("--- %s ---\n", testName);
    printf("Attempting to delete: %s\n", path);
    deleteFile(path);
    printf("\n");
}

int main(int argc, char* argv[]) {
    if (argc == 2) {
        return deleteFile(argv[1]) == 0 ? 0 : 1;
    } else {
        printf("Usage: %s <file_path>\n", argv[0]);
        printf("\n--- No file path provided, running built-in test cases ---\n");

        const char* test_file = "test_file_c.txt";
        const char* test_dir = "test_dir_c";
        const char* test_link = "test_link_c.txt";
        const char* non_existent_file = "non_existent_file.txt";
        int link_created = 0;
        
        remove(test_file);
        remove(test_link);
        rmdir(test_dir);

        FILE* f = fopen(test_file, "w");
        if (f) fclose(f);
        mkdir(test_dir, 0755);
        
#if !defined(_WIN32) && !defined(_WIN64)
        if (symlink(test_file, test_link) == 0) {
            link_created = 1;
        } else {
            fprintf(stderr, "Warning: Could not create symbolic link. %s\n", strerror(errno));
        }
#else
        fprintf(stdout, "Warning: Symbolic link test is skipped on Windows.\n");
#endif

        runTest("Test Case 1: Delete existing file", test_file);
        runTest("Test Case 2: Delete non-existent file", non_existent_file);
        runTest("Test Case 3: Delete a directory", test_dir);
        if (link_created) {
            runTest("Test Case 4: Delete a symbolic link", test_link);
        } else {
            printf("--- Skipping Test Case 4: Symbolic link was not created ---\n\n");
        }
        runTest("Test Case 5: Re-delete already deleted file", test_file);

        remove(test_file);
        remove(test_link);
        rmdir(test_dir);
        printf("--- Test cleanup complete ---\n");

        return 0;
    }
}