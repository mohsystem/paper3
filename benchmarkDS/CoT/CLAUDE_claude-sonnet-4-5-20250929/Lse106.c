
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

void create_secure_file(const char* secret_info, const char* filename) {
    // Create file with restrictive permissions (owner read-only: 0400)
    int fd = open(filename, O_CREAT | O_WRONLY | O_EXCL, S_IRUSR);
    
    if (fd == -1) {
        printf("Error: Could not create file %s\\n", filename);
        return;
    }
    
    // Write secret info
    write(fd, secret_info, strlen(secret_info));
    close(fd);
    
    // Ensure permissions are set to read-only for owner
    chmod(filename, S_IRUSR);
    
    printf("%s\\n", filename);
}

int main() {
    // Test cases
    const char* test_cases[] = {
        "SuperSecret123!",
        "Confidential Data",
        "API_KEY=abc123xyz",
        "Password: P@ssw0rd!",
        "Token: eyJhbGciOiJIUzI1NiJ9"
    };
    
    for (int i = 0; i < 5; i++) {
        char filename[50];
        snprintf(filename, sizeof(filename), "secret_file_%d.txt", i + 1);
        create_secure_file(test_cases[i], filename);
    }
    
    return 0;
}
