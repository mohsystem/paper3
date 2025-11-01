#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>

/*
 * Note: To compile and run this code, you need the MySQL C Connector (libmysqlclient).
 * On Debian/Ubuntu: sudo apt-get install libmysqlclient-dev
 * On RHEL/CentOS: sudo yum install mysql-devel
 *
 * Example compilation command:
 * gcc -o Lse67 Lse67.c $(mysql_config --cflags --libs)
 *
 * SQL to create the necessary table:
 * CREATE DATABASE IF NOT EXISTS image_db;
 * USE image_db;
 * CREATE TABLE IF NOT EXISTS images (
 *     id INT AUTO_INCREMENT PRIMARY KEY,
 *     filename VARCHAR(255) NOT NULL,
 *     image_data LONGBLOB NOT NULL,
 *     upload_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP
 * );
 */

// Forward declaration for the main class-like structure
typedef struct Lse67 Lse67;

struct Lse67 {
    int (*storeImageInDB)(const char* host, const char* user, const char* password, const char* db, const char* filePath);
};

int storeImageInDB_impl(const char* host, const char* user, const char* password, const char* db, const char* filePath) {
    MYSQL *con = NULL;
    MYSQL_STMT *stmt = NULL;
    int result = 0; // 0 for failure, 1 for success

    // 1. Read file into buffer
    FILE *fp = fopen(filePath, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Error: Cannot open file: %s\n", filePath);
        return 0;
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *buffer = (char *)malloc(file_size);
    if (buffer == NULL) {
        fprintf(stderr, "Error: Cannot allocate memory for file buffer\n");
        fclose(fp);
        return 0;
    }

    if (fread(buffer, 1, file_size, fp) != file_size) {
        fprintf(stderr, "Error: Failed to read file into buffer\n");
        free(buffer);
        fclose(fp);
        return 0;
    }
    fclose(fp);

    // 2. Initialize and connect to MySQL
    con = mysql_init(NULL);
    if (con == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        free(buffer);
        return 0;
    }

    if (mysql_real_connect(con, host, user, password, db, 0, NULL, 0) == NULL) {
        fprintf(stderr, "mysql_real_connect() failed: %s\n", mysql_error(con));
        mysql_close(con);
        free(buffer);
        return 0;
    }

    // 3. Prepare statement
    const char *sql = "INSERT INTO images(filename, image_data) VALUES(?, ?)";
    stmt = mysql_stmt_init(con);
    if (!stmt) {
        fprintf(stderr, "mysql_stmt_init() failed\n");
        goto cleanup;
    }

    if (mysql_stmt_prepare(stmt, sql, strlen(sql)) != 0) {
        fprintf(stderr, "mysql_stmt_prepare() failed: %s\n", mysql_stmt_error(stmt));
        goto cleanup;
    }

    // 4. Bind parameters
    MYSQL_BIND bind[2];
    memset(bind, 0, sizeof(bind));

    // Filename
    const char *filename = strrchr(filePath, '/');
    if (filename) {
        filename++; // Move past the '/'
    } else {
        const char *alt_filename = strrchr(filePath, '\\'); // For Windows paths
        filename = alt_filename ? alt_filename + 1 : filePath;
    }
    unsigned long filename_len = strlen(filename);

    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char *)filename;
    bind[0].buffer_length = filename_len;

    // Image data (BLOB)
    bind[1].buffer_type = MYSQL_TYPE_LONG_BLOB;
    bind[1].buffer = buffer;
    bind[1].buffer_length = file_size;

    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        fprintf(stderr, "mysql_stmt_bind_param() failed: %s\n", mysql_stmt_error(stmt));
        goto cleanup;
    }

    // 5. Execute statement
    if (mysql_stmt_execute(stmt) != 0) {
        fprintf(stderr, "mysql_stmt_execute() failed: %s\n", mysql_stmt_error(stmt));
        goto cleanup;
    }

    if (mysql_stmt_affected_rows(stmt) > 0) {
        result = 1; // Success
    }

cleanup:
    if (stmt) mysql_stmt_close(stmt);
    if (con) mysql_close(con);
    free(buffer);
    return result;
}

void createDummyFile(const char* fileName) {
    FILE *fp = fopen(fileName, "w");
    if (fp) {
        fprintf(fp, "This is a dummy file.");
        fclose(fp);
    }
}

int main() {
    Lse67 lse67_obj;
    lse67_obj.storeImageInDB = storeImageInDB_impl;

    // --- Database Configuration ---
    // IMPORTANT: In a real application, do not hardcode credentials.
    // Use environment variables, a configuration file, or a secrets management service.
    const char* HOST = "127.0.0.1";
    const char* USER = "user"; // Replace with your MySQL username
    const char* PASS = "password"; // Replace with your MySQL password
    const char* DB = "image_db";

    // --- Test Cases ---
    printf("Running 5 test cases...\n");
    const char* dummyFileName = "test_image.png";
    createDummyFile(dummyFileName);

    // Test Case 1: Successful Insertion
    printf("\n--- Test Case 1: Successful Insertion ---\n");
    int success1 = lse67_obj.storeImageInDB(HOST, USER, PASS, DB, dummyFileName);
    printf("Result: %s\n", success1 ? "SUCCESS" : "FAILURE");

    // Test Case 2: File Not Found
    printf("\n--- Test Case 2: File Not Found ---\n");
    int success2 = lse67_obj.storeImageInDB(HOST, USER, PASS, DB, "non_existent_file.jpg");
    printf("Result: %s\n", success2 ? "SUCCESS" : "FAILURE");

    // Test Case 3: Invalid Database Password
    printf("\n--- Test Case 3: Invalid Database Password ---\n");
    int success3 = lse67_obj.storeImageInDB(HOST, USER, "wrong_password", DB, dummyFileName);
    printf("Result: %s\n", success3 ? "SUCCESS" : "FAILURE");

    // Test Case 4: Path is a Directory (fopen will fail)
    printf("\n--- Test Case 4: Path is a Directory ---\n");
    int success4 = lse67_obj.storeImageInDB(HOST, USER, PASS, DB, "."); // Current directory
    printf("Result: %s\n", success4 ? "SUCCESS" : "FAILURE");

    // Test Case 5: Another successful insertion
    printf("\n--- Test Case 5: Another Successful Insertion ---\n");
    const char* anotherDummyFile = "another_test.jpg";
    createDummyFile(anotherDummyFile);
    int success5 = lse67_obj.storeImageInDB(HOST, USER, PASS, DB, anotherDummyFile);
    printf("Result: %s\n", success5 ? "SUCCESS" : "FAILURE");

    // Cleanup dummy files
    remove(dummyFileName);
    remove(anotherDummyFile);

    return 0;
}