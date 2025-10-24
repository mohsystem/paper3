#include <stdio.h>
#include <stdbool.h>
#include <sys/stat.h> // For mode constants and chmod
#include <fcntl.h>    // For open
#include <unistd.h>   // For close

/**
 * Creates a file and makes it executable (rwxr-xr-x).
 * This function is intended for POSIX-compliant systems (Linux, macOS).
 *
 * @param filename The name of the file to create.
 * @return true if successful, false otherwise.
 */
bool createAndMakeExecutable(const char* filename) {
    // For test repeatability, remove the file if it exists.
    remove(filename);
    
    // Step 1: Create the file using open(). This is more direct than fopen().
    // We pass O_CREAT to create it, O_WRONLY for writing, and O_TRUNC to clear it if it exists.
    // The third argument is the mode, which will be affected by the system's umask.
    int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);

    if (fd == -1) {
        perror("Error opening file");
        return false;
    }
    close(fd); // We just needed to create it, so close it immediately.

    // Step 2: Set explicit permissions to rwxr-xr-x (0755) using chmod().
    // This ensures the permissions are set correctly, regardless of the umask.
    mode_t mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
    if (chmod(filename, mode) != 0) {
        perror("Error setting file permissions");
        return false;
    }
    
    printf("Successfully created and set permissions for '%s'.\n", filename);
    return true;
}

int main() {
    // 5 test cases
    const char* testFiles[] = {
        "script1.sh",
        "script2.sh",
        "script3.sh",
        "script4.sh",
        "script5.sh"
    };
    int numTests = sizeof(testFiles) / sizeof(testFiles[0]);

    for (int i = 0; i < numTests; ++i) {
        printf("--- Testing: %s ---\n", testFiles[i]);
        createAndMakeExecutable(testFiles[i]);
        printf("\n");
    }

    return 0;
}