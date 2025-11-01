
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <uuid/uuid.h>
#include <ctype.h>

#define UPLOAD_DIR "uploads/"
#define DB_HOST "localhost"
#define DB_USER "root"
#define DB_PASS "password"
#define DB_NAME "pdfdb"
#define MAX_PATH 1024

int ends_with(const char* str, const char* suffix) {
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    
    if (str_len < suffix_len) return 0;
    return strcmp(str + str_len - suffix_len, suffix) == 0;
}

void to_lower(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

void generate_uuid(char* uuid_str) {
    uuid_t uuid;
    uuid_generate(uuid);
    uuid_unparse(uuid, uuid_str);
}

void create_directory(const char* path) {
    mkdir(path, 0755);
}

int save_filepath_to_database(const char* filepath, const char* original_filename) {
    MYSQL* conn = mysql_init(NULL);
    
    if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0)) {
        mysql_close(conn);
        return 0;
    }
    
    char query[1024];
    snprintf(query, sizeof(query), 
             "INSERT INTO pdf_files (filepath, original_name, upload_date) VALUES ('%s', '%s', NOW())",
             filepath, original_filename);
    
    if (mysql_query(conn, query)) {
        mysql_close(conn);
        return 0;
    }
    
    mysql_close(conn);
    return 1;
}

char* upload_pdf(const char* file_content, size_t content_size, const char* original_filename) {
    char lower_filename[MAX_PATH];
    strncpy(lower_filename, original_filename, MAX_PATH - 1);
    to_lower(lower_filename);
    
    if (!ends_with(lower_filename, ".pdf")) {
        return strdup("Error: Only PDF files are allowed");
    }
    
    create_directory(UPLOAD_DIR);
    
    char uuid_str[37];
    generate_uuid(uuid_str);
    
    char filepath[MAX_PATH];
    snprintf(filepath, MAX_PATH, "%s%s_%s", UPLOAD_DIR, uuid_str, original_filename);
    
    FILE* file = fopen(filepath, "wb");
    if (!file) {
        return strdup("Error: Could not create file");
    }
    
    fwrite(file_content, 1, content_size, file);
    fclose(file);
    
    if (!save_filepath_to_database(filepath, original_filename)) {
        return strdup("Error: Database operation failed");
    }
    
    char* result = malloc(MAX_PATH);
    snprintf(result, MAX_PATH, "Success: %s", filepath);
    return result;
}

char* download_pdf(const char* filepath, size_t* file_size) {
    if (strstr(filepath, "..") != NULL || strncmp(filepath, UPLOAD_DIR, strlen(UPLOAD_DIR)) != 0) {
        *file_size = 0;
        return NULL;
    }
    
    FILE* file = fopen(filepath, "rb");
    if (!file) {
        *file_size = 0;
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    *file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* buffer = malloc(*file_size);
    fread(buffer, 1, *file_size, file);
    fclose(file);
    
    return buffer;
}

int main() {
    // Test case 1: Valid PDF upload
    printf("Test 1: Valid PDF upload\\n");
    const char* content1 = "PDF content test 1";
    char* result1 = upload_pdf(content1, strlen(content1), "test1.pdf");
    printf("%s\\n", result1);
    free(result1);
    
    // Test case 2: Valid PDF upload
    printf("\\nTest 2: Valid PDF upload\\n");
    const char* content2 = "PDF content test 2";
    char* result2 = upload_pdf(content2, strlen(content2), "document.pdf");
    printf("%s\\n", result2);
    free(result2);
    
    // Test case 3: Invalid file extension
    printf("\\nTest 3: Invalid file extension\\n");
    const char* content3 = "Not a PDF";
    char* result3 = upload_pdf(content3, strlen(content3), "malicious.exe");
    printf("%s\\n", result3);
    free(result3);
    
    // Test case 4: Download valid file
    printf("\\nTest 4: Download valid file\\n");
    size_t file_size;
    char* download_result = download_pdf("uploads/test.pdf", &file_size);
    printf("%s\\n", download_result != NULL ? "Success" : "Failed");
    if (download_result) free(download_result);
    
    // Test case 5: Directory traversal attempt
    printf("\\nTest 5: Directory traversal attempt\\n");
    char* traversal_result = download_pdf("../../../etc/passwd", &file_size);
    printf("%s\\n", traversal_result == NULL ? "Blocked (Success)" : "Failed");
    if (traversal_result) free(traversal_result);
    
    return 0;
}
