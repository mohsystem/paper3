#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// Use preprocessor directives to handle platform differences (POSIX vs Windows)
#ifdef _WIN32
#include <io.h> 
#include <fcntl.h> 
#include <sys/stat.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#endif

/**
 * @brief Creates a file with given content and restrictive permissions.
 * On POSIX systems, this is done atomically by setting mode 0600 on creation.
 * On Windows, it's a two-step process (create, then set permissions).
 * 
 * @param filename The name of the file to create.
 * @param content The string content to write to the file.
 * @return true on success, false on failure.
 */
bool createSecretFile(const char* filename, const char* content) {
#ifdef _WIN32
    // Windows implementation (less secure due to create-then-chmod)
    int fd = _open(filename, _O_WRONLY | _O_CREAT | _O_TRUNC, _S_IREAD | _S_IWRITE);
    if (fd == -1) {
        perror("Error opening/creating file on Windows");
        return false;
    }
    
    size_t content_len = strlen(content);
    if (_write(fd, content, content_len) != (int)content_len) {
        perror("Error writing to file on Windows");
        _close(fd);
        return false;
    }
    
    _close(fd);
    
    if (_chmod(filename, _S_IREAD | _S_IWRITE) != 0) {
        perror("Error setting file permissions on Windows");
        return false;
    }
    printf("Successfully created '%s' on Windows with basic permissions.\n", filename);
    return true;
#else
    // POSIX implementation (secure, atomic permission setting)
    // S_IRUSR: read permission for owner. S_IWUSR: write permission for owner. (0600)
    mode_t mode = S_IRUSR | S_IWUSR;
    
    // O_WRONLY: write-only. O_CREAT: create. O_TRUNC: truncate if exists.
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, mode);
    
    if (fd == -1) {
        perror("Error opening/creating file");
        return false;
    }
    
    size_t content_len = strlen(content);
    ssize_t bytes_written = write(fd, content, content_len);
    
    if (bytes_written == -1) {
        perror("Error writing to file");
        close(fd);
        return false;
    }
    
    if ((size_t)bytes_written != content_len) {
        fprintf(stderr, "Error: Incomplete write to file '%s'\n", filename);
        close(fd);
        return false;
    }
    
    if (close(fd) == -1) {
        perror("Error closing file");
        return false;
    }
    
    printf("Successfully created and secured '%s' with permissions 0600.\n", filename);
    return true;
#endif
}

int main() {
    const char* filename = "secret.txt";
    const char* content = "U2FsdGVkX1/R+WzJcxgvX/Iw==";

    for (int i = 1; i <= 5; ++i) {
        printf("--- Test Case %d ---\n", i);
        
        // Clean up before each test to ensure a fresh start
        remove(filename);

        bool success = createSecretFile(filename, content);
        printf("Test %d result: %s\n\n", i, success ? "Success" : "Failure");
    }

    return 0;
}