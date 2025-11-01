#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "sqlite3.h"

// Note: This code requires linking against the SQLite3 library.
// For example: gcc your_file.c -o your_app -lsqlite3

#ifdef _WIN32
#include <windows.h>
#define O_NOFOLLOW 0 // O_NOFOLLOW is not defined on Windows, but CreateFile has similar flags
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#else
#include <sys/types.h>
#endif


static const long MAX_FILE_SIZE = 5 * 1024 * 1024; // 5 MB

// Base64 encoding function
char* base64_encode(const unsigned char* data, size_t input_length, size_t* output_length) {
    static const char encoding_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    static const int mod_table[] = {0, 2, 1};

    *output_length = 4 * ((input_length + 2) / 3);
    char *encoded_data = (char*)malloc(*output_length + 1);
    if (encoded_data == NULL) return NULL;

    for (size_t i = 0, j = 0; i < input_length;) {
        uint32_t octet_a = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_b = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_c = i < input_length ? (unsigned char)data[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
    }

    for (int i = 0; i < mod_table[input_length % 3]; i++) {
        encoded_data[*output_length - 1 - i] = '=';
    }
    encoded_data[*output_length] = '\0';
    return encoded_data;
}

/**
 * Reads an image, converts it to Base64, and inserts it into the database.
 *
 * @param db The SQLite3 database handle.
 * @param filePath The path to the image file.
 * @return 0 on success, -1 on failure.
 */
int upload_image(sqlite3* db, const char* filePath) {
    int fd = -1;
    unsigned char* buffer = NULL;
    char* encoded_string = NULL;
    sqlite3_stmt* stmt = NULL;
    int rc = -1; // Default to failure

    // Rule #1, #5: Input validation. Reject paths containing slashes.
    if (strchr(filePath, '/') != NULL || strchr(filePath, '\\') != NULL || filePath[0] == '\0') {
        fprintf(stderr, "Error: Invalid file path '%s'. Path traversal detected.\n", filePath);
        return -1;
    }

    // Rule #6: Secure open-then-use pattern to prevent TOCTOU races.
    fd = open(filePath, O_RDONLY | O_NOFOLLOW);
    if (fd < 0) {
        perror("Error opening file");
        goto cleanup;
    }

    struct stat file_stat;
    if (fstat(fd, &file_stat) != 0) {
        perror("Error getting file status");
        goto cleanup;
    }

    if (!S_ISREG(file_stat.st_mode)) {
        fprintf(stderr, "Error: '%s' is not a regular file.\n", filePath);
        goto cleanup;
    }

    if (file_stat.st_size == 0) {
        fprintf(stderr, "Error: File is empty.\n");
        goto cleanup;
    }
    if (file_stat.st_size > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: File size %ld bytes exceeds the limit of %ld bytes.\n", (long)file_stat.st_size, MAX_FILE_SIZE);
        goto cleanup;
    }

    buffer = (unsigned char*)malloc(file_stat.st_size);
    if (!buffer) {
        fprintf(stderr, "Error: Failed to allocate memory for file content.\n");
        goto cleanup;
    }
    
    // Rule #3: Ensure read operation does not go out of bounds.
    ssize_t bytes_read = read(fd, buffer, file_stat.st_size);
    if (bytes_read != file_stat.st_size) {
        fprintf(stderr, "Error: Could not read the entire file.\n");
        goto cleanup;
    }

    size_t encoded_len;
    encoded_string = base64_encode(buffer, file_stat.st_size, &encoded_len);
    if (!encoded_string) {
        fprintf(stderr, "Error: Failed to encode file content.\n");
        goto cleanup;
    }
    
    // Rule #2: Use prepared statements.
    const char* sql = "INSERT INTO images(name, data) VALUES(?, ?)";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        goto cleanup;
    }
    sqlite3_bind_text(stmt, 1, filePath, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, encoded_string, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        goto cleanup;
    }
    
    printf("Successfully uploaded %s\n", filePath);
    rc = 0; // Success

cleanup:
    if (fd >= 0) close(fd);
    if (buffer) free(buffer);
    if (encoded_string) free(encoded_string);
    if (stmt) sqlite3_finalize(stmt);
    return rc;
}

void create_dummy_file(const char* filename, size_t size) {
    FILE* fp = fopen(filename, "wb");
    if (!fp) return;
    if (size > 0) {
        for (size_t i = 0; i < size; ++i) {
            fputc(rand() % 256, fp);
        }
    }
    fclose(fp);
}

void verify_db_content(sqlite3* db, int expectedRows) {
    sqlite3_stmt* stmt;
    const char* sql = "SELECT COUNT(*) FROM images";
    int count = -1;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);
    printf("\nVerification: Found %d rows in the database. Expected %d.\n", count, expectedRows);
    if (count == expectedRows) {
        printf("Verification successful.\n");
    } else {
        printf("Verification failed.\n");
    }
}


void run_test_cases(sqlite3* db) {
    printf("\n--- Running Test Cases ---\n");

    // Test Case 1: Valid small PNG file
    const char* validFile = "test.png";
    create_dummy_file(validFile, 1024);
    printf("\n[Test 1] Uploading a valid small image...\n");
    upload_image(db, validFile);

    // Test Case 2: Valid small JPG file
    const char* validFile2 = "image.jpg";
    create_dummy_file(validFile2, 2048);
    printf("\n[Test 2] Uploading another valid small image...\n");
    upload_image(db, validFile2);

    // Test Case 3: Zero-byte file
    const char* zeroByteFile = "empty.gif";
    create_dummy_file(zeroByteFile, 0);
    printf("\n[Test 3] Uploading a zero-byte file (should fail)...\n");
    upload_image(db, zeroByteFile);

    // Test Case 4: File too large
    const char* largeFile = "large_file.bmp";
    create_dummy_file(largeFile, MAX_FILE_SIZE + 1);
    printf("\n[Test 4] Uploading a file that is too large (should fail)...\n");
    upload_image(db, largeFile);
    
    // Test Case 5: Non-existent file
    printf("\n[Test 5] Uploading a non-existent file (should fail)...\n");
    upload_image(db, "nonexistent.jpg");

    verify_db_content(db, 2);

    // Cleanup
    remove(validFile);
    remove(validFile2);
    remove(zeroByteFile);
    remove(largeFile);
    printf("\n--- Test Cases Finished ---\n");
}

int main() {
    sqlite3* db;
    if (sqlite3_open(":memory:", &db)) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    printf("Connected to in-memory SQLite database.\n");

    char* errMsg = 0;
    const char* sql_create = "CREATE TABLE images("
                             "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
                             "NAME TEXT NOT NULL, "
                             "DATA TEXT NOT NULL);";

    if (sqlite3_exec(db, sql_create, 0, 0, &errMsg) != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return 1;
    }
    printf("Table 'images' created.\n");
    
    run_test_cases(db);

    sqlite3_close(db);
    return 0;
}