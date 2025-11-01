
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <io.h>
#define chmod _chmod
#endif

char* createSecretFile(const char* secretInfo, const char* filename) {
    // Write the secret info to file
    FILE* file = fopen(filename, "w");
    if (file != NULL) {
        fprintf(file, "%s", secretInfo);
        fclose(file);
        
        // Change permissions to read-only for owner
#ifdef _WIN32
        chmod(filename, _S_IREAD);
#else
        chmod(filename, S_IRUSR);
#endif
        
        return (char*)filename;
    }
    return NULL;
}

int main() {
    // Test case 1
    char* result1 = createSecretFile("Super secret password: 12345", "secret1.txt");
    printf("%s\\n", result1);
    
    // Test case 2
    char* result2 = createSecretFile("Top secret data: classified", "secret2.txt");
    printf("%s\\n", result2);
    
    // Test case 3
    char* result3 = createSecretFile("Hidden treasure location: X marks the spot", "secret3.txt");
    printf("%s\\n", result3);
    
    // Test case 4
    char* result4 = createSecretFile("API Key: abc123xyz789", "secret4.txt");
    printf("%s\\n", result4);
    
    // Test case 5
    char* result5 = createSecretFile("Secret formula: H2O + C6H12O6", "secret5.txt");
    printf("%s\\n", result5);
    
    return 0;
}
