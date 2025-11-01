
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <io.h>
#define chmod _chmod
#endif

void createSecretFile(const char* filename, const char* content) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Error: Could not create file %s\\n", filename);
        return;
    }
    
    // Write content to file
    fprintf(file, "%s", content);
    fclose(file);
    
    // Set restrictive permissions (owner read/write only: 0600)
    #ifdef _WIN32
    if (chmod(filename, _S_IREAD | _S_IWRITE) != 0) {
    #else
    if (chmod(filename, S_IRUSR | S_IWUSR) != 0) {
    #endif
        fprintf(stderr, "Error: Could not set file permissions\\n");
        return;
    }
    
    printf("File '%s' created successfully with restrictive permissions.\\n", filename);
}

int main() {
    // Test case 1: Standard execution
    createSecretFile("secret.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    
    // Test case 2: Different filename
    createSecretFile("secret2.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    
    // Test case 3: Empty content
    createSecretFile("secret3.txt", "");
    
    // Test case 4: Longer content
    createSecretFile("secret4.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==U2FsdGVkX1/R+WzJcxgvX/Iw==");
    
    // Test case 5: Special characters in content
    createSecretFile("secret5.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==!@#$%");
    
    return 0;
}
