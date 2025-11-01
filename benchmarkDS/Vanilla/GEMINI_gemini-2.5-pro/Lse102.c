#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

/**
 * Opens/creates a file with specific POSIX permissions and writes content to it.
 *
 * @param filename The name of the file to create/open.
 * @param content  The string content to write to the file.
 */
void createAndWriteFile(const char* filename, const char* content) {
    // Permissions: S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH (0644)
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    
    // Flags: O_RDWR (read/write), O_CREAT (create if it doesn't exist)
    // Add O_TRUNC to clear file content if it already exists before writing.
    int fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, mode);
    
    if (fd == -1) {
        perror("Error opening file");
        return;
    }
    
    size_t content_len = strlen(content);
    ssize_t bytes_written = write(fd, content, content_len);
    
    if (bytes_written == -1) {
        perror("Error writing to file");
    } else {
        printf("Successfully wrote %ld bytes to %s\n", (long)bytes_written, filename);
    }
    
    if (close(fd) == -1) {
        perror("Error closing file");
    }
}

int main() {
    printf("--- C Test Cases ---\n");
    // Test Case 1: As per the prompt
    createAndWriteFile("important_config", "important_config");
    // Test Case 2: A different file and content
    createAndWriteFile("c_test.log", "Log entry from C.");
    // Test Case 3: A file with empty content
    createAndWriteFile("c_empty.txt", "");
    // Test Case 4: Overwriting the first file
    createAndWriteFile("important_config", "new important_config content from C");
    // Test Case 5: Another new file
    createAndWriteFile("c_data.json", "{\"key\": \"value\"}");
    printf("--------------------\n");
    return 0;
}