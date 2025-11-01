
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <io.h>
#define chmod _chmod
#endif

void createSecureFile(const char* filename, const char* content) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Error: Could not create file %s\\n", filename);
        return;
    }
    
    // Write content to file
    fprintf(file, "%s", content);
    fclose(file);
    
    // Set restrictive permissions (owner read/write only - 0600)
#ifdef _WIN32
    // Windows: use _chmod with limited options
    chmod(filename, _S_IREAD | _S_IWRITE);
    printf("File permissions set to owner-only (Windows): %s\\n", filename);
#else
    // Unix/Linux: use chmod with full POSIX permissions
    chmod(filename, S_IRUSR | S_IWUSR);
    
    // Print permissions
    struct stat fileStat;
    if (stat(filename, &fileStat) == 0) {
        printf("File permissions: ");
        printf("%c", (fileStat.st_mode & S_IRUSR) ? 'r' : '-');
        printf("%c", (fileStat.st_mode & S_IWUSR) ? 'w' : '-');
        printf("%c", (fileStat.st_mode & S_IXUSR) ? 'x' : '-');
        printf("%c", (fileStat.st_mode & S_IRGRP) ? 'r' : '-');
        printf("%c", (fileStat.st_mode & S_IWGRP) ? 'w' : '-');
        printf("%c", (fileStat.st_mode & S_IXGRP) ? 'x' : '-');
        printf("%c", (fileStat.st_mode & S_IROTH) ? 'r' : '-');
        printf("%c", (fileStat.st_mode & S_IWOTH) ? 'w' : '-');
        printf("%c", (fileStat.st_mode & S_IXOTH) ? 'x' : '-');
        printf(" (%s)\\n", filename);
    }
#endif
}

int main() {
    // Test case 1: Create secret.txt with encrypted data
    createSecureFile("secret.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    
    // Test case 2: Create another secure file
    createSecureFile("test1.txt", "SecureData123");
    
    // Test case 3: Create file with different content
    createSecureFile("test2.txt", "ConfidentialInfo");
    
    // Test case 4: Create file with empty content
    createSecureFile("test3.txt", "");
    
    // Test case 5: Create file with multi-line content
    createSecureFile("test4.txt", "Line1\\nLine2\\nLine3");
    
    return 0;
}
