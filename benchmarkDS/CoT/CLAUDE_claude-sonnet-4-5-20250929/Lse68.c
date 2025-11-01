
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sqlite3.h>

#define MAX_FILE_SIZE (5 * 1024 * 1024)
#define MAX_FILENAME 256
#define MAX_BASE64_SIZE (MAX_FILE_SIZE * 2)

static const char base64_chars[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

char* base64_encode(const unsigned char* data, size_t input_length) {
    size_t output_length = 4 * ((input_length + 2) / 3);
    char* encoded = (char*)malloc(output_length + 1);
    if (!encoded) return NULL;
    
    size_t i, j;
    for (i = 0, j = 0; i < input_length;) {
        uint32_t octet_a = i < input_length ? data[i++] : 0;
        uint32_t octet_b = i < input_length ? data[i++] : 0;
        uint32_t octet_c = i < input_length ? data[i++] : 0;
        uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;
        
        encoded[j++] = base64_chars[(triple >> 18) & 0x3F];
        encoded[j++] = base64_chars[(triple >> 12) & 0x3F];
        encoded[j++] = base64_chars[(triple >> 6) & 0x3F];
        encoded[j++] = base64_chars[triple & 0x3F];
    }
    
    int mod = input_length % 3;
    if (mod == 1) {
        encoded[output_length - 2] = '=';
    } else if (mod == 2) {
        encoded[output_length - 1] = '=';
    }
    
    encoded[output_length] = '\\0';
    return encoded;
}

void get_file_extension(const char* filename, char* ext, size_t ext_size) {
    const char* dot = strrchr(filename, '.');
    if (dot && dot != filename) {
        strncpy(ext, dot + 1, ext_size - 1);
        ext[ext_size - 1] = '\\0';
        for (size_t i = 0; ext[i]; i++) {
            ext[i] = tolower(ext[i]);
        }
    } else {
        ext[0] = '\\0';
    }
}

void sanitize_filename(const char* input, char* output, size_t output_size) {
    size_t j = 0;
    for (size_t i = 0; input[i] && j < output_size - 1; i++) {
        if (isalnum(input[i]) || input[i] == '.' || input[i] == '_' || input[i] == '-') {
            output[j++] = input[i];
        } else {
            output[j++] = '_';
        }
    }
    output[j] = '\\0';
}

int is_valid_extension(const char* ext) {
    const char* valid[] = {"jpg", "jpeg", "png", "gif"};
    for (size_t i = 0; i < sizeof(valid) / sizeof(valid[0]); i++) {
        if (strcmp(ext, valid[i]) == 0) return 1;
    }
    return 0;
}

char* upload_image(const char* file_path, sqlite3* db) {
    struct stat st;
    char extension[16];
    char sanitized_name[MAX_FILENAME];
    char* result = NULL;
    
    // Validate file exists
    if (stat(file_path, &st) != 0) {
        return strdup("Error: Invalid file path");
    }
    
    // Validate file size
    if (st.st_size > MAX_FILE_SIZE) {
        return strdup("Error: File size exceeds maximum allowed size");
    }
    
    // Extract filename
    const char* filename = strrchr(file_path, '/');
    if (!filename) filename = strrchr(file_path, '\\\\');
    filename = filename ? filename + 1 : file_path;
    
    // Validate extension
    get_file_extension(filename, extension, sizeof(extension));
    if (!is_valid_extension(extension)) {
        return strdup("Error: Invalid file type");
    }
    
    // Sanitize filename
    sanitize_filename(filename, sanitized_name, sizeof(sanitized_name));
    
    // Read file
    FILE* file = fopen(file_path, "rb");
    if (!file) {
        return strdup("Error: Cannot open file");
    }
    
    unsigned char* file_content = (unsigned char*)malloc(st.st_size);
    if (!file_content) {
        fclose(file);
        return strdup("Error: Memory allocation failed");
    }
    
    fread(file_content, 1, st.st_size, file);
    fclose(file);
    
    // Encode to base64
    char* base64_string = base64_encode(file_content, st.st_size);
    free(file_content);
    
    if (!base64_string) {
        return strdup("Error: Base64 encoding failed");
    }
    
    // Insert into database
    const char* sql = "INSERT INTO images (name, data) VALUES (?, ?)";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        free(base64_string);
        return strdup("Error: Failed to prepare statement");
    }
    
    sqlite3_bind_text(stmt, 1, sanitized_name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, base64_string, -1, SQLITE_TRANSIENT);
    
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        free(base64_string);
        return strdup("Error: Failed to insert data");
    }
    
    sqlite3_finalize(stmt);
    free(base64_string);
    
    result = (char*)malloc(256);
    snprintf(result, 256, "Image uploaded successfully: %s", sanitized_name);
    return result;
}

void create_test_file(const char* filename, size_t size) {
    FILE* f = fopen(filename, "wb");
    if (f) {
        for (size_t i = 0; i < size; i++) {
            fputc(0xFF, f);
        }
        fclose(f);
    }
}

int main() {
    sqlite3* db;
    
    if (sqlite3_open("test.db", &db) != SQLITE_OK) {
        fprintf(stderr, "Cannot open database\\n");
        return 1;
    }
    
    const char* create_table = "CREATE TABLE IF NOT EXISTS images "
                               "(id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, data TEXT)";
    sqlite3_exec(db, create_table, NULL, NULL, NULL);
    
    // Test Case 1
    printf("Test Case 1: Valid image\\n");
    create_test_file("test1.jpg", 100);
    char* result = upload_image("test1.jpg", db);
    printf("%s\\n", result);
    free(result);
    
    // Test Case 2
    printf("\\nTest Case 2: Valid PNG\\n");
    create_test_file("test2.png", 200);
    result = upload_image("test2.png", db);
    printf("%s\\n", result);
    free(result);
    
    // Test Case 3
    printf("\\nTest Case 3: Invalid extension\\n");
    create_test_file("test3.exe", 100);
    result = upload_image("test3.exe", db);
    printf("%s\\n", result);
    free(result);
    
    // Test Case 4
    printf("\\nTest Case 4: Non-existent file\\n");
    result = upload_image("nonexistent.jpg", db);
    printf("%s\\n", result);
    free(result);
    
    // Test Case 5
    printf("\\nTest Case 5: File with special characters\\n");
    create_test_file("test@#$5.jpg", 100);
    result = upload_image("test@#$5.jpg", db);
    printf("%s\\n", result);
    free(result);
    
    sqlite3_close(db);
    return 0;
}
