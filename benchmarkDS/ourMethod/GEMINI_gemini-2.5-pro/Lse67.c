#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <mysql/mysql.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <libgen.h>

// --- Dependencies ---
// Requires MySQL C API (libmysqlclient).
// Link with: -lmysqlclient
// Example compilation: gcc -o store_image store_image.c $(mysql_config --cflags --libs)

// --- Database Schema ---
// CREATE DATABASE IF NOT EXISTS image_db;
// USE image_db;
// CREATE TABLE IF NOT EXISTS images (
//     id INT AUTO_INCREMENT PRIMARY KEY,
//     filename VARCHAR(255) NOT NULL,
//     data LONGBLOB NOT NULL,
//     created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
// );

// --- Database Configuration ---
// It's recommended to load these from a secure source.
#define DB_HOST "localhost"
#define DB_USER "your_user"
#define DB_PASS "your_password"
#define DB_NAME "image_db"

// --- Security Configuration ---
#define BASE_DIRECTORY_PREFIX "/tmp/image_uploads_safe_dir_c"
#define MAX_FILE_SIZE (10 * 1024 * 1024) // 10 MB

/**
 * Stores an image file in the database after performing security checks.
 *
 * @param unsafe_file_path The path to the image file.
 * @return true if successful, false otherwise.
 */
bool store_image_in_db(const char* unsafe_file_path) {
    MYSQL *conn = NULL;
    MYSQL_STMT *stmt = NULL;
    char *file_buffer = NULL;
    char *path_copy = NULL;
    int fd = -1;
    bool result = false;

    if (unsafe_file_path == NULL || unsafe_file_path[0] == '\0') {
        fprintf(stderr, "Error: File path is empty.\n");
        return false;
    }

    // Rule #5: Validate path.
    char resolved_base[PATH_MAX];
    if (realpath(BASE_DIRECTORY_PREFIX, resolved_base) == NULL) {
        perror("Error resolving base directory path");
        return false;
    }

    char resolved_path[PATH_MAX];
    if (realpath(unsafe_file_path, resolved_path) == NULL) {
        perror("Error resolving input file path");
        return false;
    }

    if (strncmp(resolved_base, resolved_path, strlen(resolved_base)) != 0) {
        fprintf(stderr, "Error: Path traversal attempt detected. File is outside the allowed directory.\n");
        return false;
    }
    
    // Rule #6: Open first, then validate handle.
    fd = open(resolved_path, O_RDONLY | O_NOFOLLOW);
    if (fd < 0) {
        perror("Error opening file");
        return false;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("Error getting file status");
        goto cleanup;
    }

    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "Error: Path does not point to a regular file.\n");
        goto cleanup;
    }
    
    // Rule #1 & #3: Validate file size.
    if (st.st_size > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: File size %ld bytes exceeds the limit of %ld bytes.\n", (long)st.st_size, (long)MAX_FILE_SIZE);
        goto cleanup;
    }
    
    if (st.st_size == 0) {
        fprintf(stdout, "Warning: File is empty.\n");
    }

    if (st.st_size > 0) {
        file_buffer = malloc(st.st_size);
        if (file_buffer == NULL) {
            perror("Error allocating memory for file buffer");
            goto cleanup;
        }

        ssize_t bytes_read = read(fd, file_buffer, st.st_size);
        if (bytes_read < 0 || bytes_read != st.st_size) {
            perror("Error reading file");
            goto cleanup;
        }
    }

    close(fd);
    fd = -1; // Mark as closed

    // --- Database Interaction ---
    conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        goto cleanup;
    }

    if (mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0) == NULL) {
        fprintf(stderr, "mysql_real_connect() failed: %s\n", mysql_error(conn));
        goto cleanup;
    }

    stmt = mysql_stmt_init(conn);
    if (!stmt) {
        fprintf(stderr, "mysql_stmt_init() failed\n");
        goto cleanup;
    }
    
    const char *sql = "INSERT INTO images (filename, data) VALUES (?, ?)";
    if (mysql_stmt_prepare(stmt, sql, strlen(sql))) {
        fprintf(stderr, "mysql_stmt_prepare() failed: %s\n", mysql_stmt_error(stmt));
        goto cleanup;
    }

    path_copy = strdup(resolved_path);
    if (path_copy == NULL) {
        perror("strdup failed");
        goto cleanup;
    }
    char *fname = basename(path_copy);

    MYSQL_BIND params[2];
    memset(params, 0, sizeof(params));

    params[0].buffer_type = MYSQL_TYPE_STRING;
    params[0].buffer = (char *)fname;
    params[0].buffer_length = strlen(fname);

    params[1].buffer_type = MYSQL_TYPE_LONG_BLOB;
    params[1].buffer = file_buffer;
    params[1].length = (unsigned long *)&st.st_size;

    if (mysql_stmt_bind_param(stmt, params)) {
        fprintf(stderr, "mysql_stmt_bind_param() failed: %s\n", mysql_stmt_error(stmt));
        goto cleanup;
    }

    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, "mysql_stmt_execute() failed: %s\n", mysql_stmt_error(stmt));
        goto cleanup;
    }
    
    my_ulonglong affected_rows = mysql_stmt_affected_rows(stmt);
    if (affected_rows == 1) {
        printf("Successfully stored image: %s\n", fname);
        result = true;
    } else {
        fprintf(stderr, "Error: Storing image failed, %llu rows affected.\n", affected_rows);
    }

cleanup:
    if (fd != -1) close(fd);
    free(file_buffer);
    free(path_copy);
    if (stmt) mysql_stmt_close(stmt);
    if (conn) mysql_close(conn);
    return result;
}

void setup_test_environment() {
    mkdir(BASE_DIRECTORY_PREFIX, 0755);
    char valid_path[PATH_MAX];
    char large_path[PATH_MAX];
    char outside_path[PATH_MAX];
    
    snprintf(valid_path, sizeof(valid_path), "%s/test_image.jpg", BASE_DIRECTORY_PREFIX);
    snprintf(large_path, sizeof(large_path), "%s/large_image.jpg", BASE_DIRECTORY_PREFIX);
    snprintf(outside_path, sizeof(outside_path), "/tmp/outside_image.jpg");

    FILE *f = fopen(valid_path, "w");
    if(f) { fprintf(f, "dummy data"); fclose(f); }

    int fd_large = open(large_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(fd_large != -1) { ftruncate(fd_large, MAX_FILE_SIZE + 1); close(fd_large); }

    f = fopen(outside_path, "w");
    if(f) { fprintf(f, "outside"); fclose(f); }

    printf("Created base directory and test files.\n");
}

void cleanup_test_environment() {
    char valid_path[PATH_MAX];
    char large_path[PATH_MAX];
    char outside_path[PATH_MAX];
    
    snprintf(valid_path, sizeof(valid_path), "%s/test_image.jpg", BASE_DIRECTORY_PREFIX);
    snprintf(large_path, sizeof(large_path), "%s/large_image.jpg", BASE_DIRECTORY_PREFIX);
    snprintf(outside_path, sizeof(outside_path), "/tmp/outside_image.jpg");

    remove(valid_path);
    remove(large_path);
    remove(outside_path);
    rmdir(BASE_DIRECTORY_PREFIX);

    printf("\nCleaned up test files and directory.\n");
}

int main() {
    setup_test_environment();
    char valid_path[PATH_MAX];
    char large_path[PATH_MAX];
    char outside_path[PATH_MAX];
    char non_existent_path[PATH_MAX];

    snprintf(valid_path, sizeof(valid_path), "%s/test_image.jpg", BASE_DIRECTORY_PREFIX);
    snprintf(large_path, sizeof(large_path), "%s/large_image.jpg", BASE_DIRECTORY_PREFIX);
    snprintf(outside_path, sizeof(outside_path), "/tmp/outside_image.jpg");
    snprintf(non_existent_path, sizeof(non_existent_path), "%s/nonexistent.jpg", BASE_DIRECTORY_PREFIX);

    printf("\n--- Running Test Cases ---\n");
    
    // Test Case 1: Valid file
    printf("\n1. Testing with a valid file...\n");
    store_image_in_db(valid_path);

    // Test Case 2: File is too large
    printf("\n2. Testing with a file that is too large...\n");
    store_image_in_db(large_path);

    // Test Case 3: Path traversal attempt
    printf("\n3. Testing with a file outside the allowed directory...\n");
    store_image_in_db(outside_path);

    // Test Case 4: Non-existent file
    printf("\n4. Testing with a non-existent file...\n");
    store_image_in_db(non_existent_path);
    
    // Test Case 5: Path is a directory
    printf("\n5. Testing with a path that is a directory...\n");
    store_image_in_db(BASE_DIRECTORY_PREFIX);

    cleanup_test_environment();
    
    return 0;
}