
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>

#define MAX_FILE_SIZE (5 * 1024 * 1024)
#define UPLOAD_DIR "uploads"
#define MAX_FILENAME 256

int is_valid_pdf_extension(const char* filename) {
    const char* ext = strrchr(filename, '.');
    if (ext == NULL) return 0;
    
    char lower_ext[10];
    int i = 0;
    while (ext[i] && i < 9) {
        lower_ext[i] = tolower(ext[i]);
        i++;
    }
    lower_ext[i] = '\\0';
    
    return strcmp(lower_ext, ".pdf") == 0;
}

int is_valid_pdf_content(const char* filepath) {
    FILE* file = fopen(filepath, "rb");
    if (file == NULL) return 0;
    
    unsigned char header[4];
    size_t read = fread(header, 1, 4, file);
    fclose(file);
    
    if (read != 4) return 0;
    
    return (header[0] == '%' && header[1] == 'P' && 
            header[2] == 'D' && header[3] == 'F');
}

void sanitize_filename(const char* input, char* output, size_t max_len) {
    size_t j = 0;
    for (size_t i = 0; i < strlen(input) && j < max_len - 1; i++) {
        if (isalnum(input[i]) || input[i] == '.' || 
            input[i] == '-' || input[i] == '_') {
            output[j++] = input[i];
        } else {
            output[j++] = '_';
        }
    }
    output[j] = '\\0';
}

void generate_unique_filename(const char* filename, char* output, size_t max_len) {
    char name[MAX_FILENAME];
    char ext[MAX_FILENAME];
    
    const char* dot = strrchr(filename, '.');
    if (dot) {
        strncpy(name, filename, dot - filename);
        name[dot - filename] = '\\0';
        strcpy(ext, dot);
    } else {
        strcpy(name, filename);
        ext[0] = '\\0';
    }
    
    long timestamp = (long)time(NULL);
    snprintf(output, max_len, "%s_%ld%s", name, timestamp, ext);
}

int create_directory(const char* path) {
#ifdef _WIN32
    return mkdir(path);
#else
    return mkdir(path, 0755);
#endif
}

int save_pdf(const char* source_file, const char* filename) {
    struct stat st = {0};
    
    if (stat(UPLOAD_DIR, &st) == -1) {
        if (create_directory(UPLOAD_DIR) != 0) {
            printf("Error creating upload directory\\n");
            return 0;
        }
    }
    
    if (!is_valid_pdf_extension(filename)) {
        printf("Invalid file extension\\n");
        return 0;
    }
    
    FILE* src = fopen(source_file, "rb");
    if (src == NULL) {
        printf("Cannot open source file\\n");
        return 0;
    }
    
    fseek(src, 0, SEEK_END);
    long size = ftell(src);
    fseek(src, 0, SEEK_SET);
    
    if (size > MAX_FILE_SIZE) {
        printf("File too large\\n");
        fclose(src);
        return 0;
    }
    
    char sanitized[MAX_FILENAME];
    sanitize_filename(filename, sanitized, MAX_FILENAME);
    
    char unique[MAX_FILENAME];
    generate_unique_filename(sanitized, unique, MAX_FILENAME);
    
    char dest_path[MAX_FILENAME * 2];
    snprintf(dest_path, sizeof(dest_path), "%s/%s", UPLOAD_DIR, unique);
    
    FILE* dest = fopen(dest_path, "wb");
    if (dest == NULL) {
        printf("Cannot create destination file\\n");
        fclose(src);
        return 0;
    }
    
    char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        fwrite(buffer, 1, bytes, dest);
    }
    
    fclose(src);
    fclose(dest);
    
    if (!is_valid_pdf_content(dest_path)) {
        printf("Invalid PDF content\\n");
        remove(dest_path);
        return 0;
    }
    
    printf("File uploaded successfully: %s\\n", unique);
    return 1;
}

int main() {
    printf("Test Cases for PDF Upload:\\n");
    printf("1. Valid PDF: document.pdf\\n");
    printf("2. Invalid extension: document.txt\\n");
    printf("3. Large file: > 5MB\\n");
    printf("4. Invalid PDF content\\n");
    printf("5. Special characters: doc@#$%%.pdf\\n");
    
    return 0;
}
