
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void getent_passwd(const char* username, char* output, size_t output_size) {
    if (username == NULL || strlen(username) == 0) {
        snprintf(output, output_size, "Error: Username cannot be empty");
        return;
    }
    
    // Construct the command
    char command[256];
    snprintf(command, sizeof(command), "getent passwd %s", username);
    
    // Open pipe to execute command
    FILE* pipe = popen(command, "r");
    if (pipe == NULL) {
        snprintf(output, output_size, "Error: Failed to execute command");
        return;
    }
    
    // Read the output
    size_t bytes_read = 0;
    size_t remaining = output_size - 1;
    
    while (remaining > 0 && fgets(output + bytes_read, remaining, pipe) != NULL) {
        size_t len = strlen(output + bytes_read);
        bytes_read += len;
        remaining -= len;
    }
    
    output[bytes_read] = '\\0';
    
    // Remove trailing newline if present
    if (bytes_read > 0 && output[bytes_read - 1] == '\\n') {
        output[bytes_read - 1] = '\\0';
    }
    
    // Close the pipe
    pclose(pipe);
    
    if (strlen(output) == 0) {
        snprintf(output, output_size, "Error: User not found or command failed");
    }
}

int main() {
    // Test cases
    const char* test_users[] = {"root", "nobody", "daemon", "bin", "sys"};
    char output[512];
    
    printf("Testing getent passwd command:\\n\\n");
    
    for (int i = 0; i < 5; i++) {
        printf("User: %s\\n", test_users[i]);
        getent_passwd(test_users[i], output, sizeof(output));
        printf("%s\\n", output);
        printf("---\\n");
    }
    
    return 0;
}
