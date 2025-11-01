
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

void createAndSecureFile(const char* filename, const char* content) {
    // Create file and write content
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error: Could not create file\\n");
        return;
    }
    fprintf(file, "%s", content);
    fclose(file);
    
    // Change permissions to restrictive (owner read/write only: 0600)
    #ifdef _WIN32
        printf("Windows does not support POSIX permissions\\n");
    #else
        if (chmod(filename, S_IRUSR | S_IWUSR) != 0) {
            printf("Error: Could not change permissions\\n");
            return;
        }
        
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
            printf("\\n");
        }
    #endif
}

int main() {
    // Test cases
    printf("Test Case 1:\\n");
    createAndSecureFile("secret.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    
    printf("\\nTest Case 2:\\n");
    createAndSecureFile("secret2.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    
    printf("\\nTest Case 3:\\n");
    createAndSecureFile("secret3.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    
    printf("\\nTest Case 4:\\n");
    createAndSecureFile("secret4.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    
    printf("\\nTest Case 5:\\n");
    createAndSecureFile("secret5.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    
    return 0;
}
