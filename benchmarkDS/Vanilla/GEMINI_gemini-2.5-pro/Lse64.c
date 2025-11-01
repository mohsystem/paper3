#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <mysql/mysql.h>

/*
 Pre-requisites:
 1. MySQL server is running.
 2. A database named 'file_db' exists.
 3. A table named 'files' exists in 'file_db'.
    SQL to create table:
    CREATE TABLE files (
        id INT AUTO_INCREMENT PRIMARY KEY,
        filepath VARCHAR(255) NOT NULL,
        uploaded_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
    );
 4. MySQL C API client library (libmysqlclient) is installed and linked.
    - On Debian/Ubuntu: sudo apt-get install libmysqlclient-dev
    - On RHEL/CentOS: sudo yum install mysql-devel

 Compilation command example (Linux/gcc):
 gcc -o Lse64 Lse64.c $(mysql_config --cflags --libs)

 Note: This code uses POSIX-specific functions for directory creation (`mkdir`).
 For Windows, you would need to use `<direct.h>` and `_mkdir()`.
*/

// --- DATABASE CONFIGURATION (Update with your details) ---
#define DB_HOST "localhost"
#define DB_USER "your_username"
#define DB_PASSWORD "your_password"
#define DB_NAME "file_db"
#define UPLOAD_DIR "uploads"

// Helper function to copy a file
int copy_file(const char* src, const char* dest) {
    FILE* source = fopen(src, "rb");
    if (!source) {
        perror("Error opening source file");
        return -1;
    }
    FILE* destination = fopen(dest, "wb");
    if (!destination) {
        perror("Error opening destination file");
        fclose(source);
        return -1;
    }
    char buffer[4096];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), source)) > 0) {
        fwrite(buffer, 1, bytes_read, destination);
    }
    fclose(source);
    fclose(destination);
    return 0;
}


int upload_pdf(const char* source_file_path) {
    struct stat st;
    if (stat(source_file_path, &st) != 0) {
        fprintf(stderr, "Error: Source file does not exist: %s\n", source_file_path);
        return -1;
    }

    // Create the uploads directory if it doesn't exist
    struct stat st_dir = {0};
    if (stat(UPLOAD_DIR, &st_dir) == -1) {
        // mkdir is POSIX-specific; 0700 gives read/write/execute permissions to owner
        if (mkdir(UPLOAD_DIR, 0700) != 0) {
            perror("Error creating upload directory");
            return -1;
        }
    }
    
    // Construct destination path
    const char* file_name = strrchr(source_file_path, '/');
    if (file_name) {
        file_name++; // Move past the '/'
    } else {
        file_name = source_file_path; // No '/' found, path is the filename
    }
    char dest_path[512];
    snprintf(dest_path, sizeof(dest_path), "%s/%s", UPLOAD_DIR, file_name);

    // 1. Save the file to the uploads folder
    if (copy_file(source_file_path, dest_path) != 0) {
        fprintf(stderr, "Error: Could not copy file.\n");
        return -1;
    }
    printf("File saved to: %s\n", dest_path);

    // 2. Save the filepath to the MySQL database
    MYSQL *conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        remove(dest_path); // Rollback
        return -1;
    }

    if (mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASSWORD, DB_NAME, 0, NULL, 0) == NULL) {
        fprintf(stderr, "mysql_real_connect() failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        remove(dest_path); // Rollback
        return -1;
    }
    
    char query[1024];
    snprintf(query, sizeof(query), "INSERT INTO files (filepath) VALUES ('%s')", dest_path);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "INSERT failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        remove(dest_path); // Rollback
        return -1;
    }
    
    my_ulonglong affected_rows = mysql_affected_rows(conn);
    if (affected_rows > 0) {
        printf("Filepath saved to database successfully.\n");
    } else {
        fprintf(stderr, "Error: Failed to save filepath to database.\n");
        mysql_close(conn);
        remove(dest_path); // Rollback
        return -1;
    }

    mysql_close(conn);
    return 0; // Success
}

void create_dummy_file(const char* path) {
    FILE *fp = fopen(path, "w");
    if (fp != NULL) {
        fprintf(fp, "dummy pdf content");
        fclose(fp);
    }
}

int main() {
    // Create a directory for test files
    mkdir("test_files", 0700);
    
    const char* test_files[] = {
        "test_files/report-q1.pdf",
        "test_files/invoice-123.pdf",
        "test_files/manual_v2.pdf",
        "test_files/presentation.pdf",
        "test_files/non_existent_file.pdf"
    };

    for (int i = 0; i < 4; ++i) {
        create_dummy_file(test_files[i]);
    }
    
    printf("--- Running 5 Test Cases ---\n");
    for (int i = 0; i < 5; ++i) {
        printf("\n--- Test Case %d: Uploading %s ---\n", (i + 1), test_files[i]);
        int result = upload_pdf(test_files[i]);
        printf("Result: %s\n", (result == 0) ? "SUCCESS" : "FAILURE");
    }

    return 0;
}