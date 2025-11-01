
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mysql/mysql.h>
#include <limits.h>
#include <errno.h>

#define UPLOAD_BASE_DIR "/var/app/uploads"
#define MAX_FILE_SIZE (10 * 1024 * 1024)
#define MAX_PATH_LEN 4096

// Secure random generation for filename
static int generate_random_filename(char* out, size_t out_len) {
    FILE* urandom = fopen("/dev/urandom", "rb");
    if (!urandom) return -1;
    
    unsigned char random_bytes[16];
    size_t read = fread(random_bytes, 1, sizeof(random_bytes), urandom);
    fclose(urandom);
    
    if (read != sizeof(random_bytes)) return -1;
    
    // Convert to hex string
    const char* hex = "0123456789abcdef";
    size_t i, j;
    for (i = 0, j = 0; i < sizeof(random_bytes) && j < out_len - 5; i++) {
        out[j++] = hex[(random_bytes[i] >> 4) & 0xF];
        out[j++] = hex[random_bytes[i] & 0xF];
    }
    strcpy(&out[j], ".pdf");
    out[out_len - 1] = '\\0';
    return 0;
}

// Sanitize filename: remove dangerous characters
static void sanitize_filename(const char* input, char* output, size_t out_len) {
    size_t i, j = 0;
    for (i = 0; input[i] != '\\0' && j < out_len - 1; i++) {
        char c = input[i];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || 
            (c >= '0' && c <= '9') || c == '.' || c == '_' || c == '-') {
            output[j++] = c;
        } else {
            output[j++] = '_';
        }
    }
    output[j] = '\\0';
}

// Validate path stays within base directory (prevent traversal)
static int validate_path(const char* base, const char* filename, char* resolved, size_t resolved_len) {
    char full_path[MAX_PATH_LEN];
    char base_real[MAX_PATH_LEN];
    
    // Get real path of base directory
    if (realpath(base, base_real) == NULL) {
        return -1;
    }
    
    // Construct full path
    int ret = snprintf(full_path, sizeof(full_path), "%s/%s", base_real, filename);
    if (ret < 0 || (size_t)ret >= sizeof(full_path)) {
        return -1;
    }
    
    // Resolve the full path (but don't follow symlinks in final component)\n    char* dir = strdup(full_path);\n    if (!dir) return -1;\n    \n    char* last_slash = strrchr(dir, '/');\n    if (last_slash) {\n        *last_slash = '\\0';\n        char dir_real[MAX_PATH_LEN];\n        if (realpath(dir, dir_real) != NULL) {\n            snprintf(resolved, resolved_len, "%s/%s", dir_real, last_slash + 1);\n        } else {\n            free(dir);\n            return -1;\n        }\n    }\n    free(dir);\n    \n    // Verify resolved path starts with base directory\n    if (strncmp(resolved, base_real, strlen(base_real)) != 0) {\n        return -1; // Path traversal attempt\n    }\n    \n    return 0;\n}\n\nchar* upload_pdf(const unsigned char* file_data, size_t file_size, const char* original_filename) {\n    // Input validation\n    if (!file_data || file_size == 0) {\n        fprintf(stderr, "Error: File data is empty\\n");\n        return NULL;\n    }\n    \n    if (file_size > MAX_FILE_SIZE) {\n        fprintf(stderr, "Error: File size exceeds maximum\\n");\n        return NULL;\n    }\n    \n    if (!original_filename || strlen(original_filename) == 0) {\n        fprintf(stderr, "Error: Invalid filename\\n");\n        return NULL;\n    }\n    \n    // Validate PDF magic bytes\n    if (file_size < 5 || memcmp(file_data, "%PDF-", 5) != 0) {\n        fprintf(stderr, "Error: Not a valid PDF file\\n");\n        return NULL;\n    }\n    \n    // Sanitize original filename\n    char sanitized[256];\n    sanitize_filename(original_filename, sanitized, sizeof(sanitized));\n    \n    // Generate random filename\n    char random_name[64];\n    if (generate_random_filename(random_name, sizeof(random_name)) != 0) {\n        fprintf(stderr, "Error: Failed to generate random filename\\n");\n        return NULL;\n    }\n    \n    // Validate and resolve target path\n    char target_path[MAX_PATH_LEN];\n    if (validate_path(UPLOAD_BASE_DIR, random_name, target_path, sizeof(target_path)) != 0) {\n        fprintf(stderr, "Error: Path validation failed\\n");\n        return NULL;\n    }\n    \n    // Create upload directory if it doesn't exist
    mkdir(UPLOAD_BASE_DIR, 0700);
    
    // Write file securely: create temp file with O_CREAT|O_EXCL
    char temp_path[MAX_PATH_LEN];
    snprintf(temp_path, sizeof(temp_path), "%s/upload_XXXXXX", UPLOAD_BASE_DIR);
    
    int temp_fd = mkstemp(temp_path);
    if (temp_fd < 0) {
        fprintf(stderr, "Error: Failed to create temp file\\n");
        return NULL;
    }
    
    // Set restrictive permissions (0600)
    if (fchmod(temp_fd, S_IRUSR | S_IWUSR) != 0) {
        close(temp_fd);
        unlink(temp_path);
        return NULL;
    }
    
    // Write data
    ssize_t written = 0;
    while (written < (ssize_t)file_size) {
        ssize_t ret = write(temp_fd, file_data + written, file_size - written);
        if (ret < 0) {
            if (errno == EINTR) continue;
            close(temp_fd);
            unlink(temp_path);
            return NULL;
        }
        written += ret;
    }
    
    // Ensure data is flushed to disk
    if (fsync(temp_fd) != 0) {
        close(temp_fd);
        unlink(temp_path);
        return NULL;
    }
    
    close(temp_fd);
    
    // Atomic rename to final location
    if (rename(temp_path, target_path) != 0) {
        unlink(temp_path);
        return NULL;
    }
    
    // Store in database using parameterized query
    MYSQL* conn = mysql_init(NULL);
    if (!conn) {
        unlink(target_path);
        return NULL;
    }
    
    // Enable SSL for database connection
    mysql_ssl_set(conn, NULL, NULL, "/path/to/ca.pem", NULL, NULL);
    
    const char* db_pass = getenv("DB_PASSWORD");
    if (!mysql_real_connect(conn, "localhost", "appuser", db_pass, "filedb", 0, NULL, CLIENT_SSL)) {
        mysql_close(conn);
        unlink(target_path);
        return NULL;
    }
    
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    if (!stmt) {
        mysql_close(conn);
        unlink(target_path);
        return NULL;
    }
    
    const char* query = "INSERT INTO uploaded_files (filepath, original_name, upload_time) VALUES (?, ?, NOW())";
    if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0) {
        mysql_stmt_close(stmt);
        mysql_close(conn);
        unlink(target_path);
        return NULL;
    }
    
    MYSQL_BIND bind[2];
    memset(bind, 0, sizeof(bind));
    
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = random_name;
    bind[0].buffer_length = strlen(random_name);
    
    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = sanitized;
    bind[1].buffer_length = strlen(sanitized);
    
    if (mysql_stmt_bind_param(stmt, bind) != 0 || mysql_stmt_execute(stmt) != 0) {
        mysql_stmt_close(stmt);
        mysql_close(conn);
        unlink(target_path);
        return NULL;
    }
    
    mysql_stmt_close(stmt);
    mysql_close(conn);
    
    // Return allocated copy of filepath
    char* result = strdup(random_name);
    return result;
}

unsigned char* download_pdf(const char* filepath, size_t* out_size) {
    *out_size = 0;
    
    // Input validation
    if (!filepath || strlen(filepath) == 0) {
        fprintf(stderr, "Error: Invalid filepath\\n");
        return NULL;
    }
    
    // Sanitize filepath
    char sanitized[256];
    sanitize_filename(filepath, sanitized, sizeof(sanitized));
    
    if (strlen(sanitized) < 5 || strcmp(&sanitized[strlen(sanitized) - 4], ".pdf") != 0) {
        fprintf(stderr, "Error: Invalid filepath format\\n");
        return NULL;
    }
    
    // Validate and resolve path
    char target_path[MAX_PATH_LEN];
    if (validate_path(UPLOAD_BASE_DIR, sanitized, target_path, sizeof(target_path)) != 0) {
        fprintf(stderr, "Error: Path validation failed\\n");
        return NULL;
    }
    
    // Open file with O_NOFOLLOW to prevent symlink attacks
    int fd = open(target_path, O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
    if (fd < 0) {
        fprintf(stderr, "Error: File not found\\n");
        return NULL;
    }
    
    // Use fstat on open descriptor (race-safe)
    struct stat st;
    if (fstat(fd, &st) != 0) {
        close(fd);
        return NULL;
    }
    
    // Validate it's a regular file
    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "Error: Not a regular file\\n");
        close(fd);
        return NULL;
    }
    
    // Validate size
    if (st.st_size > MAX_FILE_SIZE || st.st_size < 5) {
        fprintf(stderr, "Error: File size out of bounds\\n");
        close(fd);
        return NULL;
    }
    
    // Allocate buffer and read file
    unsigned char* buffer = (unsigned char*)malloc(st.st_size);
    if (!buffer) {
        close(fd);
        return NULL;
    }
    
    ssize_t total_read = 0;
    while (total_read < st.st_size) {
        ssize_t ret = read(fd, buffer + total_read, st.st_size - total_read);
        if (ret < 0) {
            if (errno == EINTR) continue;
            free(buffer);
            close(fd);
            return NULL;
        }
        if (ret == 0) break;
        total_read += ret;
    }
    
    close(fd);
    
    // Validate PDF magic bytes
    if (total_read < 5 || memcmp(buffer, "%PDF-", 5) != 0) {
        fprintf(stderr, "Error: File corruption detected\\n");
        free(buffer);
        return NULL;
    }
    
    *out_size = total_read;
    return buffer;
}

int main() {
    // Test case 1: Valid PDF upload
    const unsigned char valid_pdf[] = "%PDF-1.4\\ntest content";
    char* path1 = upload_pdf(valid_pdf, sizeof(valid_pdf) - 1, "document.pdf");
    if (path1) {
        printf("Test 1 - Upload valid PDF: %s\\n", path1);
    } else {
        printf("Test 1 - Upload failed\\n");
    }
    
    // Test case 2: Download uploaded file
    if (path1) {
        size_t size;
        unsigned char* downloaded = download_pdf(path1, &size);
        if (downloaded) {
            printf("Test 2 - Download PDF: %zu bytes\\n", size);
            free(downloaded);
        } else {
            printf("Test 2 - Download failed\\n");
        }
        free(path1);
    }
    
    // Test case 3: Invalid PDF (missing magic bytes)
    const unsigned char invalid_pdf[] = "\\x00\\x00\\x00";
    char* path3 = upload_pdf(invalid_pdf, sizeof(invalid_pdf) - 1, "invalid.pdf");
    if (!path3) {
        printf("Test 3 - Correctly rejected invalid PDF\\n");
    } else {
        printf("Test 3 - Should have rejected invalid PDF\\n");
        free(path3);
    }
    
    // Test case 4: Path traversal attempt in upload
    char* path4 = upload_pdf(valid_pdf, sizeof(valid_pdf) - 1, "../../../etc/passwd.pdf");
    if (!path4) {
        printf("Test 4 - Correctly blocked path traversal\\n");
    } else {
        printf("Test 4 - Should have blocked path traversal\\n");
        free(path4);
    }
    
    // Test case 5: Path traversal attempt in download
    size_t size5;
    unsigned char* data5 = download_pdf("../../../etc/passwd", &size5);
    if (!data5) {
        printf("Test 5 - Correctly blocked path traversal\\n");
    } else {
        printf("Test 5 - Should have blocked path traversal\\n");
        free(data5);
    }
    
    return 0;
}
