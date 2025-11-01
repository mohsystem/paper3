
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

void writeToFile(const char* filename, const char* content) {
    // Open file with restricted permissions (owner read/write only - 0600)
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        fprintf(stderr, "Error opening file: %s\\n", filename);
        return;
    }
    
    write(fd, content, strlen(content));
    close(fd);
}

int main() {
    // Test case 1: Write to important_config
    writeToFile("important_config", "important_config");
    
    // Test case 2: Write different content
    writeToFile("test_config_1", "test_content_1");
    
    // Test case 3: Write empty string
    writeToFile("test_config_2", "");
    
    // Test case 4: Write multi-line content
    writeToFile("test_config_3", "line1\\nline2\\nline3");
    
    // Test case 5: Write special characters
    writeToFile("test_config_4", "special!@#$%^&*()");
    
    printf("Files written successfully\\n");
    
    return 0;
}
