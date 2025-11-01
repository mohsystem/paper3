
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include <openssl/sha.h>
#include <sys/stat.h>
#include <ctype.h>

#define MAX_FILE_SIZE (5 * 1024 * 1024)
#define MAX_FILENAME 256
#define SHA256_STRING_LENGTH 65

typedef struct {
    char* data;
    unsigned long length;
} FileData;

int is_valid_extension(const char* filename) {
    const char* valid_extensions[] = {"jpg", "jpeg", "png", "gif"};
    int num_extensions = 4;
    
    const char* ext = strrchr(filename, '.');
    if (!ext) return 0;
    ext++;
    
    char lower_ext[10];
    int i = 0;
    while (ext[i] && i < 9) {
        lower_ext[i] = tolower(ext[i]);
        i++;
    }
    lower_ext[i] = '\\0';
    
    for (i = 0; i < num_extensions; i++) {
        if (strcmp(lower_ext, valid_extensions[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int is_valid_image_file(const unsigned char* data, size_t size) {
    if (size < 4) return 0;
    
    // JPEG: FF D8 FF
    if (data[0] == 0xFF && data[1] == 0xD8 && data[2] == 0xFF) {
        return 1;
    }
    // PNG: 89 50 4E 47
    if (data[0] == 0x89 && data[1] == 0x50 && data[2] == 0x4E && data[3] == 0x47) {
        return 1;
    }
    // GIF: 47 49 46 38
    if (data[0] == 0x47 && data[1] == 0x49 && data[2] == 0x46 && data[3] == 0x38) {
        return 1;
    }
    return 0;
}

void calculate_sha256(const unsigned char* data, size_t length, char* output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(data, length, hash);
    
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[64] = '\\0';
}

FileData* read_file(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file\\n");
        return NULL;
    }
    
    struct stat st;
    if (stat(filename, &st) != 0) {
        fprintf(stderr, "Error: Cannot get file size\\n");
        fclose(file);
        return NULL;
    }
    
    if (st.st_size > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: File size exceeds maximum allowed size\\n");
        fclose(file);
        return NULL;
    }
    
    FileData* file_data = (FileData*)malloc(sizeof(FileData));
    if (!file_data) {
        fclose(file);
        return NULL;
    }
    
    file_data->length = st.st_size;
    file_data->data = (char*)malloc(file_data->length);
    if (!file_data->data) {
        free(file_data);
        fclose(file);
        return NULL;
    }
    
    size_t read_size = fread(file_data->data, 1, file_data->length, file);
    fclose(file);
    
    if (read_size != file_data->length) {
        free(file_data->data);
        free(file_data);
        return NULL;
    }
    
    return file_data;
}

void create_table_if_not_exists(MYSQL* conn) {
    const char* sql = "CREATE TABLE IF NOT EXISTS images ("
                     "id INT AUTO_INCREMENT PRIMARY KEY, "
                     "file_name VARCHAR(255) NOT NULL, "
                     "file_size BIGINT NOT NULL, "
                     "file_hash VARCHAR(64) NOT NULL, "
                     "image_data LONGBLOB NOT NULL, "
                     "upload_date DATETIME NOT NULL)";
    
    if (mysql_query(conn, sql)) {
        fprintf(stderr, "Error creating table: %s\\n", mysql_error(conn));
    }
}

int store_image(const char* image_path, const char* db_host, const char* db_user, 
                const char* db_password, const char* db_name) {
    MYSQL* conn = NULL;
    MYSQL_STMT* stmt = NULL;
    FileData* file_data = NULL;
    int result = 0;
    
    // Validate file extension
    if (!is_valid_extension(image_path)) {
        fprintf(stderr, "Error: Invalid file extension\\n");
        return 0;
    }
    
    // Read file
    file_data = read_file(image_path);
    if (!file_data) {
        return 0;
    }
    
    // Verify it's an image\n    if (!is_valid_image_file((unsigned char*)file_data->data, file_data->length)) {\n        fprintf(stderr, "Error: File is not a valid image\\n");\n        free(file_data->data);\n        free(file_data);\n        return 0;\n    }\n    \n    // Calculate hash\n    char file_hash[SHA256_STRING_LENGTH];\n    calculate_sha256((unsigned char*)file_data->data, file_data->length, file_hash);\n    \n    // Extract filename\n    const char* filename = strrchr(image_path, '/');\n    if (!filename) filename = strrchr(image_path, '\\\\');
    filename = filename ? filename + 1 : image_path;
    
    // Connect to database
    conn = mysql_init(NULL);
    if (!conn) {
        fprintf(stderr, "Error: mysql_init failed\\n");
        free(file_data->data);
        free(file_data);
        return 0;
    }
    
    if (!mysql_real_connect(conn, db_host, db_user, db_password, db_name, 0, NULL, 0)) {
        fprintf(stderr, "Error: %s\\n", mysql_error(conn));
        mysql_close(conn);
        free(file_data->data);
        free(file_data);
        return 0;
    }
    
    // Create table
    create_table_if_not_exists(conn);
    
    // Prepare statement
    const char* sql = "INSERT INTO images (file_name, file_size, file_hash, image_data, upload_date) "
                     "VALUES (?, ?, ?, ?, NOW())";
    
    stmt = mysql_stmt_init(conn);
    if (!stmt) {
        fprintf(stderr, "Error: mysql_stmt_init failed\\n");
        mysql_close(conn);
        free(file_data->data);
        free(file_data);
        return 0;
    }
    
    if (mysql_stmt_prepare(stmt, sql, strlen(sql))) {
        fprintf(stderr, "Error: %s\\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        free(file_data->data);
        free(file_data);
        return 0;
    }
    
    // Bind parameters
    MYSQL_BIND bind[4];
    memset(bind, 0, sizeof(bind));
    
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char*)filename;
    bind[0].buffer_length = strlen(filename);
    
    long long file_size = file_data->length;
    bind[1].buffer_type = MYSQL_TYPE_LONGLONG;
    bind[1].buffer = &file_size;
    
    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = file_hash;
    bind[2].buffer_length = strlen(file_hash);
    
    bind[3].buffer_type = MYSQL_TYPE_LONG_BLOB;
    bind[3].buffer = file_data->data;
    bind[3].buffer_length = file_data->length;
    bind[3].length = &file_data->length;
    
    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, "Error: %s\\n", mysql_stmt_error(stmt));
    } else if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, "Error: %s\\n", mysql_stmt_error(stmt));
    } else {
        result = 1;
    }
    
    mysql_stmt_close(stmt);
    mysql_close(conn);
    free(file_data->data);
    free(file_data);
    
    return result;
}

int main() {
    // Database configuration - CHANGE THESE VALUES
    const char* db_host = "localhost";
    const char* db_user = "root";
    const char* db_password = "password";
    const char* db_name = "imagedb";
    
    // Test cases
    printf("Test Case 1: Valid JPEG image\\n");
    int result1 = store_image("test1.jpg", db_host, db_user, db_password, db_name);
    printf("Result: %s\\n\\n", result1 ? "true" : "false");
    
    printf("Test Case 2: Valid PNG image\\n");
    int result2 = store_image("test2.png", db_host, db_user, db_password, db_name);
    printf("Result: %s\\n\\n", result2 ? "true" : "false");
    
    printf("Test Case 3: Non-existent file\\n");
    int result3 = store_image("nonexistent.jpg", db_host, db_user, db_password, db_name);
    printf("Result: %s\\n\\n", result3 ? "true" : "false");
    
    printf("Test Case 4: Invalid extension\\n");
    int result4 = store_image("test.txt", db_host, db_user, db_password, db_name);
    printf("Result: %s\\n\\n", result4 ? "true" : "false");
    
    printf("Test Case 5: Valid GIF image\\n");
    int result5 = store_image("test5.gif", db_host, db_user, db_password, db_name);
    printf("Result: %s\\n", result5 ? "true" : "false");
    
    return 0;
}
