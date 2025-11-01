
#include <stdio.h>
#include <string.h>

void writeToFile(const char* filename, const char* content) {
    FILE* file = fopen(filename, "w");
    if (file != NULL) {
        fprintf(file, "%s", content);
        fclose(file);
    } else {
        fprintf(stderr, "Error opening file: %s\\n", filename);
    }
}

int main() {
    // Test case 1
    writeToFile("important_config", "important_config");
    
    // Test case 2
    writeToFile("test_file1.txt", "test content 1");
    
    // Test case 3
    writeToFile("test_file2.txt", "test content 2");
    
    // Test case 4
    writeToFile("config.txt", "configuration data");
    
    // Test case 5
    writeToFile("data.txt", "sample data");
    
    printf("Files written successfully!\\n");
    
    return 0;
}
