
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

void writeToFile(const char* filename, const char* content) {
    // Open file with O_RDWR (read/write) and O_CREAT (create if not exists)
    // 0644 sets file permissions (owner read/write, group read, others read)
    int fd = open(filename, O_RDWR | O_CREAT, 0644);
    
    if (fd == -1) {
        fprintf(stderr, "Error opening file: %s\\n", filename);
        return;
    }
    
    // Write the content to the file
    ssize_t bytes_written = write(fd, content, strlen(content));
    
    if (bytes_written == -1) {
        fprintf(stderr, "Error writing to file: %s\\n", filename);
    } else {
        printf("Successfully wrote to file: %s\\n", filename);
    }
    
    // Close the file descriptor
    close(fd);
}

int main() {
    // Test case 1
    writeToFile("config1.txt", "important_config");
    
    // Test case 2
    writeToFile("config2.txt", "database_settings=localhost:5432");
    
    // Test case 3
    writeToFile("config3.txt", "api_key=secret123");
    
    // Test case 4
    writeToFile("config4.txt", "timeout=30");
    
    // Test case 5
    writeToFile("config5.txt", "debug_mode=true");
    
    return 0;
}
