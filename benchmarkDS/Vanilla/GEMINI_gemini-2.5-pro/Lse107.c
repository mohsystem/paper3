#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

// Note: This code uses POSIX-specific functions (chmod, stat) and will work on
// Linux, macOS, and other Unix-like systems.

void manageSecretFile(const char* filename, const char* content) {
    printf("Processing file: %s\n", filename);
    FILE *fp = NULL;

    // 1. Create a file and write the string to it.
    fp = fopen(filename, "w");
    if (fp == NULL) {
        fprintf(stderr, "  - Error: Could not open file %s for writing: %s\n", filename, strerror(errno));
        return;
    }
    fprintf(fp, "%s", content);
    fclose(fp);
    printf("  - Successfully wrote to %s\n", filename);

    // 2. Change permissions to restrictive (owner read/write: 600).
    // S_IRUSR for user read, S_IWUSR for user write.
    if (chmod(filename, S_IRUSR | S_IWUSR) != 0) {
        fprintf(stderr, "  - Error: Could not change permissions for %s: %s\n", filename, strerror(errno));
    } else {
        printf("  - Set permissions to 0600 (rw-------)\n");
    }

    // 3. Print the permissions of the file.
    struct stat file_stat;
    if (stat(filename, &file_stat) != 0) {
        fprintf(stderr, "  - Error: Could not get status of file %s: %s\n", filename, strerror(errno));
    } else {
        // Mask with 0777 to get only the permission bits and print in octal.
        printf("  - Current permissions: 0%o\n", file_stat.st_mode & 0777);
    }
    
    // Clean up the created file for the next test case
    if (remove(filename) == 0) {
        printf("  - Cleaned up file: %s\n", filename);
    } else {
        fprintf(stderr, "  - Warning: Could not clean up file %s: %s\n", filename, strerror(errno));
    }
    printf("--------------------\n");
}

int main() {
    const char* content = "U2FsdGVkX1/R+WzJcxgvX/Iw==";

    // Test Case 1
    manageSecretFile("secret.txt", content);

    // Test Case 2
    manageSecretFile("secret_file_2.txt", content);
    
    // Test Case 3
    manageSecretFile("another_secret.log", content);
    
    // Test Case 4
    manageSecretFile("temp_secret_4.dat", content);
    
    // Test Case 5
    manageSecretFile("last_secret.txt", content);

    return 0;
}