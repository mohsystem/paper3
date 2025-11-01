
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <io.h>
#define chmod _chmod
#endif

void createSecretFile(const char* filename, const char* content) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Error: Could not open file %s\\n", filename);
        return;
    }
    
    fprintf(file, "%s", content);
    fclose(file);
    
    // Set restrictive permissions (owner read/write only - 0600)
#ifdef _WIN32
    chmod(filename, _S_IREAD | _S_IWRITE);
#else
    chmod(filename, S_IRUSR | S_IWUSR);
#endif
}

int main() {
    // Test case 1: Default secret.txt
    createSecretFile("secret.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    
    // Test case 2: Different filename
    createSecretFile("secret2.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    
    // Test case 3: Different content
    createSecretFile("secret3.txt", "U2FsdGVkX1/TestData123==");
    
    // Test case 4: Another test file
    createSecretFile("secret4.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    
    // Test case 5: Fifth test file
    createSecretFile("secret5.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    
    printf("Secret files created with restrictive permissions\\n");
    
    return 0;
}
