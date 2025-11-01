#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

// Function to print file permissions from a mode_t value
void print_permissions_from_mode(mode_t mode) {
    char perms[11];
    strcpy(perms, "----------");
    if (S_ISDIR(mode)) perms[0] = 'd';
    if (S_ISLNK(mode)) perms[0] = 'l';

    if (mode & S_IRUSR) perms[1] = 'r';
    if (mode & S_IWUSR) perms[2] = 'w';
    if (mode & S_IXUSR) perms[3] = 'x';

    if (mode & S_IRGRP) perms[4] = 'r';
    if (mode & S_IWGRP) perms[5] = 'w';
    if (mode & S_IXGRP) perms[6] = 'x';

    if (mode & S_IROTH) perms[7] = 'r';
    if (mode & S_IWOTH) perms[8] = 'w';
    if (mode & S_IXOTH) perms[9] = 'x';
    
    printf("%s (octal: %04o)", perms, mode & 0777);
}

// Creates a file, writes content, sets restrictive permissions, and prints them.
int create_and_secure_file(const char* file_name, const char* content) {
    // Restrictive permissions: S_IRUSR | S_IWUSR is 0600 (owner read/write)
    mode_t permissions = S_IRUSR | S_IWUSR;
    
    // Atomically create a new file with restrictive permissions.
    // O_EXCL fails if the file already exists, preventing TOCTOU race conditions.
    int fd = open(file_name, O_WRONLY | O_CREAT | O_EXCL, permissions);
    
    if (fd == -1) {
        fprintf(stderr, "Error: Failed to create file %s: %s\n", file_name, strerror(errno));
        return 0; // Failure
    }
    
    size_t content_len = strlen(content);
    ssize_t bytes_written = write(fd, content, content_len);
    if (bytes_written == -1 || (size_t)bytes_written != content_len) {
        fprintf(stderr, "Error: Failed to write to file %s: %s\n", file_name, (bytes_written == -1 ? strerror(errno) : "Incomplete write"));
        close(fd);
        return 0; // Failure
    }
    printf("Successfully created and wrote to %s\n", file_name);

    // Use fstat on the file descriptor to securely get file status.
    struct stat file_stat;
    if (fstat(fd, &file_stat) == -1) {
        fprintf(stderr, "Warning: Could not fstat file descriptor: %s\n", strerror(errno));
    } else {
        printf("File permissions for %s: ", file_name);
        print_permissions_from_mode(file_stat.st_mode);
        printf("\n");
    }

    if (close(fd) == -1) {
        fprintf(stderr, "Error: Failed to close file %s: %s\n", file_name, strerror(errno));
        return 0; // Failure
    }

    return 1; // Success
}

int main(void) {
    const char* file_name = "secret.txt";
    const char* content = "U2FsdGVkX1/R+WzJcxgvX/Iw==";

    for (int i = 1; i <= 5; ++i) {
        printf("--- Test Case %d ---\n", i);
        if (create_and_secure_file(file_name, content)) {
            printf("Test case %d successful.\n", i);
        } else {
            fprintf(stderr, "Test case %d failed.\n", i);
        }

        // Cleanup for the next test case
        if (remove(file_name) != 0 && errno != ENOENT) {
            fprintf(stderr, "Failed to clean up file %s: %s\n", file_name, strerror(errno));
        }
        printf("\n");
    }
    
    return 0;
}