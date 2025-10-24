
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <limits.h>

#define MAX_CONTENT_SIZE 1048576  // 1MB max file size

char* read_file_content(const char* file_path) {
    FILE* file = NULL;
    char* content = NULL;
    char resolved_path[PATH_MAX];
    struct stat st;
    long file_size;
    size_t read_size;
    
    // Resolve the canonical path to prevent directory traversal
    if (realpath(file_path, resolved_path) == NULL) {
        content = (char*)malloc(100);
        snprintf(content, 100, "Error: Unable to resolve path - %s", strerror(errno));
        return content;
    }
    
    // Check if file exists and get file info
    if (stat(resolved_path, &st) != 0) {
        content = (char*)malloc(100);
        snprintf(content, 100, "Error: File does not exist or cannot be accessed");
        return content;
    }
    
    // Check if it's a regular file\n    if (!S_ISREG(st.st_mode)) {\n        content = (char*)malloc(100);\n        snprintf(content, 100, "Error: Path is not a regular file");\n        return content;\n    }\n    \n    // Open file\n    file = fopen(resolved_path, "r");\n    if (file == NULL) {\n        content = (char*)malloc(100);\n        snprintf(content, 100, "Error: Unable to open file - %s", strerror(errno));\n        return content;\n    }\n    \n    // Get file size\n    fseek(file, 0, SEEK_END);\n    file_size = ftell(file);\n    fseek(file, 0, SEEK_SET);\n    \n    // Check file size limit\n    if (file_size > MAX_CONTENT_SIZE) {\n        fclose(file);\n        content = (char*)malloc(100);\n        snprintf(content, 100, "Error: File too large");\n        return content;\n    }\n    \n    // Allocate memory for content\n    content = (char*)malloc(file_size + 1);\n    if (content == NULL) {\n        fclose(file);\n        content = (char*)malloc(100);\n        snprintf(content, 100, "Error: Memory allocation failed");\n        return content;\n    }\n    \n    // Read file content\n    read_size = fread(content, 1, file_size, file);\n    content[read_size] = '\\0';
    
    fclose(file);
    return content;
}

int main(int argc, char* argv[]) {
    char* test_files[] = {
        "test1.txt",
        "test2.txt",
        "nonexistent.txt",
        "../etc/passwd",
        "test3.txt"
    };
    int num_tests = 5;
    
    if (argc > 1) {
        char* content = read_file_content(argv[1]);
        printf("File Content:\\n%s\\n", content);
        free(content);
    } else {
        printf("Running test cases:\\n\\n");
        for (int i = 0; i < num_tests; i++) {
            printf("Test Case %d: %s\\n", i + 1, test_files[i]);
            char* content = read_file_content(test_files[i]);
            printf("%s\\n", content);
            printf("-------------------\\n\\n");
            free(content);
        }
    }
    
    return 0;
}
