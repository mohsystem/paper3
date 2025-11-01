
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <mysql/mysql.h>

/* Maximum file size: 5MB to prevent DoS */
#define MAX_FILE_SIZE (5 * 1024 * 1024)

/* Maximum filename length */
#define MAX_FILENAME_LENGTH 255

/**
 * Converts character to lowercase
 */
static char to_lower(char c) {
    return (c >= 'A' && c <= 'Z') ? (c + 32) : c;
}

/**
 * Checks if file extension is allowed (case-insensitive)
 */
static int is_valid_extension(const char* filename) {
    if (filename == NULL) return 0;
    
    const char* dot = strrchr(filename, '.');
    if (dot == NULL) return 0;
    
    /* Convert extension to lowercase for comparison */
    char ext_lower[16] = {0};
    size_t i;
    for (i = 0; i < sizeof(ext_lower) - 1 && dot[i] != '\\0'; i++) {
        ext_lower[i] = to_lower(dot[i]);
    }
    ext_lower[i] = '\\0';
    
    /* Whitelist of allowed extensions */
    const char* allowed[] = {".jpg", ".jpeg", ".png", ".gif", ".bmp", ".webp", NULL};
    for (i = 0; allowed[i] != NULL; i++) {
        if (strcmp(ext_lower, allowed[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

/**
 * Extracts filename from path
 */
static const char* get_filename(const char* path) {
    const char* last_slash = strrchr(path, '/');
    if (last_slash == NULL) {
        last_slash = strrchr(path, '\\\\');\n    }\n    return (last_slash != NULL) ? (last_slash + 1) : path;\n}\n\n/**\n * Securely stores an image file in MySQL database\n */\nint store_image_in_database(const char* file_path, const char* db_host,\n                            const char* db_user, const char* db_password,\n                            const char* db_name) {\n    /* Input validation: reject NULL or empty paths */\n    if (file_path == NULL || file_path[0] == '\\0') {\n        fprintf(stderr, "Error: File path cannot be NULL or empty\\n");\n        return 0;\n    }\n    \n    MYSQL* conn = NULL;\n    MYSQL_STMT* stmt = NULL;\n    int fd = -1;\n    unsigned char* image_data = NULL;\n    int success = 0;\n    \n    /* Secure file handling: open first with O_RDONLY and O_NOFOLLOW to prevent symlink attacks */\n    fd = open(file_path, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);\n    if (fd < 0) {\n        fprintf(stderr, "Error: Failed to open file\\n");\n        return 0;\n    }\n    \n    /* Validation: fstat the opened descriptor (TOCTOU prevention) */\n    struct stat st;\n    if (fstat(fd, &st) != 0) {\n        fprintf(stderr, "Error: Failed to stat file\\n");\n        close(fd);\n        return 0;\n    }\n    \n    /* Validation: ensure it's a regular file */
    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "Error: Not a regular file\\n");
        close(fd);
        return 0;
    }
    
    /* Validation: check file size to prevent DoS */
    if (st.st_size > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: File size exceeds maximum\\n");
        close(fd);
        return 0;
    }
    
    if (st.st_size == 0) {
        fprintf(stderr, "Error: File is empty\\n");
        close(fd);
        return 0;
    }
    
    size_t file_size = (size_t)st.st_size;
    
    /* Extract and validate filename */
    const char* filename = get_filename(file_path);
    if (strlen(filename) > MAX_FILENAME_LENGTH) {
        fprintf(stderr, "Error: Filename too long\\n");
        close(fd);
        return 0;
    }
    
    /* Validation: check file extension */
    if (!is_valid_extension(filename)) {
        fprintf(stderr, "Error: Invalid or unsupported file type\\n");
        close(fd);
        return 0;
    }
    
    /* Allocate buffer for image data - check for overflow */
    image_data = (unsigned char*)malloc(file_size);
    if (image_data == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        close(fd);
        return 0;
    }
    
    /* Initialize allocated memory */
    memset(image_data, 0, file_size);
    
    /* Read file content using the opened fd */
    size_t total_read = 0;
    while (total_read < file_size) {
        ssize_t bytes_read = read(fd, image_data + total_read, file_size - total_read);
        if (bytes_read < 0) {
            fprintf(stderr, "Error: Failed to read file\\n");
            free(image_data);
            close(fd);
            return 0;
        }
        if (bytes_read == 0) {
            break;
        }
        total_read += (size_t)bytes_read;
    }
    
    close(fd);
    fd = -1;
    
    if (total_read != file_size) {
        fprintf(stderr, "Error: Failed to read complete file\\n");
        free(image_data);
        return 0;
    }
    
    /* Initialize MySQL connection */
    conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "Error: MySQL init failed\\n");
        free(image_data);
        return 0;
    }
    
    /* Connect to database */
    if (mysql_real_connect(conn, db_host, db_user, db_password, db_name, 0, NULL, 0) == NULL) {
        fprintf(stderr, "Error: Database connection failed\\n");
        mysql_close(conn);
        free(image_data);
        return 0;
    }
    
    /* Prepare parameterized statement to prevent SQL injection */
    const char* query = "INSERT INTO images (filename, file_size, image_data) VALUES (?, ?, ?)";
    stmt = mysql_stmt_init(conn);
    if (stmt == NULL) {
        fprintf(stderr, "Error: Statement init failed\\n");
        goto cleanup;
    }
    
    if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0) {
        fprintf(stderr, "Error: Statement prepare failed\\n");
        goto cleanup;
    }
    
    /* Bind parameters */
    MYSQL_BIND bind[3];
    memset(bind, 0, sizeof(bind));
    
    /* Filename parameter */
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char*)filename;
    bind[0].buffer_length = strlen(filename);
    
    /* File size parameter */
    unsigned long long size_param = file_size;
    bind[1].buffer_type = MYSQL_TYPE_LONGLONG;
    bind[1].buffer = &size_param;
    bind[1].is_unsigned = 1;
    
    /* Image data parameter */
    unsigned long data_length = file_size;
    bind[2].buffer_type = MYSQL_TYPE_LONG_BLOB;
    bind[2].buffer = image_data;
    bind[2].buffer_length = file_size;
    bind[2].length = &data_length;
    
    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        fprintf(stderr, "Error: Parameter binding failed\\n");
        goto cleanup;
    }
    
    /* Execute statement */
    if (mysql_stmt_execute(stmt) == 0) {
        success = 1;
    }
    
cleanup:
    /* Cleanup resources in reverse order of acquisition */
    if (stmt != NULL) {
        mysql_stmt_close(stmt);
    }
    if (conn != NULL) {
        mysql_close(conn);
    }
    if (image_data != NULL) {
        free(image_data);
    }
    
    return success;
}

int main(void) {
    /* Test configuration */
    const char* db_host = "localhost";
    const char* db_user = "testuser";
    const char* db_password = "testpass";
    const char* db_name = "imagedb";
    
    /* Test case 1: Valid JPEG */
    printf("Test 1 - Valid JPEG:\\n");
    int result1 = store_image_in_database("test1.jpg", db_host, db_user, db_password, db_name);
    printf("Result: %s\\n\\n", result1 ? "true" : "false");
    
    /* Test case 2: Valid PNG */
    printf("Test 2 - Valid PNG:\\n");
    int result2 = store_image_in_database("test2.png", db_host, db_user, db_password, db_name);
    printf("Result: %s\\n\\n", result2 ? "true" : "false");
    
    /* Test case 3: NULL path */
    printf("Test 3 - NULL path:\\n");
    int result3 = store_image_in_database(NULL, db_host, db_user, db_password, db_name);
    printf("Result: %s\\n\\n", result3 ? "true" : "false");
    
    /* Test case 4: Empty path */
    printf("Test 4 - Empty path:\\n");
    int result4 = store_image_in_database("", db_host, db_user, db_password, db_name);
    printf("Result: %s\\n\\n", result4 ? "true" : "false");
    
    /* Test case 5: Non-existent file */
    printf("Test 5 - Non-existent file:\\n");
    int result5 = store_image_in_database("nonexistent.jpg", db_host, db_user, db_password, db_name);
    printf("Result: %s\\n", result5 ? "true" : "false");
    
    return 0;
}
