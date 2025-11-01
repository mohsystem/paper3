#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> // For chmod on POSIX systems

/**
 * Creates a file, writes content to it, and sets permissions to owner-read-only.
 * Note: This uses the POSIX `chmod` function and may not work as expected on 
 * non-POSIX systems like Windows.
 *
 * @param fileName The name of the file to create.
 * @param content The super secret info to write to the file.
 * @return 0 on success, -1 on failure.
 */
int createSecretFile(const char* fileName, const char* content) {
    // 1. & 2. Create a file and write the super secret info to it.
    FILE* fp = fopen(fileName, "w");
    if (fp == NULL) {
        perror("Error opening file for writing");
        return -1;
    }
    fprintf(fp, "%s", content);
    fclose(fp);

    // 3. Change the permissions of the file to read-only for the owner (0400).
    // S_IRUSR is the POSIX constant for owner read permission.
    if (chmod(fileName, S_IRUSR) != 0) {
        perror("Error changing file permissions");
        return -1;
    }

    // 4. Print the name of the file to the screen.
    printf("%s\n", fileName);

    return 0;
}

int main() {
    printf("--- Running C Test Cases ---\n");
    char fileName[50];
    char secretInfo[100];

    for (int i = 0; i < 5; ++i) {
        // Create dynamic file names and content for each test case
        sprintf(fileName, "c_secret_file_%d.txt", i);
        sprintf(secretInfo, "This is C secret #%d. The combination is 1-2-3-4.", i);
        
        printf("Created file: ");
        createSecretFile(fileName, secretInfo);
    }
    printf("--------------------------\n");
    return 0;
}