
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <io.h>
#define chmod _chmod
#endif

const char* createSecretFile(const char* secretInfo, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        return "Error: Could not open file";
    }
    
    fprintf(file, "%s", secretInfo);
    fclose(file);
    
    // Change permissions to read-only for owner
    #ifdef _WIN32
    chmod(filename, _S_IREAD);
    #else
    chmod(filename, S_IRUSR);
    #endif
    
    return filename;
}

int main() {
    // Test case 1
    const char* result1 = createSecretFile("Super secret password: admin123", "secret1.txt");
    printf("%s\\n", result1);
    
    // Test case 2
    const char* result2 = createSecretFile("API Key: abc123xyz789", "secret2.txt");
    printf("%s\\n", result2);
    
    // Test case 3
    const char* result3 = createSecretFile("Database credentials: user=root, pass=secret", "secret3.txt");
    printf("%s\\n", result3);
    
    // Test case 4
    const char* result4 = createSecretFile("Private key: RSA-2048-BEGIN", "secret4.txt");
    printf("%s\\n", result4);
    
    // Test case 5
    const char* result5 = createSecretFile("Social Security Number: 123-45-6789", "secret5.txt");
    printf("%s\\n", result5);
    
    return 0;
}
