
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

void writeConfigToFile(const char* filename, const char* content) {
    if (filename == NULL || strlen(filename) == 0) {
        fprintf(stderr, "Error: Filename cannot be NULL or empty\\n");
        return;
    }
    
    if (content == NULL) {
        content = "";
    }
    
    // Open file with O_RDWR (read/write), O_CREAT (create if not exists)
    // Set permissions to 0600 (owner read/write only)
    int fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    
    if (fd == -1) {
        fprintf(stderr, "Error opening file: %s\\n", strerror(errno));
        return;
    }
    
    // Write content to file
    size_t content_length = strlen(content);
    ssize_t bytes_written = write(fd, content, content_length);
    
    if (bytes_written == -1) {
        fprintf(stderr, "Error writing to file: %s\\n", strerror(errno));
    } else if ((size_t)bytes_written != content_length) {
        fprintf(stderr, "Warning: Not all bytes were written\\n");
    } else {
        printf("Successfully wrote to file: %s\\n", filename);
    }
    
    // Always close the file descriptor
    if (close(fd) == -1) {
        fprintf(stderr, "Error closing file: %s\\n", strerror(errno));
    }
}

int main() {
    printf("Test Case 1: Normal write\\n");
    writeConfigToFile("config1.txt", "important_config");
    
    printf("\\nTest Case 2: Empty content\\n");
    writeConfigToFile("config2.txt", "");
    
    printf("\\nTest Case 3: Long content\\n");
    writeConfigToFile("config3.txt", "important_config_with_more_data_12345");
    
    printf("\\nTest Case 4: Special characters\\n");
    writeConfigToFile("config4.txt", "important_config\\nline2\\ttab");
    
    printf("\\nTest Case 5: Overwrite existing\\n");
    writeConfigToFile("config1.txt", "important_config_updated");
    
    return 0;
}
