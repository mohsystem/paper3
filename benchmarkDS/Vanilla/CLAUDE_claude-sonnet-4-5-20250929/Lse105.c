
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

void createSecretFile(const char* filename, const char* content) {
    // Write content to file
    FILE* file = fopen(filename, "w");
    if (file != NULL) {
        fputs(content, file);
        fclose(file);
        
        // Set restrictive permissions (owner read/write only - 0600)
        #ifdef _WIN32
            _chmod(filename, _S_IREAD | _S_IWRITE);
        #else
            chmod(filename, S_IRUSR | S_IWUSR);
        #endif
    } else {
        fprintf(stderr, "Unable to open file: %s\\n", filename);
    }
}

int main() {
    // Test case 1
    createSecretFile("secret.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    
    // Test case 2
    createSecretFile("secret2.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    
    // Test case 3
    createSecretFile("secret3.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    
    // Test case 4
    createSecretFile("secret4.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    
    // Test case 5
    createSecretFile("secret5.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    
    printf("Files created with restrictive permissions\\n");
    
    return 0;
}
