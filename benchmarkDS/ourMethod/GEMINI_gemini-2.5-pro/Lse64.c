#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <mysql/mysql.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <libgen.h>

#define UPLOAD_DIRECTORY "uploads"
#define MAX_FILE_SIZE (10 * 1024 * 1024) // 10 MB
#define ALLOWED_EXTENSION ".pdf"
#define FILENAME_BUFFER_SIZE 256
#define PATH_BUFFER_SIZE 512

// In a real application, do not hard-code credentials.
// Read from environment variables or a secure config file.
const char* DB_HOST = getenv("DB_HOST");
const char* DB_USER = getenv("DB_USER");
const char* DB_PASS = getenv("DB_PASS");
const char* DB_NAME = getenv("DB_NAME");

// Forward declaration for cleanup
void cleanup_mysql(MYSQL *conn, MYSQL_STMT *stmt);

/**
 * Uploads a PDF file, saves it securely, and records the filepath in the database.
 * Returns a dynamically allocated string with the new filepath on success, which the caller must free.
 * Returns NULL on failure.
 */
char* upload_pdf(const char* file_data, size_t data_size, const char* original_filename) {
    char* destination_path = NULL;
    FILE *fp = NULL;

    // 1. Validate inputs
    if (file_data == NULL || original_filename == NULL || original_filename[0] == '\0' || data_size == 0) {
        fprintf(stderr, "Error: Invalid input provided.\n");
        return NULL;
    }

    if (data_size > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: File size exceeds maximum limit.\n");
        return NULL;
    }

    // 2. Sanitize and validate filename
    char* original_filename_copy = strdup(original_filename);
    if (!original_filename_copy) {
        fprintf(stderr, "Error: Failed to duplicate filename.\n");
        return NULL;
    }
    char* base_name = basename(original_filename_copy);

    const char* ext = strrchr(base_name, '.');
    if (!ext || strcasecmp(ext, ALLOWED_EXTENSION) != 0) {
        fprintf(stderr, "Error: Invalid file type. Only %s allowed.\n", ALLOWED_EXTENSION);
        free(original_filename_copy);
        return NULL;
    }
    free(original_filename_copy);

    // 3. Generate a secure, unique filename
    char secure_filename[FILENAME_BUFFER_SIZE];
    time_t t = time(NULL);
    snprintf(secure_filename, sizeof(secure_filename), "%ld_%d%s", t, rand() % 1000, ext);
    
    // 4. Create directory and save the file
    struct stat st = {0};
    if (stat(UPLOAD_DIRECTORY, &st) == -1) {
        if (mkdir(UPLOAD_DIRECTORY, 0700) != 0 && errno != EEXIST) {
            perror("Error creating upload directory");
            return NULL;
        }
    }

    destination_path = (char*)malloc(PATH_BUFFER_SIZE);
    if (!destination_path) {
        fprintf(stderr, "Error: Could not allocate memory for path.\n");
        return NULL;
    }
    snprintf(destination_path, PATH_BUFFER_SIZE, "%s/%s", UPLOAD_DIRECTORY, secure_filename);

    fp = fopen(destination_path, "wb");
    if (!fp) {
        perror("Error opening file for writing");
        free(destination_path);
        return NULL;
    }
    if (fwrite(file_data, 1, data_size, fp) != data_size) {
        fprintf(stderr, "Error: Failed to write all data to file.\n");
        fclose(fp);
        remove(destination_path);
        free(destination_path);
        return NULL;
    }
    fclose(fp);
    fp = NULL;

    // 5. Save to database (if configured)
    if (!DB_HOST || !DB_USER || !DB_PASS || !DB_NAME) {
        fprintf(stderr, "Database environment variables not set. Skipping DB insert.\n");
        printf("File saved at (simulation): %s\n", destination_path);
        return destination_path; // Return path even if DB is not configured
    }

    MYSQL *conn = NULL;
    MYSQL_STMT *stmt = NULL;
    conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        goto db_error;
    }

    if (mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0) == NULL) {
        fprintf(stderr, "mysql_real_connect() failed: %s\n", mysql_error(conn));
        goto db_error;
    }

    stmt = mysql_stmt_init(conn);
    if (!stmt) {
        fprintf(stderr, "mysql_stmt_init() failed\n");
        goto db_error;
    }

    const char* sql = "INSERT INTO files (filepath) VALUES (?)";
    if (mysql_stmt_prepare(stmt, sql, strlen(sql))) {
        fprintf(stderr, "mysql_stmt_prepare() failed: %s\n", mysql_stmt_error(stmt));
        goto db_error;
    }

    MYSQL_BIND bind[1];
    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = destination_path;
    bind[0].buffer_length = strlen(destination_path);

    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, "mysql_stmt_bind_param() failed: %s\n", mysql_stmt_error(stmt));
        goto db_error;
    }

    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, "mysql_stmt_execute() failed: %s\n", mysql_stmt_error(stmt));
        goto db_error;
    }

    printf("File uploaded successfully to: %s\n", destination_path);
    cleanup_mysql(conn, stmt);
    return destination_path;

db_error:
    cleanup_mysql(conn, stmt);
    if (destination_path) {
        remove(destination_path); // Rollback file
        free(destination_path);
    }
    return NULL;
}

void cleanup_mysql(MYSQL *conn, MYSQL_STMT *stmt) {
    if (stmt) mysql_stmt_close(stmt);
    if (conn) mysql_close(conn);
}

int main() {
    srand(time(NULL)); // Seed random number generator
    printf("--- Running Test Cases ---\n");
    printf("Note: A running MySQL instance and libmysqlclient are required.\n");
    printf("Compile with: gcc your_file.c -o app `mysql_config --cflags --libs`\n");
    printf("CREATE TABLE files (id INT AUTO_INCREMENT PRIMARY KEY, filepath VARCHAR(255) NOT NULL, created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);\n");
    printf("Set DB_HOST, DB_USER, DB_PASS, DB_NAME environment variables.\n");

    const char* pdf_content = "%PDF-1.4 content...";
    char* returned_path = NULL;

    // Test Case 1: Valid PDF
    printf("\n--- Test Case 1: Valid PDF ---\n");
    returned_path = upload_pdf(pdf_content, strlen(pdf_content), "document.pdf");
    if (returned_path) free(returned_path);

    // Test Case 2: Invalid file type
    printf("\n--- Test Case 2: Invalid file type ---\n");
    returned_path = upload_pdf(pdf_content, strlen(pdf_content), "image.jpg");
    if (returned_path) free(returned_path);

    // Test Case 3: Path traversal attempt
    printf("\n--- Test Case 3: Path traversal attempt ---\n");
    returned_path = upload_pdf(pdf_content, strlen(pdf_content), "../../root/.bashrc.pdf");
    if (returned_path) free(returned_path);

    // Test Case 4: File too large
    printf("\n--- Test Case 4: File too large ---\n");
    returned_path = upload_pdf(pdf_content, MAX_FILE_SIZE + 1, "large.pdf");
    if (returned_path) free(returned_path);

    // Test Case 5: Null input
    printf("\n--- Test Case 5: Null input ---\n");
    returned_path = upload_pdf(NULL, 0, NULL);
    if (returned_path) free(returned_path);

    return 0;
}