
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <ctype.h>

#define MAX_FILE_SIZE (10 * 1024 * 1024)
#define MAX_PATH_LEN 512
#define MAX_FILENAME_LEN 256
#define UPLOAD_DIR "uploads"

const char* ALLOWED_EXTENSIONS[] = {"txt", "pdf", "jpg", "jpeg", "png", "gif", NULL};

void sanitize_filename(const char* input, char* output) {
    int j = 0;
    for (int i = 0; input[i] != '\\0' && j < MAX_FILENAME_LEN - 1; i++) {
        char c = input[i];
        if (isalnum(c) || c == '.' || c == '_' || c == '-') {
            output[j++] = c;
        } else {
            output[j++] = '_';
        }
    }
    output[j] = '\\0';
}

void get_file_extension(const char* filename, char* extension) {
    const char* dot = strrchr(filename, '.');
    if (dot && dot != filename) {
        strcpy(extension, dot + 1);
    } else {
        extension[0] = '\\0';
    }
}

int is_extension_allowed(const char* extension) {
    char lower_ext[64];
    int i;
    
    for (i = 0; extension[i] && i < 63; i++) {
        lower_ext[i] = tolower(extension[i]);
    }
    lower_ext[i] = '\\0';
    
    for (i = 0; ALLOWED_EXTENSIONS[i] != NULL; i++) {
        if (strcmp(lower_ext, ALLOWED_EXTENSIONS[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

void generate_unique_filename(const char* original, char* unique) {
    char name_without_ext[MAX_FILENAME_LEN];
    char extension[64];
    
    get_file_extension(original, extension);
    
    const char* dot = strrchr(original, '.');
    if (dot) {
        size_t len = dot - original;
        strncpy(name_without_ext, original, len);
        name_without_ext[len] = '\\0';
    } else {
        strcpy(name_without_ext, original);
    }
    
    snprintf(unique, MAX_FILENAME_LEN, "%s_%ld_%d.%s", 
             name_without_ext, (long)time(NULL), rand() % 10000, extension);
}

char* upload_file(const unsigned char* file_data, size_t data_size, const char* original_filename) {
    static char result[512];
    char sanitized_name[MAX_FILENAME_LEN];
    char extension[64];
    char unique_filename[MAX_FILENAME_LEN];
    char file_path[MAX_PATH_LEN];
    FILE* fp;
    
    // Create uploads directory
    #ifdef _WIN32
    _mkdir(UPLOAD_DIR);
    #else
    mkdir(UPLOAD_DIR, 0755);
    #endif
    
    // Validate file size
    if (data_size > MAX_FILE_SIZE) {
        strcpy(result, "Error: File size exceeds limit");
        return result;
    }
    
    // Sanitize filename
    sanitize_filename(original_filename, sanitized_name);
    if (strlen(sanitized_name) == 0) {
        strcpy(result, "Error: Invalid filename");
        return result;
    }
    
    // Validate extension
    get_file_extension(sanitized_name, extension);
    if (!is_extension_allowed(extension)) {
        strcpy(result, "Error: File type not allowed");
        return result;
    }
    
    // Generate unique filename
    generate_unique_filename(sanitized_name, unique_filename);
    snprintf(file_path, MAX_PATH_LEN, "%s/%s", UPLOAD_DIR, unique_filename);
    
    // Check for path traversal
    if (strstr(file_path, "..") != NULL) {
        strcpy(result, "Error: Invalid file path");
        return result;
    }
    
    // Save file
    fp = fopen(file_path, "wb");
    if (!fp) {
        strcpy(result, "Error: Could not create file");
        return result;
    }
    
    fwrite(file_data, 1, data_size, fp);
    fclose(fp);
    
    snprintf(result, sizeof(result), "Success: File uploaded as %s", unique_filename);
    return result;
}

int main() {
    srand((unsigned)time(NULL));
    
    printf("=== File Upload Test Cases ===\\n\\n");
    
    // Test case 1: Valid text file
    const unsigned char test_file1[] = "Hello, World!";
    printf("Test 1 - Valid text file: %s\\n", 
           upload_file(test_file1, sizeof(test_file1) - 1, "test.txt"));
    
    // Test case 2: Valid image file
    unsigned char test_file2[1024] = {0};
    printf("Test 2 - Valid image file: %s\\n", 
           upload_file(test_file2, sizeof(test_file2), "image.jpg"));
    
    // Test case 3: Invalid extension
    const unsigned char test_file3[] = "script content";
    printf("Test 3 - Invalid extension: %s\\n", 
           upload_file(test_file3, sizeof(test_file3) - 1, "malicious.exe"));
    
    // Test case 4: Path traversal attempt
    const unsigned char test_file4[] = "data";
    printf("Test 4 - Path traversal attempt: %s\\n", 
           upload_file(test_file4, sizeof(test_file4) - 1, "../../../etc/passwd.txt"));
    
    // Test case 5: Empty filename
    const unsigned char test_file5[] = "content";
    printf("Test 5 - Empty filename: %s\\n", 
           upload_file(test_file5, sizeof(test_file5) - 1, ""));
    
    return 0;
}
