/*
 * Prerequisites for C:
 * 1. MySQL Server running.
 * 2. A database created. For example: `CREATE DATABASE testdb;`
 * 3. A table for storing images. For example:
 *    CREATE TABLE images (
 *        id INT AUTO_INCREMENT PRIMARY KEY,
 *        name VARCHAR(255) NOT NULL,
 *        data LONGBLOB NOT NULL
 *    );
 * 4. MySQL C Connector (libmysqlclient).
 *    On Ubuntu/Debian: sudo apt-get install libmysqlclient-dev
 *    Compilation command:
 *    gcc -o store_image Lse67.c $(mysql_config --cflags --libs)
 *    Or manually:
 *    gcc -o store_image Lse67.c -I/usr/include/mysql -L/usr/lib/mysql -lmysqlclient
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>

// --- IMPORTANT: UPDATE DATABASE CONNECTION DETAILS ---
#define DB_HOST "localhost"
#define DB_USER "your_username"
#define DB_PASS "your_password"
#define DB_NAME "testdb"

/**
 * Stores a file in the MySQL database using prepared statements.
 * @param filePath The path to the file to store.
 * @return 0 on success, -1 on failure.
 */
int storeImageInDB(const char *filePath) {
    MYSQL *conn;
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[2];
    int result = -1; // Default to failure

    conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        return -1;
    }

    if (mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0) == NULL) {
        fprintf(stderr, "mysql_real_connect() failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return -1;
    }

    // Read file into buffer
    FILE *fp = fopen(filePath, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Cannot open file: %s\n", filePath);
        mysql_close(conn);
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *buffer = (char *)malloc(fileSize);
    if (buffer == NULL) {
        fprintf(stderr, "Could not allocate memory for file buffer\n");
        fclose(fp);
        mysql_close(conn);
        return -1;
    }

    if (fread(buffer, fileSize, 1, fp) != 1) {
        fprintf(stderr, "Error reading file into buffer\n");
        free(buffer);
        fclose(fp);
        mysql_close(conn);
        return -1;
    }
    fclose(fp);

    // Prepare statement
    char *query = "INSERT INTO images(name, data) VALUES(?, ?)";
    stmt = mysql_stmt_init(conn);
    if (!stmt) {
        fprintf(stderr, "mysql_stmt_init(), out of memory\n");
        goto cleanup;
    }
    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, "mysql_stmt_prepare(), INSERT failed: %s\n", mysql_stmt_error(stmt));
        goto cleanup;
    }

    memset(bind, 0, sizeof(bind));

    // Bind file name
    const char *fileName = strrchr(filePath, '/');
    if (fileName == NULL) {
        fileName = filePath;
    } else {
        fileName++; // Move past the '/'
    }
    unsigned long name_len = strlen(fileName);
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char *)fileName;
    bind[0].buffer_length = name_len;
    bind[0].length = &name_len;
    
    // Bind file data (blob)
    bind[1].buffer_type = MYSQL_TYPE_LONG_BLOB;
    bind[1].buffer = buffer;
    bind[1].buffer_length = fileSize;
    bind[1].length = (unsigned long *)&fileSize;

    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, "mysql_stmt_bind_param() failed: %s\n", mysql_stmt_error(stmt));
        goto cleanup;
    }

    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, "mysql_stmt_execute(), failed: %s\n", mysql_stmt_error(stmt));
        goto cleanup;
    }
    
    printf("Successfully inserted %lld rows.\n", mysql_stmt_affected_rows(stmt));
    result = 0; // Success

cleanup:
    free(buffer);
    if(stmt) mysql_stmt_close(stmt);
    mysql_close(conn);
    return result;
}

/**
 * Helper function to create a dummy file for testing.
 * @param fileName The name of the file to create.
 */
void createDummyFile(const char* fileName) {
    FILE* fp = fopen(fileName, "w");
    if (fp) {
        fprintf(fp, "This is dummy content for %s", fileName);
        fclose(fp);
    } else {
        fprintf(stderr, "Could not create dummy file: %s\n", fileName);
    }
}

int main() {
    // Test Cases
    const char* testFiles[] = {
        "test_image_1.jpg",
        "test_image_2.png",
        "document.pdf",
        "archive.zip",
        "data.bin"
    };
    int numTests = sizeof(testFiles) / sizeof(testFiles[0]);

    for (int i = 0; i < numTests; ++i) {
        const char* fileName = testFiles[i];
        
        createDummyFile(fileName);

        printf("--- Test Case %d ---\n", i + 1);
        printf("Attempting to store: %s\n", fileName);
        
        int status = storeImageInDB(fileName);
        if (status == 0) {
            printf("Successfully stored '%s' in the database.\n", fileName);
        } else {
            printf("Failed to store '%s' in the database.\n", fileName);
        }

        remove(fileName);
        printf("\n");
    }

    return 0;
}