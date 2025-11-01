
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

void createAndSecureFile(const char* filename, const char* content) {
    FILE* file;
    
    // Create file and write content
    file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Error: Could not create file %s\\n", filename);
        return;
    }
    fprintf(file, "%s", content);
    fclose(file);
    
    // Change permissions to restrictive
#ifdef _WIN32
    // Windows implementation
    if (_chmod(filename, _S_IREAD | _S_IWRITE) != 0) {
        fprintf(stderr, "Error: Could not change permissions\\n");
        return;
    }
    printf("Permissions for %s: Owner read/write only\\n", filename);
#else
    // Unix/Linux implementation (0600 - owner read/write only)
    if (chmod(filename, S_IRUSR | S_IWUSR) != 0) {
        fprintf(stderr, "Error: Could not change permissions\\n");
        return;
    }
    
    // Print permissions
    struct stat fileStat;
    if (stat(filename, &fileStat) == 0) {
        printf("Permissions for %s: %o\\n", filename, fileStat.st_mode & 0777);
    }
#endif
}

int main() {
    // Test case 1: Create secret.txt with encrypted content
    printf("Test Case 1:\\n");
    createAndSecureFile("secret.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    printf("\\n");
    
    // Test case 2: Create another secure file
    printf("Test Case 2:\\n");
    createAndSecureFile("test1.txt", "Confidential Data");
    printf("\\n");
    
    // Test case 3: Create file with different content
    printf("Test Case 3:\\n");
    createAndSecureFile("test2.txt", "Sensitive Information");
    printf("\\n");
    
    // Test case 4: Create file with empty content
    printf("Test Case 4:\\n");
    createAndSecureFile("test3.txt", "");
    printf("\\n");
    
    // Test case 5: Create file with special characters
    printf("Test Case 5:\\n");
    createAndSecureFile("test4.txt", "Special@#$%^&*()");
    printf("\\n");
    
    return 0;
}
