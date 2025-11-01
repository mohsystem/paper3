
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

/**
 * Opens a file with O_CREAT | O_TRUNC | O_WRONLY | O_EXCL flags
 * Creates file if it doesn't exist, fails if it exists
 */
int openFileExclusive(const char* filename) {
    /* O_CREAT | O_EXCL | O_WRONLY | O_TRUNC */
    int fd = open(filename, O_CREAT | O_EXCL | O_WRONLY | O_TRUNC, 0644);
    if (fd == -1) {
        return -1; /* Failure */
    }
    close(fd);
    return 0; /* Success */
}

/**
 * Opens a file with O_CREAT | O_TRUNC | O_WRONLY flags
 * Creates or truncates existing file
 */
int openFileTruncate(const char* filename) {
    /* O_CREAT | O_WRONLY | O_TRUNC */
    int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd == -1) {
        return -1; /* Failure */
    }
    close(fd);
    return 0; /* Success */
}

int main() {
    printf("Test Case 1: Create new file with exclusive flag\\n");
    int result1 = openFileExclusive("test1.txt");
    printf("Result: %s\\n", result1 == 0 ? "SUCCESS" : "FAILED");

    printf("\\nTest Case 2: Try to create existing file with exclusive flag (should fail)\\n");
    int result2 = openFileExclusive("test1.txt");
    printf("Result: %s\\n", result2 == -1 ? "FAILED AS EXPECTED" : "UNEXPECTED SUCCESS");

    printf("\\nTest Case 3: Create new file with truncate flag\\n");
    int result3 = openFileTruncate("test2.txt");
    printf("Result: %s\\n", result3 == 0 ? "SUCCESS" : "FAILED");

    printf("\\nTest Case 4: Truncate existing file\\n");
    int result4 = openFileTruncate("test2.txt");
    printf("Result: %s\\n", result4 == 0 ? "SUCCESS" : "FAILED");

    printf("\\nTest Case 5: Create file in subdirectory\\n");
    mkdir("testdir", 0755);
    int result5 = openFileTruncate("testdir/test3.txt");
    printf("Result: %s\\n", result5 == 0 ? "SUCCESS" : "FAILED");

    /* Cleanup */
    unlink("test1.txt");
    unlink("test2.txt");
    unlink("testdir/test3.txt");
    rmdir("testdir");

    return 0;
}
