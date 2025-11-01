#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>

#define UPLOAD_DIR "uploads"
#define MAX_FILE_SIZE (10 * 1024 * 1024) // 10 MB
#define PDF_MAGIC_BYTES "%PDF-"
#define PDF_MAGIC_BYTES_LEN 5
#define MAX_PATH_LEN 256

// Forward declaration
void save_filepath_to_database(const char* file_path);

/**
 * Creates a directory if it does not exist.
 * NOTE: This uses POSIX-specific `mkdir`. For Windows, you would use `_mkdir`.
 */
int create_directory_if_not_exists(const char* path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        if (mkdir(path, 0755) != 0) { // 0755 permissions: rwx for owner, rx for group/others
            perror("mkdir");
            return -1;
        }
    }
    return 0;
}

/**
 * Extracts the basename of a file path, mitigating path traversal.
 */
const char* get_basename(const char* path) {
    const char* basename = strrchr(path, '/');
    #ifdef _WIN32
        const char* basename_win = strrchr(path, '\\');
        if (basename_win > basename) basename = basename_win;
    #endif
    return basename ? basename + 1 : path;
}

/**
 * Simulates uploading a PDF file.
 * @param original_filename The original filename from the user.
 * @param file_content The byte content of the file.
 * @param content_size The size of the file content.
 * @return A dynamically allocated string with the sanitized filepath, or NULL on failure.
 *         The caller is responsible for freeing the returned string.
 */
char* upload_pdf(const char* original_filename, const char* file_content, size_t content_size) {
    if (original_filename == NULL || file_content == NULL) {
        fprintf(stderr, "Error: Invalid input provided.\n");
        return NULL;
    }

    // 1. Security: Check file size
    if (content_size > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: File size exceeds the limit of %d bytes.\n", MAX_FILE_SIZE);
        return NULL;
    }
    
    // 2. Security: Validate file type using magic bytes
    if (content_size < PDF_MAGIC_BYTES_LEN || strncmp(file_content, PDF_MAGIC_BYTES, PDF_MAGIC_BYTES_LEN) != 0) {
        fprintf(stderr, "Error: Invalid file type. Not a PDF.\n");
        return NULL;
    }

    // 3. Security: Prevent path traversal by extracting only the filename
    const char* base_filename = get_basename(original_filename);
    
    // Check extension
    const char* dot = strrchr(base_filename, '.');
    if (!dot || (strcmp(dot, ".pdf") != 0 && strcmp(dot, ".PDF") != 0)) {
        fprintf(stderr, "Error: Filename must end with .pdf.\n");
        return NULL;
    }

    // 4. Security: Generate a unique filename to prevent overwrites (using timestamp + basename)
    time_t t = time(NULL);
    char unique_filename[MAX_PATH_LEN];
    snprintf(unique_filename, sizeof(unique_filename), "%ld_%s", t, base_filename);
    
    if (create_directory_if_not_exists(UPLOAD_DIR) != 0) {
        return NULL;
    }
    
    char* destination_path = (char*)malloc(MAX_PATH_LEN);
    if (destination_path == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for path.\n");
        return NULL;
    }
    snprintf(destination_path, MAX_PATH_LEN, "%s/%s", UPLOAD_DIR, unique_filename);
    
    // Save the file
    FILE* fp = fopen(destination_path, "wb");
    if (fp == NULL) {
        perror("fopen");
        free(destination_path);
        return NULL;
    }
    
    if (fwrite(file_content, 1, content_size, fp) != content_size) {
        fprintf(stderr, "Error: Failed to write all bytes to file.\n");
        fclose(fp);
        free(destination_path);
        return NULL;
    }
    fclose(fp);
    
    printf("File saved successfully to: %s\n", destination_path);

    // 5. Security: Simulate saving the filepath to a database
    save_filepath_to_database(destination_path);

    return destination_path;
}

/**
 * Simulates saving a filepath to a MySQL database using parameterized queries to prevent SQL Injection.
 */
void save_filepath_to_database(const char* file_path) {
    printf("\n--- Simulating Database Interaction ---\n");
    // In a real C application using a library like libmysqlclient,
    // you would use mysql_stmt_prepare and mysql_stmt_bind_param.
    printf("SQL (Prepared Statement): INSERT INTO pdf_files (filepath) VALUES (?);\n");
    printf("Binding parameter 1: %s\n", file_path);
    printf("Executing query...\n");
    printf("Filepath successfully saved in the database.\n");
    printf("-------------------------------------\n\n");
}


int main() {
    printf("--- Running C Test Cases ---\n");
    char* saved_path = NULL;

    // Test Case 1: Valid PDF upload
    printf("--- Test Case 1: Valid PDF ---\n");
    const char* valid_pdf_content = "%PDF-1.5\n...";
    saved_path = upload_pdf("document.pdf", valid_pdf_content, strlen(valid_pdf_content));
    if (saved_path) free(saved_path);

    // Test Case 2: Invalid file type (not a PDF)
    printf("--- Test Case 2: Invalid File Type (TXT) ---\n");
    const char* invalid_content = "This is a text file.";
    saved_path = upload_pdf("not_a_pdf.txt", invalid_content, strlen(invalid_content));
    if (saved_path) free(saved_path);
    
    // Test Case 3: Path traversal attempt
    printf("--- Test Case 3: Path Traversal Attempt ---\n");
    saved_path = upload_pdf("../../etc/passwd.pdf", valid_pdf_content, strlen(valid_pdf_content));
    if (saved_path) free(saved_path);

    // Test Case 4: File too large
    printf("--- Test Case 4: File Too Large ---\n");
    char* large_content = (char*)malloc(MAX_FILE_SIZE + 1);
    if(large_content) {
        memcpy(large_content, valid_pdf_content, strlen(valid_pdf_content));
        saved_path = upload_pdf("large_file.pdf", large_content, MAX_FILE_SIZE + 1);
        if (saved_path) free(saved_path);
        free(large_content);
    }
    
    // Test Case 5: Another valid PDF to show unique naming
    printf("--- Test Case 5: Another Valid PDF ---\n");
    saved_path = upload_pdf("report.pdf", valid_pdf_content, strlen(valid_pdf_content));
    if (saved_path) free(saved_path);

    return 0;
}