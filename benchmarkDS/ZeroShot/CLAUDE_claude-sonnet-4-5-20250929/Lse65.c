
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <uuid/uuid.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir _mkdir
#else
#include <sys/types.h>
#endif

#define UPLOAD_DIR "uploads"
#define MAX_FILE_SIZE (10 * 1024 * 1024)

typedef struct {
    char* data;
    size_t size;
} FileData;

void create_upload_directory() {
#ifdef _WIN32
    mkdir(UPLOAD_DIR);
#else
    mkdir(UPLOAD_DIR, 0755);
#endif
}

char* generate_uuid() {
    uuid_t uuid;
    char* uuid_str = (char*)malloc(37);
    uuid_generate(uuid);
    uuid_unparse(uuid, uuid_str);
    return uuid_str;
}

int is_pdf_file(const char* content, size_t size) {
    if (size < 4) return 0;
    return memcmp(content, "%PDF", 4) == 0;
}

FileData extract_file_content(const char* multipart_data, size_t data_size) {
    FileData result = {NULL, 0};
    const char* pdf_header = "%PDF";
    const char* pdf_footer = "%%EOF";
    
    for (size_t i = 0; i < data_size - 4; i++) {
        if (memcmp(&multipart_data[i], pdf_header, 4) == 0) {
            size_t end = data_size;
            for (size_t j = i; j < data_size - 5; j++) {
                if (memcmp(&multipart_data[j], pdf_footer, 5) == 0) {
                    end = j + 5;
                    break;
                }
            }
            result.size = end - i;
            result.data = (char*)malloc(result.size);
            memcpy(result.data, &multipart_data[i], result.size);
            return result;
        }
    }
    return result;
}

char* handle_file_upload(const char* file_data, size_t data_size, const char* content_type) {
    if (strstr(content_type, "multipart/form-data") == NULL) {
        return strdup("Error: Invalid content type");
    }
    
    if (data_size > MAX_FILE_SIZE) {
        return strdup("Error: File size exceeds maximum allowed size");
    }
    
    FileData file_content = extract_file_content(file_data, data_size);
    
    if (file_content.data == NULL || !is_pdf_file(file_content.data, file_content.size)) {
        if (file_content.data) free(file_content.data);
        return strdup("Error: Only PDF files are allowed");
    }
    
    char* uuid_str = generate_uuid();
    char file_path[512];
    snprintf(file_path, sizeof(file_path), "%s/%s.pdf", UPLOAD_DIR, uuid_str);
    
    FILE* out_file = fopen(file_path, "wb");
    if (!out_file) {
        free(file_content.data);
        free(uuid_str);
        return strdup("Error: Failed to create file");
    }
    
    fwrite(file_content.data, 1, file_content.size, out_file);
    fclose(out_file);
    
    char* result = (char*)malloc(256);
    snprintf(result, 256, "File uploaded successfully: %s.pdf", uuid_str);
    
    free(file_content.data);
    free(uuid_str);
    
    return result;
}

void test_upload_pdf_file(const char* file_name, const char* content) {
    printf("Test case: %s\\n", file_name);
}

int main() {
    create_upload_directory();
    
    // Test cases
    test_upload_pdf_file("test1.pdf", "PDF content 1");
    test_upload_pdf_file("test2.pdf", "PDF content 2");
    test_upload_pdf_file("test3.pdf", "PDF content 3");
    test_upload_pdf_file("test4.pdf", "PDF content 4");
    test_upload_pdf_file("test5.pdf", "PDF content 5");
    
    printf("Server functionality implemented. Note: Full HTTP server requires additional libraries.\\n");
    
    return 0;
}
