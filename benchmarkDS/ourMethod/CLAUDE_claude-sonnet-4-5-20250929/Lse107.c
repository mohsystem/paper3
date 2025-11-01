
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

void createAndSecureFile(const char* filename, const char* content) {
    if (filename == NULL || content == NULL) {
        fprintf(stderr, "Invalid parameters\\n");
        return;
    }
    
    /* Create file with restrictive permissions (0600 - owner read/write only) */
    /* O_CREAT|O_EXCL ensures atomic creation and fails if file exists */
    /* O_WRONLY for write-only, O_CLOEXEC prevents descriptor leaks */
    int flags = O_CREAT | O_EXCL | O_WRONLY | O_CLOEXEC;
    #ifdef O_NOFOLLOW
    /* O_NOFOLLOW prevents following symlinks for security */
    flags |= O_NOFOLLOW;
    #endif
    
    mode_t mode = S_IRUSR | S_IWUSR;  /* 0600 - owner read/write only */
    
    /* Open file with secure flags and restrictive permissions from creation */
    int fd = open(filename, flags, mode);
    if (fd < 0) {
        fprintf(stderr, "Error creating file: %s\\n", strerror(errno));
        return;
    }
    
    /* Write content to file */
    size_t content_len = strlen(content);
    ssize_t written = write(fd, content, content_len);
    
    if (written < 0 || (size_t)written != content_len) {
        fprintf(stderr, "Error writing to file\\n");
        close(fd);
        return;
    }
    
    /* Ensure data is flushed to disk */
    if (fsync(fd) < 0) {
        fprintf(stderr, "Error syncing file\\n");
    }
    
    /* Use fstat on the open descriptor to avoid TOCTOU */
    struct stat file_stat;
    if (fstat(fd, &file_stat) < 0) {
        fprintf(stderr, "Error getting file stats\\n");
        close(fd);
        return;
    }
    
    /* Close the file descriptor */
    close(fd);
    
    /* Print permissions in octal format */
    mode_t perms = file_stat.st_mode & 0777;
    printf("File permissions (octal): 0%o\\n", perms);
    
    /* Print permissions in symbolic format */
    printf("File permissions (symbolic): ");
    printf("%c", (file_stat.st_mode & S_IRUSR) ? 'r' : '-');
    printf("%c", (file_stat.st_mode & S_IWUSR) ? 'w' : '-');
    printf("%c", (file_stat.st_mode & S_IXUSR) ? 'x' : '-');
    printf("%c", (file_stat.st_mode & S_IRGRP) ? 'r' : '-');
    printf("%c", (file_stat.st_mode & S_IWGRP) ? 'w' : '-');
    printf("%c", (file_stat.st_mode & S_IXGRP) ? 'x' : '-');
    printf("%c", (file_stat.st_mode & S_IROTH) ? 'r' : '-');
    printf("%c", (file_stat.st_mode & S_IWOTH) ? 'w' : '-');
    printf("%c", (file_stat.st_mode & S_IXOTH) ? 'x' : '-');
    printf("\\n");
}

int main(void) {
    /* Test case 1: Standard operation */
    printf("Test 1: Creating secret.txt\\n");
    unlink("secret.txt");  /* Remove if exists */
    createAndSecureFile("secret.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    
    /* Test case 2: Different filename */
    printf("\\nTest 2: Creating secret2.txt\\n");
    unlink("secret2.txt");
    createAndSecureFile("secret2.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    
    /* Test case 3: Different content */
    printf("\\nTest 3: Creating secret3.txt with different content\\n");
    unlink("secret3.txt");
    createAndSecureFile("secret3.txt", "TestContent123");
    
    /* Test case 4: Empty content */
    printf("\\nTest 4: Creating secret4.txt with empty content\\n");
    unlink("secret4.txt");
    createAndSecureFile("secret4.txt", "");
    
    /* Test case 5: Special characters */
    printf("\\nTest 5: Creating secret5.txt with special characters\\n");
    unlink("secret5.txt");
    createAndSecureFile("secret5.txt", "Special!@#$%^&*()");
    
    return 0;
}
