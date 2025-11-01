#include <stdio.h>
#include <string.h>

// Platform-specific includes and definitions for changing file permissions
#ifdef _WIN32
    #include <io.h>
    #include <sys/stat.h> // for _S_IREAD, _S_IWRITE
#else
    #include <sys/stat.h> // for chmod, S_IRUSR, S_IWUSR
#endif

/**
 * @brief Creates a file, writes content to it, and sets restrictive permissions.
 *
 * @param filename The name of the file to create.
 * @param content The string content to write to the file.
 */
void createAndSecureFile(const char* filename, const char* content) {
    // 1. Write the string to the file
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening file for writing");
        return;
    }
    fprintf(file, "%s", content);
    fclose(file);
    printf("Successfully created and wrote to %s\n", filename);

    // 2. Change permissions to be restrictive (owner read/write)
    int result;
#ifdef _WIN32
    result = _chmod(filename, _S_IREAD | _S_IWRITE);
#else
    // S_IRUSR | S_IWUSR corresponds to 0600 permissions
    result = chmod(filename, S_IRUSR | S_IWUSR);
#endif

    if (result == 0) {
        printf("Set permissions for %s to restrictive.\n", filename);
    } else {
        perror("Error setting file permissions");
    }
}

int main() {
    const char* content = "U2FsdGVkX1/R+WzJcxgvX/Iw==";
    
    // 5 test cases, creating 5 different files.
    for (int i = 1; i <= 5; ++i) {
        char filename[20];
        snprintf(filename, sizeof(filename), "secret%d.txt", i);
        printf("\n--- Test Case %d: Creating %s ---\n", i, filename);
        createAndSecureFile(filename, content);
    }
    return 0;
}