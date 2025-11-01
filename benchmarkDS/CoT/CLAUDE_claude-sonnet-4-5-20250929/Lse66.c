
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>

#define UPLOAD_DIR "uploads"
#define MAX_FILE_SIZE (10 * 1024 * 1024)
#define MAX_FILENAME_LEN 256

int is_pdf_file(const unsigned char* content, size_t size) {
    if (size < 4) return 0;
    return content[0] == 0x25 && content[1] == 0x50 && 
           content[2] == 0x44 && content[3] == 0x46; /* %PDF */
}

void sanitize_filename(const char* input, char* output, size_t output_size) {
    size_t j = 0;
    for (size_t i = 0; input[i] != '\\0' && j < output_size - 1; i++) {
        if (isalnum(input[i]) || input[i] == '_' || input[i] == '-' || input[i] == '.') {
            output[j++] = input[i];
        } else {
            output[j++] = '_';
        }
    }
    output[j] = '\\0';
    if (j > 100) output[100] = '\\0';
}

void generate_uuid(char* uuid_str, size_t size) {
    srand(time(NULL) + rand());
    snprintf(uuid_str, size, "%08x%08x", rand(), rand());
}

int ends_with(const char* str, const char* suffix) {
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    if (str_len < suffix_len) return 0;
    return strcmp(str + str_len - suffix_len, suffix) == 0;
}

char* upload_pdf_file(const unsigned char* file_content, size_t content_size, 
                      const char* original_filename) {
    static char result[512];
    char safe_filename[MAX_FILENAME_LEN];
    char unique_filename[MAX_FILENAME_LEN * 2];
    char uuid[32];
    char target_path[512];
    FILE* fp;
    
    /* Validate file size */
    if (!file_content || content_size == 0) {
        strcpy(result, "Error: File is empty");
        return result;
    }
    if (content_size > MAX_FILE_SIZE) {
        strcpy(result, "Error: File size exceeds limit");
        return result;
    }
    
    /* Validate filename */
    if (!original_filename || strlen(original_filename) == 0) {
        strcpy(result, "Error: Invalid filename");
        return result;
    }
    
    /* Validate PDF extension */
    if (!ends_with(original_filename, ".pdf") && !ends_with(original_filename, ".PDF")) {
        strcpy(result, "Error: Only PDF files are allowed");
        return result;
    }
    
    /* Validate PDF magic bytes */
    if (!is_pdf_file(file_content, content_size)) {
        strcpy(result, "Error: File is not a valid PDF");
        return result;
    }
    
    /* Create uploads directory */
    #ifdef _WIN32
    _mkdir(UPLOAD_DIR);
    #else
    mkdir(UPLOAD_DIR, 0700);
    #endif
    
    /* Generate safe filename */
    sanitize_filename(original_filename, safe_filename, sizeof(safe_filename));
    generate_uuid(uuid, sizeof(uuid));
    snprintf(unique_filename, sizeof(unique_filename), "%s_%s", uuid, safe_filename);
    
    /* Create target path */
    snprintf(target_path, sizeof(target_path), "%s/%s", UPLOAD_DIR, unique_filename);
    
    /* Write file */
    fp = fopen(target_path, "wb");
    if (!fp) {
        strcpy(result, "Error: Cannot create file");
        return result;
    }
    
    fwrite(file_content, 1, content_size, fp);
    fclose(fp);
    
    /* Set secure permissions */
    #ifndef _WIN32
    chmod(target_path, S_IRUSR | S_IWUSR);
    #endif
    
    snprintf(result, sizeof(result), "Success: File uploaded as %s", unique_filename);
    return result;
}

int main() {
    /* Test case 1: Valid PDF */
    unsigned char valid_pdf[] = {0x25, 0x50, 0x44, 0x46, 0x2D, 0x31, 0x2E, 0x34};
    printf("Test 1: %s\\n", upload_pdf_file(valid_pdf, sizeof(valid_pdf), "test.pdf"));
    
    /* Test case 2: Invalid extension */
    printf("Test 2: %s\\n", upload_pdf_file(valid_pdf, sizeof(valid_pdf), "test.txt"));
    
    /* Test case 3: Empty file */
    printf("Test 3: %s\\n", upload_pdf_file(NULL, 0, "empty.pdf"));
    
    /* Test case 4: Invalid PDF content */
    unsigned char invalid_pdf[] = {0x00, 0x01, 0x02, 0x03};
    printf("Test 4: %s\\n", upload_pdf_file(invalid_pdf, sizeof(invalid_pdf), "fake.pdf"));
    
    /* Test case 5: Malicious filename */
    printf("Test 5: %s\\n", upload_pdf_file(valid_pdf, sizeof(valid_pdf), "../../../etc/passwd.pdf"));
    
    return 0;
}
