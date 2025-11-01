
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <mysql/mysql.h>

#define UPLOAD_DIR "uploads/"
#define DB_HOST "localhost"
#define DB_USER "root"
#define DB_PASS "password"
#define DB_NAME "pdf_db"
#define MAX_PATH 512

int create_directory(const char* path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        return mkdir(path, 0777);
    }
    return 0;
}

int copy_file(const char* source, const char* destination) {
    FILE* src = fopen(source, "rb");
    if (!src) return 0;
    
    FILE* dst = fopen(destination, "wb");
    if (!dst) {
        fclose(src);
        return 0;
    }
    
    char buffer[4096];
    size_t bytes;
    
    while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        fwrite(buffer, 1, bytes, dst);
    }
    
    fclose(src);
    fclose(dst);
    return 1;
}

void save_to_database(const char* filepath, const char* filename) {
    MYSQL* conn = mysql_init(NULL);
    
    if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0)) {
        fprintf(stderr, "Database connection failed: %s\\n", mysql_error(conn));
        return;
    }
    
    char query[1024];
    snprintf(query, sizeof(query), 
             "INSERT INTO pdf_files (filepath, filename, upload_date) VALUES ('%s', '%s', NOW())",
             filepath, filename);
    
    if (mysql_query(conn, query)) {
        fprintf(stderr, "Insert failed: %s\\n", mysql_error(conn));
    }
    
    mysql_close(conn);
}

char* upload_pdf(const char* source_file_path, const char* file_name) {
    static char destination_path[MAX_PATH];
    
    create_directory(UPLOAD_DIR);
    
    char unique_filename[MAX_PATH];
    snprintf(unique_filename, sizeof(unique_filename), "%ld_%s", (long)time(NULL), file_name);
    snprintf(destination_path, sizeof(destination_path), "%s%s", UPLOAD_DIR, unique_filename);
    
    if (!copy_file(source_file_path, destination_path)) {
        return NULL;
    }
    
    save_to_database(destination_path, file_name);
    
    return destination_path;
}

int download_pdf(int file_id, const char* download_path) {
    MYSQL* conn = mysql_init(NULL);
    
    if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0)) {
        fprintf(stderr, "Database connection failed: %s\\n", mysql_error(conn));
        return 0;
    }
    
    char query[256];
    snprintf(query, sizeof(query), "SELECT filepath FROM pdf_files WHERE id = %d", file_id);
    
    if (mysql_query(conn, query)) {
        fprintf(stderr, "Query failed: %s\\n", mysql_error(conn));
        mysql_close(conn);
        return 0;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    MYSQL_ROW row = mysql_fetch_row(result);
    
    if (row) {
        int success = copy_file(row[0], download_path);
        mysql_free_result(result);
        mysql_close(conn);
        return success;
    }
    
    mysql_free_result(result);
    mysql_close(conn);
    return 0;
}

int main() {
    printf("PDF Upload/Download System Test Cases:\\n");
    
    // Test Case 1: Upload a PDF file
    printf("\\nTest Case 1: Upload PDF\\n");
    char* result1 = upload_pdf("test1.pdf", "document1.pdf");
    printf("Upload result: %s\\n", result1 ? result1 : "Failed");
    
    // Test Case 2: Upload another PDF file
    printf("\\nTest Case 2: Upload another PDF\\n");
    char* result2 = upload_pdf("test2.pdf", "document2.pdf");
    printf("Upload result: %s\\n", result2 ? result2 : "Failed");
    
    // Test Case 3: Upload PDF with special characters
    printf("\\nTest Case 3: Upload PDF with special name\\n");
    char* result3 = upload_pdf("test3.pdf", "my_document_2024.pdf");
    printf("Upload result: %s\\n", result3 ? result3 : "Failed");
    
    // Test Case 4: Download PDF by ID
    printf("\\nTest Case 4: Download PDF\\n");
    int download1 = download_pdf(1, "downloaded_file1.pdf");
    printf("Download result: %s\\n", download1 ? "Success" : "Failed");
    
    // Test Case 5: Download non-existent PDF
    printf("\\nTest Case 5: Download non-existent PDF\\n");
    int download2 = download_pdf(9999, "downloaded_file2.pdf");
    printf("Download result: %s\\n", download2 ? "Success" : "Failed (Expected)");
    
    return 0;
}
