#include <stdio.h>
#include <stdlib.h>

// For chmod on POSIX-compliant systems (Linux, macOS)
#include <sys/stat.h>

/**
 * Creates a file, writes secret info to it, and sets permissions to owner-read-only.
 *
 * @param fileName The name of the file to create.
 * @param secretInfo The secret string to write to the file.
 * @return 0 on success, -1 on failure.
 */
int createAndSecureFile(const char* fileName, const char* secretInfo) {
    FILE *fp = NULL;

    // 1. Create and write to the file
    fp = fopen(fileName, "w");
    if (fp == NULL) {
        perror("Error opening file for writing");
        return -1;
    }

    if (fputs(secretInfo, fp) == EOF) {
        perror("Error writing to file");
        fclose(fp); // Clean up
        remove(fileName); // Attempt to remove partially written file
        return -1;
    }

    if (fclose(fp) != 0) {
        perror("Error closing file");
        return -1;
    }

    // 2. Change permissions to read-only for the owner (0400).
    // This uses chmod, which is specific to POSIX systems. For Windows,
    // you would need to use _chmod and different permission constants.
    if (chmod(fileName, S_IRUSR) != 0) {
        perror("Error changing file permissions");
        remove(fileName); // Attempt to remove file if permissions fail
        return -1;
    }

    printf("Successfully created and secured file: %s\n", fileName);
    return 0; // Success
}

int main() {
    // 5 test cases
    printf("--- C Test Cases ---\n");
    createAndSecureFile("secret_alpha.txt", "The launch code is 1234.");
    createAndSecureFile("secret_beta.log", "Meeting at midnight.");
    createAndSecureFile("secret_gamma.dat", "Alpha protocol is a go.");
    createAndSecureFile("top_secret.conf", "Password=SuperSecret!@#");
    createAndSecureFile("confidential_notes.txt", "This is a test of the emergency secret system.");
    printf("--------------------\n\n");

    return 0;
}