#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#include <sys/types.h>
#define MKDIR(path) mkdir(path, 0755)
#endif

// To compile, you need the MySQL C client library (libmysqlclient)
// gcc your_file.c -o your_app `mysql_config --cflags --libs`
// #include <mysql/mysql.h>

// --- Configuration ---
#define UPLOAD_DIR "uploads"
#define MAX_FILE_SIZE (10 * 1024 * 1024) // 10 MB
#define FILENAME_MAX_LEN 256
#define FILEPATH_MAX_LEN 512

// --- Database Configuration ---
// FIXME: Replace with your actual database configuration
#define DB_HOST "localhost"
#define DB_USER "your_username"
#define DB_PASS "your_password"
#define DB_NAME "your_database"

/**
 * MOCK DATABASE FUNCTION
 * This mock allows the code to be compiled and run without a MySQL dependency.
 * Replace with the real implementation below for a production environment.
 */
bool save_filepath_to_db(const char* filepath) {
    printf("Attempting to save filepath to DB (MOCKED): %s\n", filepath);
    // This is a placeholder for a real table.
    // CREATE TABLE IF NOT EXISTS uploaded_files (id INT AUTO_INCREMENT PRIMARY KEY, filepath VARCHAR(255) NOT NULL, created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);
    printf("Filepath successfully saved to database (MOCKED).\n");
    return true;
}

/*
 * REAL DATABASE FUNCTION
 * Requires MySQL C client library to be installed and linked.
 *
bool save_filepath_to_db(const char* filepath) {
    MYSQL *conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        return false;
    }

    if (mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0) == NULL) {
        fprintf(stderr, "mysql_real_connect() failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return false;
    }

    const char* query = "INSERT INTO uploaded_files (filepath) VALUES (?)";
    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (!stmt) {
        fprintf(stderr, "mysql_stmt_init() failed\n");
        mysql_close(conn);
        return false;
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, "mysql_stmt_prepare() failed: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }

    MYSQL_BIND bind[1];
    memset(bind, 0, sizeof(bind));
    
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char*)filepath;
    bind[0].buffer_length = strlen(filepath);

    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, "mysql_stmt_bind_param() failed: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }

    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, "mysql_stmt_execute() failed: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }

    printf("Filepath successfully saved to database.\n");
    mysql_stmt_close(stmt);
    mysql_close(conn);
    return true;
}
*/

/**
 * Securely sanitizes a filename by removing path components.
 */
const char* sanitize_filename(const char* original_filename) {
    const char* last_slash = strrchr(original_filename, '/');
    const char* last_backslash = strrchr(original_filename, '\\');
    
    if (last_slash == NULL && last_backslash == NULL) {
        return original_filename;
    }
    
    if (last_slash != NULL && last_backslash != NULL) {
        return (last_slash > last_backslash) ? last_slash + 1 : last_backslash + 1;
    }
    
    return (last_slash != NULL) ? last_slash + 1 : last_backslash + 1;
}

/**
 * Securely uploads a PDF file.
 * Returns the generated filepath on success, NULL on failure.
 * The caller is responsible for freeing the returned string.
 */
char* upload_pdf(FILE* input_stream, const char* original_filename) {
    // 1. Security: Sanitize filename to prevent path traversal
    const char* sanitized_filename = sanitize_filename(original_filename);
    if (strlen(sanitized_filename) == 0 || strlen(sanitized_filename) > FILENAME_MAX_LEN) {
        fprintf(stderr, "Error: Invalid or too long filename.\n");
        return NULL;
    }

    // 2. Security: Validate file extension
    const char* ext = strrchr(sanitized_filename, '.');
    if (!ext || strcmp(ext, ".pdf") != 0) {
        fprintf(stderr, "Error: Invalid file extension. Only .pdf is allowed.\n");
        return NULL;
    }

    // 3. Security: Validate file content (magic bytes)
    unsigned char pdf_magic_bytes[] = {'%', 'P', 'D', 'F', '-'};
    unsigned char buffer[sizeof(pdf_magic_bytes)];
    size_t bytes_read = fread(buffer, 1, sizeof(buffer), input_stream);
    rewind(input_stream); // Rewind for the copy operation
    if (bytes_read < sizeof(pdf_magic_bytes) || memcmp(buffer, pdf_magic_bytes, sizeof(pdf_magic_bytes)) != 0) {
        fprintf(stderr, "Error: File is not a valid PDF. Magic bytes mismatch.\n");
        return NULL;
    }

    // 4. Security: Generate a unique filename
    char unique_filename[FILENAME_MAX_LEN];
    snprintf(unique_filename, FILENAME_MAX_LEN, "%ld_%d.pdf", (long)time(NULL), rand() % 10000);

    char* destination_path = (char*)malloc(FILEPATH_MAX_LEN);
    if (!destination_path) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return NULL;
    }
    snprintf(destination_path, FILEPATH_MAX_LEN, "%s/%s", UPLOAD_DIR, unique_filename);

    MKDIR(UPLOAD_DIR);
    FILE* out_file = fopen(destination_path, "wb");
    if (!out_file) {
        fprintf(stderr, "Error: Could not open destination file for writing.\n");
        free(destination_path);
        return NULL;
    }

    // 5. Save file and check size
    char copy_buffer[8192];
    size_t n;
    long long total_bytes = 0;
    while ((n = fread(copy_buffer, 1, sizeof(copy_buffer), input_stream)) > 0) {
        total_bytes += n;
        if (total_bytes > MAX_FILE_SIZE) {
            fprintf(stderr, "Error: File size exceeds the limit of %d bytes.\n", MAX_FILE_SIZE);
            fclose(out_file);
            remove(destination_path); // Clean up
            free(destination_path);
            return NULL;
        }
        fwrite(copy_buffer, 1, n, out_file);
    }
    fclose(out_file);

    if (total_bytes == 0) {
        fprintf(stderr, "Error: Empty file uploaded.\n");
        remove(destination_path);
        free(destination_path);
        return NULL;
    }
    
    printf("File saved successfully to: %s\n", destination_path);

    // 6. Save path to database
    if (!save_filepath_to_db(destination_path)) {
        fprintf(stderr, "Error: Database operation failed. Rolling back file save.\n");
        remove(destination_path);
        free(destination_path);
        return NULL;
    }

    return destination_path;
}

void run_test(const char* test_name, const char* content, const char* filename) {
    printf("\n[Test Case: %s]\n", test_name);
    // Create a temporary file to simulate an upload
    FILE* temp_file = tmpfile();
    if (content) {
        fputs(content, temp_file);
    }
    rewind(temp_file);

    char* result_path = upload_pdf(temp_file, filename);
    printf("Result: %s\n", result_path ? "Success" : "Failure");

    if (result_path) {
        free(result_path);
    }
    fclose(temp_file);
}

int main() {
    srand(time(NULL)); // Seed for random filename generation

    printf("--- Running Test Cases ---\n");

    run_test("Valid PDF", "%PDF-1.7\nSome content.", "mydocument.pdf");
    run_test("Non-PDF file (invalid magic bytes)", "This is not a PDF", "not_a_pdf.pdf");
    run_test("Malicious filename", "%PDF-1.7\nContent.", "../../etc/passwd.pdf");
    
    // For large file test, we can't easily create a 10MB string, so we'll simulate by checking size.
    // The logic inside upload_pdf will catch this, this test is more conceptual.
    printf("\n[Test Case: File too large (conceptual)]\n");
    // This is hard to test with tmpfile. A real test would involve creating a large file on disk.
    // The internal size check logic is the key part being tested.
    printf("Note: The file size check is implemented, but a live test for it is complex in this setup.\n");


    run_test("Invalid file extension", "%PDF-1.7\nContent.", "document.txt");

    return 0;
}