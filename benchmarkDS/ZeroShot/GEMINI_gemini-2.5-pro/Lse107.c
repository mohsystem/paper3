#include <stdio.h>
#include <string.h>
#include <errno.h>

// Platform-specific includes and definitions for file operations
#ifdef _WIN32
    #include <io.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <share.h>      // For _SH_DENYNO
    // Use secure CRT functions on Windows
    #define SPRINTF sprintf_s
    #define STRERROR strerror_s
    #define REMOVE _unlink
#else // POSIX systems
    #include <unistd.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #define SPRINTF snprintf
    #define STRERROR strerror_r
    #define REMOVE remove
#endif

// Platform-specific permission flags
#ifdef _WIN32
    #define SECURE_PERMS (_S_IREAD | _S_IWRITE)
#else
    #define SECURE_PERMS (S_IRUSR | S_IWUSR) // 600 permissions
#endif

const char* CONTENT = "U2FsdGVkX1/R+WzJcxgvX/Iw==";

// Forward declarations
void print_permissions(const char* filename);
void manage_secret_file(const char* filename);
void cleanup(const char* filename);

/**
 * Creates a file securely with restrictive permissions, writes content,
 * and then prints the permissions. This function uses low-level file APIs
 * to set permissions atomically upon file creation.
 */
void manage_secret_file(const char* filename) {
    printf("Processing file: %s\n", filename);
    int fd = -1;

#ifdef _WIN32
    errno_t err = _sopen_s(&fd, filename, _O_WRONLY | _O_CREAT | _O_TRUNC, _SH_DENYNO, SECURE_PERMS);
    if (err != 0) {
        char err_buf[256];
        strerror_s(err_buf, sizeof(err_buf), errno);
        fprintf(stderr, "Error opening file %s: %s\n", filename, err_buf);
        return;
    }
#else
    fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, SECURE_PERMS);
    if (fd == -1) {
        fprintf(stderr, "Error opening file %s: %s\n", filename, strerror(errno));
        return;
    }
#endif

    ssize_t content_len = (ssize_t)strlen(CONTENT);
#ifdef _WIN32
    ssize_t bytes_written = _write(fd, CONTENT, (unsigned int)content_len);
#else
    ssize_t bytes_written = write(fd, CONTENT, content_len);
#endif

    if (bytes_written < 0 || bytes_written != content_len) {
        fprintf(stderr, "Error writing to file %s: %s\n", filename, strerror(errno));
    } else {
        printf("File created and content written.\n");
    }

#ifdef _WIN32
    _close(fd);
#else
    close(fd);
#endif

    print_permissions(filename);
    printf("---\n");
}

/**
 * Retrieves and prints the permissions of a given file.
 */
void print_permissions(const char* filename) {
#ifdef _WIN32
    struct _stat file_stat;
    if (_stat(filename, &file_stat) != 0) {
        fprintf(stderr, "Error stating file %s: %s\n", filename, strerror(errno));
        return;
    }
    printf("Current file permissions (owner): ");
    if (file_stat.st_mode & _S_IREAD) printf("Read ");
    if (file_stat.st_mode & _S_IWRITE) printf("Write");
    printf("\n");
#else
    struct stat file_stat;
    if (stat(filename, &file_stat) != 0) {
        fprintf(stderr, "Error stating file %s: %s\n", filename, strerror(errno));
        return;
    }
    // Print permissions in octal format (e.g., 600)
    printf("Current file permissions: %o\n", file_stat.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO));
#endif
}

/**
 * Deletes the specified file.
 */
void cleanup(const char* filename) {
    if (REMOVE(filename) != 0) {
        if (errno != ENOENT) { // Don't report an error if file just doesn't exist
            fprintf(stderr, "Error cleaning up file %s: %s\n", filename, strerror(errno));
        }
    }
}

int main() {
    for (int i = 1; i <= 5; ++i) {
        char test_filename[50];
        SPRINTF(test_filename, sizeof(test_filename), "secret_c_%d.txt", i);
        manage_secret_file(test_filename);
        cleanup(test_filename);
    }
    return 0;
}