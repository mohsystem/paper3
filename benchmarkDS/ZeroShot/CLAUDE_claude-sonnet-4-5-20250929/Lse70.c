
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>
#include <stdbool.h>

#define UPLOAD_DIR "uploads"
#define MAX_FILE_SIZE (5 * 1024 * 1024)  // 5MB
#define MAX_FILENAME_LEN 256
#define MAX_PATH_LEN 512

static const char* ALLOWED_EXTENSIONS[] = {
    "txt", "pdf", "png", "jpg", "jpeg", "gif", "doc", "docx", NULL
};

// Get file extension
static char* get_file_extension(const char* filename) {
    char* dot = strrchr(filename, '.');
    if (dot && dot != filename) {
        return dot + 1;
    }
    return NULL;
}

// Convert string to lowercase
static void to_lowercase(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

// Check if extension is allowed
static bool is_allowed_extension(const char* filename) {
    char* ext = get_file_extension(filename);
    if (!ext) return false;
    
    char ext_lower[64];
    strncpy(ext_lower, ext, sizeof(ext_lower) - 1);
    ext_lower[sizeof(ext_lower) - 1] = '\\0';
    to_lowercase(ext_lower);
    
    for (int i = 0; ALLOWED_EXTENSIONS[i] != NULL; i++) {
        if (strcmp(ext_lower, ALLOWED_EXTENSIONS[i]) == 0) {
            return true;
        }
    }
    return false;
}

// Sanitize filename
static void sanitize_filename(const char* input, char* output, size_t max_len) {
    // Extract basename
    const char* basename = strrchr(input, '/');
    if (!basename) basename = strrchr(input, '\\\\');\n    if (basename) basename++;\n    else basename = input;\n    \n    size_t j = 0;\n    for (size_t i = 0; basename[i] && j < max_len - 1; i++) {\n        if (isalnum(basename[i]) || basename[i] == '.' || \n            basename[i] == '-' || basename[i] == '_') {\n            output[j++] = basename[i];\n        } else {\n            output[j++] = '_';\n        }\n    }\n    output[j] = '\\0';\n}\n\n// Generate unique filename\nstatic void generate_unique_filename(const char* original, char* output, size_t max_len) {\n    time_t now = time(NULL);\n    snprintf(output, max_len, "%ld_%s", (long)now, original);\n}\n\n// Get file size\nstatic long get_file_size(FILE* file) {\n    fseek(file, 0, SEEK_END);\n    long size = ftell(file);\n    fseek(file, 0, SEEK_SET);\n    return size;\n}\n\n// Main upload function\nchar* upload_file(const char* source_path, const char* original_filename, char* result_buffer, size_t buffer_size) {\n    // Create upload directory if it doesn't exist
    struct stat st = {0};
    if (stat(UPLOAD_DIR, &st) == -1) {
        #ifdef _WIN32
        mkdir(UPLOAD_DIR);
        #else
        mkdir(UPLOAD_DIR, 0755);
        #endif
    }
    
    // Validate extension
    if (!is_allowed_extension(original_filename)) {
        snprintf(result_buffer, buffer_size, "Error: File type not allowed");
        return result_buffer;
    }
    
    // Open source file
    FILE* source = fopen(source_path, "rb");
    if (!source) {
        snprintf(result_buffer, buffer_size, "Error: Cannot open source file");
        return result_buffer;
    }
    
    // Check file size
    long file_size = get_file_size(source);
    if (file_size > MAX_FILE_SIZE) {
        fclose(source);
        snprintf(result_buffer, buffer_size, "Error: File size exceeds maximum (5MB)");
        return result_buffer;
    }
    
    // Sanitize and generate unique filename
    char safe_filename[MAX_FILENAME_LEN];
    char unique_filename[MAX_FILENAME_LEN];
    sanitize_filename(original_filename, safe_filename, sizeof(safe_filename));
    generate_unique_filename(safe_filename, unique_filename, sizeof(unique_filename));
    
    // Create destination path
    char dest_path[MAX_PATH_LEN];
    snprintf(dest_path, sizeof(dest_path), "%s/%s", UPLOAD_DIR, unique_filename);
    
    // Open destination file
    FILE* dest = fopen(dest_path, "wb");
    if (!dest) {
        fclose(source);
        snprintf(result_buffer, buffer_size, "Error: Cannot create destination file");
        return result_buffer;
    }
    
    // Copy file
    char buffer[8192];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), source)) > 0) {
        if (fwrite(buffer, 1, bytes, dest) != bytes) {
            fclose(source);
            fclose(dest);
            snprintf(result_buffer, buffer_size, "Error: Write failed");
            return result_buffer;
        }
    }
    
    fclose(source);
    fclose(dest);
    
    snprintf(result_buffer, buffer_size, "Success: File uploaded as %s", unique_filename);
    return result_buffer;
}

int main() {
    printf("File Upload Security Features Implemented:\\n");
    printf("1. File extension whitelist validation\\n");
    printf("2. File size limit (5MB)\\n");
    printf("3. Filename sanitization\\n");
    printf("4. Unique filename generation\\n");
    printf("5. Path traversal prevention\\n\\n");
    
    printf("Test Cases:\\n");
    
    // Test 1: Valid extension
    printf("Test 1 - Valid extension 'test.txt': %s\\n", 
           is_allowed_extension("test.txt") ? "true" : "false");
    
    // Test 2: Invalid extension
    printf("Test 2 - Invalid extension 'test.exe': %s\\n", 
           is_allowed_extension("test.exe") ? "true" : "false");
    
    // Test 3: Path traversal
    char sanitized[MAX_FILENAME_LEN];
    sanitize_filename("../../../etc/passwd", sanitized, sizeof(sanitized));
    printf("Test 3 - Sanitize '../../../etc/passwd': %s\\n", sanitized);
    
    // Test 4: Special characters
    sanitize_filename("test@#$%.txt", sanitized, sizeof(sanitized));
    printf("Test 4 - Sanitize 'test@#$%.txt': %s\\n", sanitized);
    
    // Test 5: Unique filename
    char unique[MAX_FILENAME_LEN];
    generate_unique_filename("test.txt", unique, sizeof(unique));
    printf("Test 5 - Generate unique filename: %s\\n", unique);
    
    return 0;
}
