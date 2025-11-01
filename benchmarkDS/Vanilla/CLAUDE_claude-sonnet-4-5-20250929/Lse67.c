
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>

int storeImageInDatabase(const char* imagePath) {
    MYSQL* conn;
    MYSQL_STMT* stmt;
    MYSQL_BIND bind[2];
    FILE* file;
    long fileSize;
    char* buffer;
    char* filename;
    
    const char* host = "localhost";
    const char* user = "root";
    const char* password = "password";
    const char* database = "imagedb";
    
    // Initialize MySQL connection
    conn = mysql_init(NULL);
    
    if (!mysql_real_connect(conn, host, user, password, database, 0, NULL, 0)) {
        fprintf(stderr, "Connection failed: %s\\n", mysql_error(conn));
        return 0;
    }
    
    // Open and read file
    file = fopen(imagePath, "rb");
    if (!file) {
        fprintf(stderr, "Cannot open file\\n");
        mysql_close(conn);
        return 0;
    }
    
    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    buffer = (char*)malloc(fileSize);
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed\\n");
        fclose(file);
        mysql_close(conn);
        return 0;
    }
    
    fread(buffer, 1, fileSize, file);
    fclose(file);
    
    // Extract filename
    filename = strrchr(imagePath, '/');
    if (!filename) filename = strrchr(imagePath, '\\\\');
    filename = filename ? filename + 1 : (char*)imagePath;
    
    // Prepare statement
    const char* query = "INSERT INTO images (image_name, image_data) VALUES (?, ?)";
    stmt = mysql_stmt_init(conn);
    
    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, "Prepare failed: %s\\n", mysql_stmt_error(stmt));
        free(buffer);
        mysql_close(conn);
        return 0;
    }
    
    // Bind parameters
    memset(bind, 0, sizeof(bind));
    
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = filename;
    bind[0].buffer_length = strlen(filename);
    
    bind[1].buffer_type = MYSQL_TYPE_LONG_BLOB;
    bind[1].buffer = buffer;
    bind[1].buffer_length = fileSize;
    
    mysql_stmt_bind_param(stmt, bind);
    
    // Execute
    int result = (mysql_stmt_execute(stmt) == 0);
    
    // Cleanup
    free(buffer);
    mysql_stmt_close(stmt);
    mysql_close(conn);
    
    return result;
}

int main() {
    // Test cases - Note: These require actual image files and MySQL setup
    printf("Test Case 1: %d\\n", storeImageInDatabase("test1.jpg"));
    printf("Test Case 2: %d\\n", storeImageInDatabase("test2.png"));
    printf("Test Case 3: %d\\n", storeImageInDatabase("test3.gif"));
    printf("Test Case 4: %d\\n", storeImageInDatabase("test4.bmp"));
    printf("Test Case 5: %d\\n", storeImageInDatabase("test5.jpeg"));
    
    return 0;
}
