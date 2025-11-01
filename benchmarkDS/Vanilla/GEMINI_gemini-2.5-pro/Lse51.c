#include <stdio.h>
#include <stdlib.h> // For system()
#include <string.h>
#include <sys/stat.h> // For mkdir
#include <sys/types.h> // For mkdir

/**
 * Extracts a tar archive by calling the system's `tar` command.
 *
 * @param archive_path Path to the .tar, .tar.gz, or .tar.bz2 file.
 * @param dest_path Path to the destination directory.
 * @return 0 on success, non-zero on failure.
 */
int extract_archive(const char* archive_path, const char* dest_path) {
    // Create the destination directory if it doesn't exist
    // On Unix-like systems.
    mkdir(dest_path, 0755);

    // Create the command string. Buffer size should be large enough.
    char command[1024];
    // Use snprintf for safe string formatting to prevent buffer overflows.
    // tar -xf <archive> -C <destination>
    int written = snprintf(command, sizeof(command), "tar -xf \"%s\" -C \"%s\"", archive_path, dest_path);

    if (written < 0 || written >= sizeof(command)) {
        fprintf(stderr, "Error: Command string is too long.\n");
        return -1;
    }
    
    printf("Executing: %s\n", command);
    
    int status = system(command);
    
    if (status != 0) {
        fprintf(stderr, "Error: System command failed with exit code %d\n", status);
        return status;
    }
    
    return 0; // Success
}

int main() {
    printf("--- Running C Tests ---\n");
    printf("Note: These tests assume the Python script has been run to create test archives in /tmp/.\n");

    // Test Case 1: Extract a standard tar.gz archive
    printf("\n[Test 1] Extracting simple.tar.gz to /tmp/c_unpack_1\n");
    int status1 = extract_archive("/tmp/simple.tar.gz", "/tmp/c_unpack_1");
    printf("Test 1 Success: %s\n", (status1 == 0) ? "true" : "false");

    // Test Case 2: Extract a tar.bz2 archive
    printf("\n[Test 2] Extracting simple.tar.bz2 to /tmp/c_unpack_2\n");
    int status2 = extract_archive("/tmp/simple.tar.bz2", "/tmp/c_unpack_2");
    printf("Test 2 Success: %s\n", (status2 == 0) ? "true" : "false");

    // Test Case 3: Attempt to extract a non-existent file
    printf("\n[Test 3] Extracting non_existent.tar.gz to /tmp/c_unpack_3 (expecting error)\n");
    int status3 = extract_archive("/tmp/non_existent.tar.gz", "/tmp/c_unpack_3");
    printf("Test 3 Success: %s\n", (status3 == 0) ? "true" : "false");

    // Test Case 4: Extract an archive with subdirectories
    printf("\n[Test 4] Extracting nested.tar.gz to /tmp/c_unpack_4\n");
    int status4 = extract_archive("/tmp/nested.tar.gz", "/tmp/c_unpack_4");
    printf("Test 4 Success: %s\n", (status4 == 0) ? "true" : "false");

    // Test Case 5: Extract an empty archive
    printf("\n[Test 5] Extracting empty.tar.gz to /tmp/c_unpack_5\n");
    int status5 = extract_archive("/tmp/empty.tar.gz", "/tmp/c_unpack_5");
    printf("Test 5 Success: %s\n", (status5 == 0) ? "true" : "false");

    return 0;
}