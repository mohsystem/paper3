/*
Prerequisites for C:
1. MySQL Server running.
2. A database named 'testdb'.
3. A table created with the following SQL command:
   CREATE TABLE images (
       id INT AUTO_INCREMENT PRIMARY KEY,
       name VARCHAR(255) NOT NULL,
       data LONGBLOB NOT NULL
   );
4. MySQL C Connector (libmysqlclient). You need to install the development
   library and link against it during compilation.
   - On Ubuntu/Debian: sudo apt-get install default-libmysqlclient-dev
   - On RHEL/CentOS: sudo yum install mysql-devel

5. Compilation command (example for gcc):
   gcc your_source_file.c -o your_executable $(mysql_config --cflags --libs)

   Or manually:
   gcc your_source_file.c -o your_executable -I/usr/include/mysql -lmysqlclient
*/

#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Database Configuration ---
// WARNING: Storing credentials in code is insecure. Use environment variables
// or a secrets management system in a production environment.
#define DB_HOST "localhost"
#define DB_USER "user"      // Replace with your DB username
#define DB_PASS "password"  // Replace with your DB password
#define DB_NAME "testdb"

/**
 * @brief Stores an image file in the MySQL database using prepared statements.
 * @param conn A connected MYSQL handle.
 * @param filePath The path to the image file.
 * @return 0 on success, -1 on failure.
 */
int storeImage(MYSQL *conn, const char *filePath) {
    MYSQL_STMT *stmt = NULL;
    MYSQL_BIND bind[2];
    char *buffer = NULL;
    long file_size = 0;
    int result = -1; // Default to failure

    // Open file
    FILE *fp = fopen(filePath, "rb");
    if (!fp) {
        fprintf(stderr, "Error: Cannot open file: %s\n", filePath);
        return -1;
    }

    // Get file size
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    // Allocate memory for the file content
    buffer = (char*)malloc(file_size);
    if (!buffer) {
        fprintf(stderr, "Error: Cannot allocate memory for file content.\n");
        fclose(fp);
        return -1;
    }
    
    // Read file into buffer
    if (fread(buffer, 1, file_size, fp) != file_size) {
        fprintf(stderr, "Error: Failed to read entire file.\n");
        goto cleanup; // Use goto for centralized cleanup
    }

    // Prepare SQL statement
    stmt = mysql_stmt_init(conn);
    if (!stmt) {
        fprintf(stderr, "mysql_stmt_init() failed\n");
        goto cleanup;
    }

    const char *sql = "INSERT INTO images (name, data) VALUES (?, ?)";
    if (mysql_stmt_prepare(stmt, sql, strlen(sql))) {
        fprintf(stderr, "mysql_stmt_prepare() failed: %s\n", mysql_stmt_error(stmt));
        goto cleanup;
    }

    // Bind parameters
    memset(bind, 0, sizeof(bind));

    // Bind file name (parameter 1)
    const char *fileName = strrchr(filePath, '/');
    fileName = fileName ? fileName + 1 : filePath;
    unsigned long name_len = strlen(fileName);
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char *)fileName;
    bind[0].buffer_length = name_len;
    bind[0].length = &name_len;

    // Bind file data (parameter 2)
    bind[1].buffer_type = MYSQL_TYPE_LONG_BLOB;
    bind[1].buffer = buffer;
    bind[1].buffer_length = file_size;
    bind[1].length = (unsigned long *)&file_size; // Length must be a pointer

    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, "mysql_stmt_bind_param() failed: %s\n", mysql_stmt_error(stmt));
        goto cleanup;
    }

    // Execute statement
    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, "mysql_stmt_execute() failed: %s\n", mysql_stmt_error(stmt));
        goto cleanup;
    }

    if (mysql_stmt_affected_rows(stmt) > 0) {
        printf("Successfully inserted %s (%ld bytes).\n", fileName, file_size);
        result = 0; // Success
    } else {
        printf("Insertion failed for %s.\n", fileName);
    }

cleanup:
    if (fp) fclose(fp);
    if (buffer) free(buffer);
    if (stmt) mysql_stmt_close(stmt);
    return result;
}

void createTestFile(const char* path, const char* content, size_t len) {
    FILE *fp = fopen(path, "wb");
    if (fp) {
        fwrite(content, 1, len, fp);
        fclose(fp);
    }
}

// Main function with 5 test cases
int main() {
    MYSQL *conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        return EXIT_FAILURE;
    }

    if (mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0) == NULL) {
        fprintf(stderr, "mysql_real_connect() failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return EXIT_FAILURE;
    }
    
    // --- Test Setup ---
    const char* dummyImagePath = "test_image.png";
    const char* emptyImagePath = "empty_image.png";
    const char* anotherImagePath = "another_image.jpg";
    const char* longNamePath = "a_very_long_file_name_to_test_database_field_limits_and_handling_of_such_cases_0123456789.bin";

    createTestFile(dummyImagePath, "\x89PNG\x01\x02\x03\x04", 8);
    createTestFile(emptyImagePath, "", 0);
    createTestFile(anotherImagePath, "\x89JPG\x01\x02\x03\x04", 8);
    createTestFile(longNamePath, "long name test", 14);

    printf("Running C Image Storage Tests...\n");

    // Test Case 1: Store a valid image file.
    printf("\n--- Test Case 1: Store a valid image ---\n");
    int res1 = storeImage(conn, dummyImagePath);
    printf("Result: %s\n", res1 == 0 ? "SUCCESS" : "FAILURE");

    // Test Case 2: Store another valid image.
    printf("\n--- Test Case 2: Store another valid image ---\n");
    int res2 = storeImage(conn, anotherImagePath);
    printf("Result: %s\n", res2 == 0 ? "SUCCESS" : "FAILURE");

    // Test Case 3: Try to store a non-existent file.
    printf("\n--- Test Case 3: Store a non-existent file ---\n");
    int res3 = storeImage(conn, "non_existent_file.jpg");
    printf("Result: %s\n", res3 == 0 ? "SUCCESS" : "FAILURE");

    // Test Case 4: Store a file with a long name.
    printf("\n--- Test Case 4: Store a file with a long name ---\n");
    int res4 = storeImage(conn, longNamePath);
    printf("Result: %s\n", res4 == 0 ? "SUCCESS" : "FAILURE");
    
    // Test Case 5: Store an empty file.
    printf("\n--- Test Case 5: Store an empty file ---\n");
    int res5 = storeImage(conn, emptyImagePath);
    printf("Result: %s\n", res5 == 0 ? "SUCCESS" : "FAILURE");

    // --- Cleanup ---
    printf("\nCleaning up test files...\n");
    remove(dummyImagePath);
    remove(emptyImagePath);
    remove(anotherImagePath);
    remove(longNamePath);
    printf("Done.\n");

    mysql_close(conn);
    return EXIT_SUCCESS;
}