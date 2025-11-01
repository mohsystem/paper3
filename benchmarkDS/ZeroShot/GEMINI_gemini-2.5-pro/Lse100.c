#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <libgen.h> // for dirname
#endif

/**
 * Checks if a given path points to an existing directory.
 * @param path The directory path to check.
 * @return True if the directory exists, false otherwise.
 */
bool directoryExists(const char* path) {
    #ifdef _WIN32
        DWORD attribs = GetFileAttributesA(path);
        return (attribs != INVALID_FILE_ATTRIBUTES && (attribs & FILE_ATTRIBUTE_DIRECTORY));
    #else
        struct stat info;
        if (stat(path, &info) != 0) {
            return false;
        }
        return (info.st_mode & S_IFDIR);
    #endif
}

/**
 * Atomically creates a file with secure permissions (owner read/write only).
 * @param filePath The path to the file to be created.
 * @return True on success, false on failure.
 */
bool createSecureFile(const char* filePath) {
    if (filePath == NULL || *filePath == '\0') {
        fprintf(stderr, "Error: File path cannot be null or empty.\n");
        return false;
    }
    
    // Security: Basic path traversal check
    if (strstr(filePath, "..") != NULL) {
        fprintf(stderr, "Error: Invalid file path (contains '..').\n");
        return false;
    }
    
    // Check if parent directory exists.
    // We must work on a copy because dirname() on POSIX and our manual
    // manipulation on Windows can modify the input string.
    char* filePathCopy = strdup(filePath);
    if (!filePathCopy) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return false;
    }

#ifdef _WIN32
    // Windows doesn't have a standard dirname, so we find the last slash manually.
    char* last_slash = strrchr(filePathCopy, '\\');
    char* last_fwd_slash = strrchr(filePathCopy, '/');
    if (last_fwd_slash > last_slash) last_slash = last_fwd_slash;
    
    if (last_slash != NULL) {
        *last_slash = '\0'; // Terminate string at slash to get dir path
        if (strlen(filePathCopy) > 0 && !directoryExists(filePathCopy)) {
            fprintf(stderr, "Error: Parent directory does not exist for path: %s\n", filePath);
            free(filePathCopy);
            return false;
        }
    }
#else
    char* dir = dirname(filePathCopy);
    if (strcmp(dir, ".") != 0 && strcmp(dir, "/") != 0 && !directoryExists(dir)) {
        fprintf(stderr, "Error: Parent directory does not exist for path: %s\n", filePath);
        free(filePathCopy);
        return false;
    }
#endif
    free(filePathCopy);

#ifdef _WIN32
    int pmode = _S_IREAD | _S_IWRITE;
    int fd;
    errno_t err = _sopen_s(&fd, filePath, _O_CREAT | _O_EXCL | _O_WRONLY, _SH_DENYNO, pmode);
    if (err != 0) {
        fprintf(stderr, "Error creating file '%s': %s\n", filePath, strerror(err));
        return false;
    }
    _close(fd);
#else
    mode_t mode = S_IRUSR | S_IWUSR; // 0600 permissions
    int flags = O_WRONLY | O_CREAT | O_EXCL;
    int fd = open(filePath, flags, mode);

    if (fd == -1) {
        fprintf(stderr, "Error creating file '%s': %s\n", filePath, strerror(errno));
        return false;
    }
    close(fd);
#endif

    printf("Successfully created secure file: %s\n", filePath);
    return true;
}

void runTestCase(int n, const char* path) {
    printf("\nTest Case %d: Create file '%s'.\n", n, path);
    createSecureFile(path);
}

int main() {
    printf("--- C Test Cases ---\n");

    // Test Case 1: Standard successful creation
    runTestCase(1, "important_config.c");
    
    // Test Case 2: Attempt to create a file that already exists
    runTestCase(2, "important_config.c");

    // Test Case 3: Create a file with a relative path
    #ifdef _WIN32
        runTestCase(3, ".\\another_config.c.cfg");
    #else
        runTestCase(3, "./another_config.c.cfg");
    #endif

    // Test Case 4: Attempt to create a file in a non-existent directory
    #ifdef _WIN32
        runTestCase(4, "non_existent_dir\\some_file.txt");
    #else
        runTestCase(4, "non_existent_dir/some_file.txt");
    #endif

    // Test Case 5: Another successful creation
    runTestCase(5, "config_3.c.dat");

    return 0;
}