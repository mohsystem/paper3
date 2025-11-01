
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include <sys/stat.h>
#include <ctype.h>

#define MAX_FILE_SIZE (10 * 1024 * 1024)
#define MAX_PATH_LEN 512
#define NUM_EXTENSIONS 5

const char* ALLOWED_EXTENSIONS[] = {".jpg", ".jpeg", ".png", ".gif", ".bmp"};

int validate_extension(const char* filename) {
    char* dot = strrchr(filename, '.');
    if (!dot) return 0;
    
    char ext[10];
    strncpy(ext, dot, sizeof(ext) - 1);
    ext[sizeof(ext) - 1] = '\\0';
    
    for (int i = 0; ext[i]; i++) {
        ext[i] = tolower(ext[i]);
    }
    
    for (int i = 0; i < NUM_EXTENSIONS; i++) {
        if (strcmp(ext, ALLOWED_EXTENSIONS[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int store_image_in_database(const char* image_path, const char* db_host,
                            const char* db_user, const char* db_password,
                            const char* db_name) {
    // Validate input
    if (!image_path || strlen(image_path) == 0) {
        printf("Invalid image path\\n");
        return 0;
    }
    
    // Check file existence and size
    struct stat file_stat;
    if (stat(image_path, &file_stat) != 0) {
        printf("File does not exist or cannot be accessed\\n");
        return 0;
    }
    
    // Validate file size
    if (file_stat.st_size > MAX_FILE_SIZE) {
        printf("File size exceeds maximum allowed size\\n");
        return 0;
    }
    
    // Validate file extension
    if (!validate_extension(image_path)) {
        printf("Invalid file extension\\n");
        return 0;
    }
    
    // Read file
    FILE* file = fopen(image_path, "rb");
    if (!file) {
        printf("Cannot open file\\n");
        return 0;
    }
    
    unsigned char* image_data = malloc(file_stat.st_size);
    if (!image_data) {
        printf("Memory allocation failed\\n");
        fclose(file);
        return 0;
    }
    
    size_t bytes_read = fread(image_data, 1, file_stat.st_size, file);
    fclose(file);
    
    if (bytes_read != file_stat.st_size) {
        printf("File reading error\\n");
        free(image_data);
        return 0;
    }
    
    // Initialize MySQL
    MYSQL* conn = mysql_init(NULL);
    if (!conn) {
        printf("MySQL initialization failed\\n");
        free(image_data);
        return 0;
    }
    
    // Connect to database
    if (!mysql_real_connect(conn, db_host, db_user, db_password, db_name, 0, NULL, 0)) {
        printf("Database connection failed: %s\\n", mysql_error(conn));
        mysql_close(conn);
        free(image_data);
        return 0;
    }
    
    // Prepare statement
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    if (!stmt) {
        printf("Statement initialization failed\\n");
        mysql_close(conn);
        free(image_data);
        return 0;
    }
    
    const char* query = "INSERT INTO images (filename, image_data, upload_date) VALUES (?, ?, NOW())";
    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        printf("Statement preparation failed: %s\\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        free(image_data);
        return 0;
    }
    
    // Extract filename
    const char* filename = strrchr(image_path, '/');
    if (!filename) filename = strrchr(image_path, '\\\\');
    filename = filename ? filename + 1 : image_path;
    
    // Bind parameters
    MYSQL_BIND bind[2];
    memset(bind, 0, sizeof(bind));
    
    unsigned long filename_len = strlen(filename);
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char*)filename;
    bind[0].buffer_length = filename_len;
    bind[0].length = &filename_len;
    
    unsigned long data_len = file_stat.st_size;
    bind[1].buffer_type = MYSQL_TYPE_BLOB;
    bind[1].buffer = image_data;
    bind[1].buffer_length = data_len;
    bind[1].length = &data_len;
    
    if (mysql_stmt_bind_param(stmt, bind)) {
        printf("Parameter binding failed: %s\\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        free(image_data);
        return 0;
    }
    
    // Execute statement
    int success = 0;
    if (mysql_stmt_execute(stmt) == 0) {
        success = 1;
    } else {
        printf("Execution failed: %s\\n", mysql_stmt_error(stmt));
    }
    
    // Clean up
    mysql_stmt_close(stmt);
    mysql_close(conn);
    free(image_data);
    
    return success;
}

int main() {
    const char* db_host = "localhost";
    const char* db_user = "root";
    const char* db_password = "password";
    const char* db_name = "imagedb";
    
    // Test cases
    printf("Test 1 (valid image): %d\\n", 
           store_image_in_database("test1.jpg", db_host, db_user, db_password, db_name));
    
    printf("Test 2 (valid image): %d\\n", 
           store_image_in_database("test2.png", db_host, db_user, db_password, db_name));
    
    printf("Test 3 (NULL path): %d\\n", 
           store_image_in_database(NULL, db_host, db_user, db_password, db_name));
    
    printf("Test 4 (non-existent file): %d\\n", 
           store_image_in_database("nonexistent.jpg", db_host, db_user, db_password, db_name));
    
    printf("Test 5 (invalid extension): %d\\n", 
           store_image_in_database("test.txt", db_host, db_user, db_password, db_name));
    
    return 0;
}
