
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir(dir, mode) _mkdir(dir)
#endif

#define MAX_FILE_SIZE (10 * 1024 * 1024)
#define UPLOAD_DIR "./uploads/"
#define MAX_FILENAME 256

const char* ALLOWED_EXTENSIONS[] = {"txt", "pdf", "jpg", "jpeg", "png", "gif", "doc", "docx", NULL};

int is_valid_extension(const char* filename) {
    const char* ext = strrchr(filename, '.');
    if (!ext) return 0;
    ext++;
    
    char lower_ext[10];
    int i;
    for (i = 0; ext[i] && i < 9; i++) {
        lower_ext[i] = tolower(ext[i]);
    }
    lower_ext[i] = '\\0';
    
    for (i = 0; ALLOWED_EXTENSIONS[i] != NULL; i++) {
        if (strcmp(lower_ext, ALLOWED_EXTENSIONS[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

void sanitize_filename(char* dest, const char* src) {
    int j = 0;
    for (int i = 0; src[i] && j < MAX_FILENAME - 1; i++) {
        if (isalnum(src[i]) || src[i] == '.' || src[i] == '_' || src[i] == '-') {
            dest[j++] = src[i];
        } else {
            dest[j++] = '_';
        }
    }
    dest[j] = '\\0';
}

void generate_uuid(char* uuid) {
    sprintf(uuid, "%08x-%04x-%04x-%04x-%012x",
            rand(), rand() & 0xFFFF, rand() & 0xFFFF,
            rand() & 0xFFFF, rand());
}

int upload_file(const char* filename, const char* data, size_t size) {
    // Check file size
    if (size > MAX_FILE_SIZE) {
        printf("Error: File too large\\n");
        return 0;
    }
    
    // Validate file type
    if (!is_valid_extension(filename)) {
        printf("Error: Invalid file type\\n");
        return 0;
    }
    
    // Create upload directory
    mkdir(UPLOAD_DIR, 0755);
    
    // Generate safe filename
    char safe_name[MAX_FILENAME];
    sanitize_filename(safe_name, filename);
    
    char uuid[40];
    generate_uuid(uuid);
    
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s%s_%s", UPLOAD_DIR, uuid, safe_name);
    
    // Write file
    FILE* file = fopen(filepath, "wb");
    if (!file) {
        printf("Error: Could not create file\\n");
        return 0;
    }
    
    fwrite(data, 1, size, file);
    fclose(file);
    
    printf("File uploaded successfully: %s_%s\\n", uuid, safe_name);
    return 1;
}

int main() {
    srand(time(NULL));
    
    printf("=== Secure File Upload System ===\\n");
    printf("\\nRunning test cases...\\n\\n");
    
    // Test case 1: Valid text file
    printf("Test 1: Valid .txt file\\n");
    const char* test_data1 = "Hello, this is test data!";
    upload_file("test1.txt", test_data1, strlen(test_data1));
    
    // Test case 2: Valid image file
    printf("\\nTest 2: Valid .jpg file\\n");
    const char* test_data2 = "Fake image data";
    upload_file("photo.jpg", test_data2, strlen(test_data2));
    
    // Test case 3: Invalid file type
    printf("\\nTest 3: Invalid .exe file\\n");
    const char* test_data3 = "Malicious data";
    upload_file("virus.exe", test_data3, strlen(test_data3));
    
    // Test case 4: File with special characters
    printf("\\nTest 4: File with special characters\\n");
    const char* test_data4 = "Test data";
    upload_file("test@#$%.txt", test_data4, strlen(test_data4));
    
    // Test case 5: Valid PDF file
    printf("\\nTest 5: Valid .pdf file\\n");
    const char* test_data5 = "PDF content";
    upload_file("document.pdf", test_data5, strlen(test_data5));
    
    return 0;
}
