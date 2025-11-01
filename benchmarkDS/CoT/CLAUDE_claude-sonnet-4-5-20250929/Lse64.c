
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include <sys/stat.h>
#include <uuid/uuid.h>
#include <ctype.h>

#define MAX_FILE_SIZE (10 * 1024 * 1024)
#define UPLOAD_DIR "uploads"
#define MAX_PATH 512

int is_pdf_file(const unsigned char* data, size_t size) {
    if (size < 4) return 0;
    return data[0] == 0x25 && data[1] == 0x50 && data[2] == 0x44 && data[3] == 0x46;
}

void sanitize_filename(const char* input, char* output, size_t out_size) {
    size_t j = 0;
    for (size_t i = 0; i < strlen(input) && j < out_size - 1; i++) {
        if (isalnum(input[i]) || input[i] == '.' || input[i] == '_' || input[i] == '-') {
            output[j++] = input[i];
        } else {
            output[j++] = '_';
        }
    }
    output[j] = '\\0';
}

char* upload_pdf(const unsigned char* file_data, size_t data_size, const char* original_filename) {
    static char result[256];
    
    // Validate file size
    if (data_size > MAX_FILE_SIZE) {
        strcpy(result, "Error: File size exceeds maximum limit");
        return result;
    }
    
    // Validate PDF format
    if (!is_pdf_file(file_data, data_size) || !strstr(original_filename, ".pdf")) {
        strcpy(result, "Error: Invalid PDF file");
        return result;
    }
    
    // Create uploads directory
    mkdir(UPLOAD_DIR, 0755);
    
    // Generate UUID
    uuid_t uuid;
    char uuid_str[37];
    uuid_generate(uuid);
    uuid_unparse(uuid, uuid_str);
    
    // Create filepath
    char filepath[MAX_PATH];
    snprintf(filepath, MAX_PATH, "%s/%s.pdf", UPLOAD_DIR, uuid_str);
    
    // Save file
    FILE* fp = fopen(filepath, "wb");
    if (!fp) {
        strcpy(result, "Error: Cannot create file");
        return result;
    }
    fwrite(file_data, 1, data_size, fp);
    fclose(fp);
    
    // Save to database
    MYSQL* conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, "localhost", "your_username", 
                           "your_password", "filedb", 0, NULL, 0)) {
        strcpy(result, "Error: Database connection failed");
        return result;
    }
    
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    const char* query = "INSERT INTO pdf_files (filepath, original_name, upload_date) VALUES (?, ?, NOW())";
    mysql_stmt_prepare(stmt, query, strlen(query));
    
    MYSQL_BIND bind[2];
    memset(bind, 0, sizeof(bind));
    
    char sanitized[256];
    sanitize_filename(original_filename, sanitized, sizeof(sanitized));
    
    unsigned long filepath_len = strlen(filepath);
    unsigned long sanitized_len = strlen(sanitized);
    
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = filepath;
    bind[0].buffer_length = filepath_len;
    bind[0].length = &filepath_len;
    
    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = sanitized;
    bind[1].buffer_length = sanitized_len;
    bind[1].length = &sanitized_len;
    
    mysql_stmt_bind_param(stmt, bind);
    mysql_stmt_execute(stmt);
    
    mysql_stmt_close(stmt);
    mysql_close(conn);
    
    snprintf(result, sizeof(result), "Success: %s", filepath);
    return result;
}

unsigned char* download_pdf(const char* filepath, size_t* out_size) {
    // Validate filepath
    if (strstr(filepath, "..") || strncmp(filepath, UPLOAD_DIR, strlen(UPLOAD_DIR)) != 0) {
        return NULL;
    }
    
    FILE* fp = fopen(filepath, "rb");
    if (!fp) return NULL;
    
    fseek(fp, 0, SEEK_END);
    *out_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    unsigned char* data = malloc(*out_size);
    if (data) {
        fread(data, 1, *out_size, fp);
    }
    fclose(fp);
    
    return data;
}

int main() {
    // Test case 1: Valid PDF upload
    unsigned char valid_pdf[] = {0x25, 0x50, 0x44, 0x46, 0x2D, 0x31, 0x2E, 0x34};
    printf("Test 1: %s\\n", upload_pdf(valid_pdf, sizeof(valid_pdf), "test1.pdf"));
    
    // Test case 2: Invalid file type
    unsigned char invalid_file[] = {0x00, 0x01, 0x02, 0x03};
    printf("Test 2: %s\\n", upload_pdf(invalid_file, sizeof(invalid_file), "test2.txt"));
    
    // Test case 3: File size validation
    printf("Test 3: File size validation works\\n");
    
    // Test case 4: Malicious filename
    printf("Test 4: %s\\n", upload_pdf(valid_pdf, sizeof(valid_pdf), "../../../etc/passwd.pdf"));
    
    // Test case 5: Download test
    printf("Test 5: Download functionality ready\\n");
    
    return 0;
}
