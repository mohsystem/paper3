
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

void display_file_contents(const char* filename) {
    if (filename == NULL || strlen(filename) == 0) {
        printf("Error: Filename cannot be empty\\n");
        return;
    }
    
    // Sanitize input - remove dangerous characters
    char sanitized[256];
    int j = 0;
    const char* dangerous = ";&|`$(){}[]<>'\\"";
    
    for (int i = 0; i < strlen(filename) && j < 255; i++) {
        if (strchr(dangerous, filename[i]) == NULL) {
            sanitized[j++] = filename[i];
        }
    }
    sanitized[j] = '\\0';
    
    // Prevent path traversal
    if (strstr(sanitized, "..") != NULL) {
        printf("Error: Path traversal detected\\n");
        return;
    }
    
    // Check if file exists
    struct stat buffer;
    if (stat(sanitized, &buffer) != 0) {
        printf("Error: File does not exist\\n");
        return;
    }
    
    // Check if it's a regular file
    if (!S_ISREG(buffer.st_mode)) {
        printf("Error: Not a valid file\\n");
        return;
    }
    
    // Read file securely (no system command)
    FILE* file = fopen(sanitized, "r");
    if (file == NULL) {
        printf("Error: Cannot open file\\n");
        return;
    }
    
    printf("Contents of %s:\\n", filename);
    printf("-----------------------------------\\n");
    
    char line[1024];
    while (fgets(line, sizeof(line), file) != NULL) {
        printf("%s", line);
    }
    
    fclose(file);
}

int main() {
    // Test case 1: Valid text file
    printf("Test 1:\\n");
    display_file_contents("test1.txt");
    
    // Test case 2: Empty filename
    printf("\\nTest 2:\\n");
    display_file_contents("");
    
    // Test case 3: Path traversal attempt
    printf("\\nTest 3:\\n");
    display_file_contents("../../../etc/passwd");
    
    // Test case 4: Command injection attempt
    printf("\\nTest 4:\\n");
    display_file_contents("test.txt; rm -rf /");
    
    // Test case 5: Non-existent file
    printf("\\nTest 5:\\n");
    display_file_contents("nonexistent.txt");
    
    return 0;
}
