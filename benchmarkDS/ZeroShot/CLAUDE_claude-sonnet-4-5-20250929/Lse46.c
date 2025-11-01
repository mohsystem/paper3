
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#define MAX_PATH 256
#define MAX_FILENAME 128

void sanitize_filename(const char* input, char* output, size_t output_size) {
    size_t j = 0;
    for (size_t i = 0; i < strlen(input) && j < output_size - 1; i++) {
        if (isalnum(input[i]) || input[i] == '.' || input[i] == '_' || input[i] == '-') {
            output[j++] = input[i];
        } else {
            output[j++] = '_';
        }
    }
    output[j] = '\\0';
}

char* img(const char* name) {
    char sanitized_name[MAX_FILENAME];
    char filepath[MAX_PATH];
    static char content[4096];
    
    // Sanitize filename
    sanitize_filename(name, sanitized_name, MAX_FILENAME);
    
    // Construct file path
    snprintf(filepath, MAX_PATH, "images/%s", sanitized_name);
    
    // Check if file path is safe (basic check)
    if (strstr(filepath, "..") != NULL) {
        strcpy(content, "Error: Forbidden access");
        return content;
    }
    
    // Open file
    FILE* file = fopen(filepath, "rb");
    if (file == NULL) {
        strcpy(content, "Error: File not found");
        return content;
    }
    
    // Read file content
    size_t bytes_read = fread(content, 1, sizeof(content) - 1, file);
    content[bytes_read] = '\\0';
    
    fclose(file);
    return content;
}

void create_test_files() {
    #ifdef _WIN32
    _mkdir("images");
    #else
    mkdir("images", 0755);
    #endif
    
    const char* test_files[] = {"test1.jpg", "test2.png", "test3.gif", "test4.jpeg", "test5.jpg"};
    
    for (int i = 0; i < 5; i++) {
        char filepath[MAX_PATH];
        snprintf(filepath, MAX_PATH, "images/%s", test_files[i]);
        
        FILE* file = fopen(filepath, "w");
        if (file) {
            fprintf(file, "Test content for %s", test_files[i]);
            fclose(file);
        }
    }
}

int main() {
    create_test_files();
    
    const char* test_cases[] = {"test1.jpg", "test2.png", "test3.gif", "test4.jpeg", "test5.jpg"};
    
    printf("Running 5 test cases:\\n\\n");
    
    for (int i = 0; i < 5; i++) {
        printf("Test case %d: %s\\n", i + 1, test_cases[i]);
        char* result = img(test_cases[i]);
        printf("Result: %s\\n\\n", result);
    }
    
    return 0;
}
