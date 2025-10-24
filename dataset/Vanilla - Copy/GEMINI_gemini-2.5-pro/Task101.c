#include <stdio.h>
#include <stdlib.h>
// Required for chmod on POSIX systems (Linux, macOS, etc.)
#include <sys/stat.h>

/**
 * Creates a file with the given name, writes a simple shell script to it,
 * and makes it executable.
 * Note: This implementation uses chmod, which is specific to POSIX-compliant
 * operating systems and will not work on Windows.
 *
 * @param filename The name of the file to create.
 * @return 1 on success, 0 on failure.
 */
int createAndMakeExecutable(const char* filename) {
    // Create and write to the file
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) {
        perror("Error creating file");
        return 0; // Failure
    }
    fprintf(fp, "#!/bin/bash\n");
    fprintf(fp, "echo \"Hello from '%s'!\"\n", filename);
    fclose(fp);
    printf("File created: %s\n", filename);

    // Set permissions to rwxr-xr-x (0755)
    if (chmod(filename, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0) {
        perror("Error setting file permissions");
        return 0; // Failure
    }

    printf("File permissions set to executable.\n");
    return 1; // Success
}

int main() {
    // 5 Test Cases
    const char* testFiles[] = {"script.sh", "test_script_1.sh", "test_script_2.sh", "another.sh", "final.sh"};
    int numTests = sizeof(testFiles) / sizeof(testFiles[0]);

    for (int i = 0; i < numTests; i++) {
        printf("--- Test Case %d: %s ---\n", i + 1, testFiles[i]);
        int success = createAndMakeExecutable(testFiles[i]);
        printf("Result: %s\n\n", success ? "SUCCESS" : "FAILURE");
    }

    return 0;
}