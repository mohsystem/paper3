
// Note: C doesn't have decorators or built-in web frameworks\n// This is a simplified implementation showing the core security logic\n\n#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\n#include <sys/stat.h>\n#include <regex.h>\n#include <limits.h>\n\n#define IMAGES_DIR "images"\n#define MAX_FILENAME 256\n#define MAX_PATH 4096\n\nint is_valid_filename(const char* filename) {\n    regex_t regex;\n    int result;\n    \n    // Pattern: ^[a-zA-Z0-9_-]+\\.(jpg|jpeg|png|gif|bmp)$\n    const char* pattern = "^[a-zA-Z0-9_-]+\\\\.(jpg|jpeg|png|gif|bmp)$";\n    \n    if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {\n        return 0;\n    }\n    \n    result = regexec(&regex, filename, 0, NULL, 0);\n    regfree(&regex);\n    \n    return result == 0;\n}\n\nint is_path_safe(const char* filepath) {\n    char resolved[MAX_PATH];\n    char images_dir[MAX_PATH];\n    \n    // Resolve absolute paths\n    if (realpath(filepath, resolved) == NULL) {\n        return 0;\n    }\n    \n    if (realpath(IMAGES_DIR, images_dir) == NULL) {\n        return 0;\n    }\n    \n    // Check if resolved path starts with images directory\n    if (strncmp(resolved, images_dir, strlen(images_dir)) != 0) {\n        return 0;\n    }\n    \n    return 1;\n}\n\nchar* img(const char* name) {\n    static char result[1024];\n    char filepath[MAX_PATH];\n    FILE* file;\n    struct stat st;\n    \n    if (name == NULL || !is_valid_filename(name)) {\n        snprintf(result, sizeof(result), "ERROR: Invalid filename");\n        return result;\n    }\n    \n    snprintf(filepath, sizeof(filepath), "%s/%s", IMAGES_DIR, name);\n    \n    if (!is_path_safe(filepath)) {\n        snprintf(result, sizeof(result), "ERROR: Access denied");\n        return result;\n    }\n    \n    if (stat(filepath, &st) != 0) {\n        snprintf(result, sizeof(result), "ERROR: File not found");\n        return result;\n    }\n    \n    file = fopen(filepath, "rb");\n    if (file == NULL) {\n        snprintf(result, sizeof(result), "ERROR: Cannot open file");\n        return result;\n    }\n    \n    size_t read_size = fread(result, 1, sizeof(result) - 1, file);\n    result[read_size] = '\\0';
    fclose(file);
    
    return result;
}

void setup_test_environment() {
    mkdir(IMAGES_DIR, 0755);
    
    FILE* file1 = fopen("images/test1.jpg", "w");
    if (file1) {
        fprintf(file1, "Fake JPG content");
        fclose(file1);
    }
    
    FILE* file2 = fopen("images/test2.png", "w");
    if (file2) {
        fprintf(file2, "Fake PNG content");
        fclose(file2);
    }
}

int main() {
    setup_test_environment();
    
    printf("Test Cases:\\n");
    printf("1. Valid JPG: %s\\n", img("test1.jpg"));
    printf("2. Valid PNG: %s\\n", img("test2.png"));
    printf("3. Path traversal attempt: %s\\n", img("../etc/passwd"));
    printf("4. Invalid extension: %s\\n", img("test.txt"));
    printf("5. Non-existent file: %s\\n", img("nonexistent.jpg"));
    
    return 0;
}
