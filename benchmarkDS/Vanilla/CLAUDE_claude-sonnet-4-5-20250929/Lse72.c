
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir(dir, mode) _mkdir(dir)
#endif

#define UPLOAD_DIR "uploads"
#define MAX_PATH_LEN 256

void create_upload_directory() {
    struct stat st = {0};
    if (stat(UPLOAD_DIR, &st) == -1) {
        mkdir(UPLOAD_DIR, 0755);
    }
}

int upload_file(const char* data, size_t data_size, const char* filename) {
    char filepath[MAX_PATH_LEN];
    snprintf(filepath, sizeof(filepath), "%s/%s", UPLOAD_DIR, filename);
    
    FILE* fp = fopen(filepath, "wb");
    if (fp == NULL) {
        fprintf(stderr, "Error: Could not open file for writing\\n");
        return 0;
    }
    
    size_t written = fwrite(data, 1, data_size, fp);
    fclose(fp);
    
    if (written == data_size) {
        printf("File uploaded successfully: %s\\n", filename);
        return 1;
    } else {
        fprintf(stderr, "Error: Could not write complete file\\n");
        return 0;
    }
}

char* read_file(const char* filename, size_t* size) {
    char filepath[MAX_PATH_LEN];
    snprintf(filepath, sizeof(filepath), "%s/%s", UPLOAD_DIR, filename);
    
    FILE* fp = fopen(filepath, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Error: Could not open file for reading\\n");
        return NULL;
    }
    
    fseek(fp, 0, SEEK_END);
    *size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    char* buffer = (char*)malloc(*size + 1);
    if (buffer == NULL) {
        fclose(fp);
        return NULL;
    }
    
    fread(buffer, 1, *size, fp);
    buffer[*size] = '\\0';
    fclose(fp);
    
    return buffer;
}

int main() {
    create_upload_directory();
    
    // Test Case 1: Upload directory creation
    printf("Test Case 1: Upload directory created\\n");
    struct stat st = {0};
    printf("Directory exists: %s\\n", (stat(UPLOAD_DIR, &st) == 0) ? "Yes" : "No");
    
    // Test Case 2: Upload a text file
    printf("\\nTest Case 2: Upload text file\\n");
    const char* content1 = "This is test content 1";
    upload_file(content1, strlen(content1), "test1.txt");
    
    // Test Case 3: Upload another file
    printf("\\nTest Case 3: Upload another file\\n");
    const char* content2 = "This is test content 2";
    upload_file(content2, strlen(content2), "test2.txt");
    
    // Test Case 4: Upload binary data
    printf("\\nTest Case 4: Upload binary data\\n");
    const char binary_data[] = {0x48, 0x65, 0x6C, 0x6C, 0x6F};
    upload_file(binary_data, sizeof(binary_data), "binary.dat");
    
    // Test Case 5: Verify file content
    printf("\\nTest Case 5: Verify file content\\n");
    size_t file_size;
    char* file_content = read_file("test1.txt", &file_size);
    if (file_content != NULL) {
        printf("Content of test1.txt: %s\\n", file_content);
        free(file_content);
    }
    
    return 0;
}
