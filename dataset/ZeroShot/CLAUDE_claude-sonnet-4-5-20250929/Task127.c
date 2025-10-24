
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#define MAX_PATH 512
#define MAX_LINE 1024

const char* get_temp_dir() {
    const char* tmpDir = getenv("TMPDIR");
    if (tmpDir == NULL) {
        tmpDir = getenv("TEMP");
    }
    if (tmpDir == NULL) {
        tmpDir = getenv("TMP");
    }
    if (tmpDir == NULL) {
        tmpDir = "/tmp";
    }
    return tmpDir;
}

void generate_uuid(char* buffer, size_t size) {
    const char* hex = "0123456789abcdef";
    for (size_t i = 0; i < size - 1 && i < 32; i++) {
        buffer[i] = hex[rand() % 16];
    }
    buffer[size - 1] = '\\0';
}

int file_exists(const char* path) {
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

int is_regular_file(const char* path) {
    struct stat buffer;
    if (stat(path, &buffer) != 0) {
        return 0;
    }
    return S_ISREG(buffer.st_mode);
}

char* read_and_write_file(const char* source_file_path) {
    static char temp_file_path[MAX_PATH];
    FILE *source_file = NULL;
    FILE *temp_file = NULL;
    char line[MAX_LINE];
    char unique_id[33];
    
    memset(temp_file_path, 0, sizeof(temp_file_path));
    
    if (source_file_path == NULL || strlen(source_file_path) == 0) {
        fprintf(stderr, "Error: Source file path cannot be NULL or empty\\n");
        return NULL;
    }
    
    /* Validate source file */
    if (!file_exists(source_file_path)) {
        fprintf(stderr, "Error: File not found - %s\\n", source_file_path);
        return NULL;
    }
    
    if (!is_regular_file(source_file_path)) {
        fprintf(stderr, "Error: Path is not a file - %s\\n", source_file_path);
        return NULL;
    }
    
    /* Open source file */
    source_file = fopen(source_file_path, "r");
    if (source_file == NULL) {
        fprintf(stderr, "Error: Cannot open source file - %s\\n", source_file_path);
        return NULL;
    }
    
    /* Create temporary file path */
    generate_uuid(unique_id, sizeof(unique_id));
    snprintf(temp_file_path, sizeof(temp_file_path), "%s/temp_%s.txt", 
             get_temp_dir(), unique_id);
    
    /* Open temporary file */
    temp_file = fopen(temp_file_path, "w");
    if (temp_file == NULL) {
        fprintf(stderr, "Error: Cannot create temporary file\\n");
        fclose(source_file);
        return NULL;
    }
    
    /* Copy content */
    while (fgets(line, sizeof(line), source_file) != NULL) {
        if (fputs(line, temp_file) == EOF) {
            fprintf(stderr, "Error: Write failed\\n");
            fclose(source_file);
            fclose(temp_file);
            return NULL;
        }
    }
    
    /* Close files */
    fclose(source_file);
    fclose(temp_file);
    
    return temp_file_path;
}

int main() {
    char test_file_path[MAX_PATH];
    char *result;
    FILE *test_file;
    
    srand(time(NULL));
    
    printf("=== File Read/Write Error Handling Test Cases ===\\n\\n");
    
    const char* temp_dir = get_temp_dir();
    
    /* Test Case 1: Valid file */
    snprintf(test_file_path, sizeof(test_file_path), "%s/test_input_1.txt", temp_dir);
    test_file = fopen(test_file_path, "w");
    if (test_file != NULL) {
        fprintf(test_file, "This is test content line 1\\n");
        fprintf(test_file, "This is test content line 2\\n");
        fclose(test_file);
        
        printf("Test Case 1: Valid file\\n");
        result = read_and_write_file(test_file_path);
        printf("Result: %s\\n", result ? result : "Failed");
        printf("\\n");
    }
    
    /* Test Case 2: Non-existent file */
    printf("Test Case 2: Non-existent file\\n");
    result = read_and_write_file("/non/existent/file.txt");
    printf("Result: %s\\n", result ? "Success" : "Failed as expected");
    printf("\\n");
    
    /* Test Case 3: NULL file path */
    printf("Test Case 3: NULL file path\\n");
    result = read_and_write_file(NULL);
    printf("Result: %s\\n", result ? "Failed" : "Failed as expected");
    printf("\\n");
    
    /* Test Case 4: Empty file path */
    printf("Test Case 4: Empty file path\\n");
    result = read_and_write_file("");
    printf("Result: %s\\n", result ? "Failed" : "Failed as expected");
    printf("\\n");
    
    /* Test Case 5: Directory instead of file */
    printf("Test Case 5: Directory instead of file\\n");
    result = read_and_write_file(temp_dir);
    printf("Result: %s\\n", result ? "Success" : "Failed as expected");
    printf("\\n");
    
    return 0;
}
