
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>

#define MAX_FILE_SIZE 10485760
#define UPLOAD_DIR "uploads"
#define MAX_FILENAME 256

int is_pdf(const char* data, size_t len) {
    if (len < 4) return 0;
    return data[0] == 0x25 && data[1] == 0x50 && data[2] == 0x44 && data[3] == 0x46;
}

void sanitize_filename(const char* input, char* output, size_t max_len) {
    size_t j = 0;
    for (size_t i = 0; i < strlen(input) && j < max_len - 1; i++) {
        if (isalnum(input[i]) || input[i] == '.' || input[i] == '_' || input[i] == '-') {
            output[j++] = input[i];
        } else {
            output[j++] = '_';
        }
    }
    output[j] = '\\0';
}

int save_file(const char* data, size_t len, const char* filename) {
    if (len > MAX_FILE_SIZE) {
        printf("File too large\\n");
        return 0;
    }
    
    if (!is_pdf(data, len)) {
        printf("Invalid PDF file\\n");
        return 0;
    }
    
    char safe_filename[MAX_FILENAME];
    sanitize_filename(filename, safe_filename, MAX_FILENAME);
    
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/%s", UPLOAD_DIR, safe_filename);
    
    FILE* fp = fopen(filepath, "wb");
    if (!fp) {
        perror("Failed to open file");
        return 0;
    }
    
    size_t written = fwrite(data, 1, len, fp);
    fclose(fp);
    
    if (written != len) {
        printf("Failed to write complete file\\n");
        return 0;
    }
    
    printf("File uploaded successfully: %s\\n", safe_filename);
    return 1;
}

int main() {
    #ifdef _WIN32
    _mkdir(UPLOAD_DIR);
    #else
    mkdir(UPLOAD_DIR, 0755);
    #endif
    
    /* Test cases */
    printf("Test 1: Valid PDF simulation\\n");
    char test_pdf[] = "%PDF-1.4\\nTest content";
    save_file(test_pdf, strlen(test_pdf), "test1.pdf");
    
    printf("\\nTest 2: Invalid file type\\n");
    char test_txt[] = "This is not a PDF";
    save_file(test_txt, strlen(test_txt), "test2.pdf");
    
    printf("\\nTest 3: Filename sanitization\\n");
    char test_pdf3[] = "%PDF-1.4\\nTest";
    save_file(test_pdf3, strlen(test_pdf3), "../../../etc/passwd.pdf");
    
    printf("\\nTest 4: Valid filename\\n");
    save_file(test_pdf, strlen(test_pdf), "valid_file_123.pdf");
    
    printf("\\nTest 5: Special characters in filename\\n");
    save_file(test_pdf, strlen(test_pdf), "file@#$%^&*().pdf");
    
    return 0;
}
