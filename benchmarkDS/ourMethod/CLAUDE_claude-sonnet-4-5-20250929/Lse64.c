
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

/* Generate cryptographically secure random filename */
static int generate_random_filename(char* out, size_t out_len) {
    FILE* urandom = NULL;
    unsigned char random_bytes[16];
    size_t read_count = 0;
    size_t i, j;
    const char* hex = "0123456789abcdef";
    
    urandom = fopen("/dev/urandom", "rb");
    if (!urandom) return -1;
    
    read_count = fread(random_bytes, 1, sizeof(random_bytes), urandom);
    fclose(urandom);
    
    if (read_count != sizeof(random_bytes)) return -1;
    
    /* Convert to hex string */
    for (i = 0, j = 0; i < sizeof(random_bytes) && j < out_len - 5; i++) {
        out[j++] = hex[(random_bytes[i] >> 4) & 0xF];
        out[j++] = hex[random_bytes[i] & 0xF];
    }
    strcpy(&out[j], ".pdf");
    out[out_len - 1] = '\\0';
    return 0;
}

/* Sanitize filename to remove dangerous characters */
static void sanitize_filename(const char* input, char* output, size_t out_len) {
    size_t i, j = 0;
    char c;
    
    for (i = 0; input[i] != '\\0' && j < out_len - 1; i++) {
        c = input[i];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || 
            (c >= '0' && c <= '9') || c == '.' || c == '_' || c == '-') {
            output[j++] = c;
        } else {
            output[j++] = '_';
        }
    }
    output[j] = '\\0';
}

/* Validate path stays within base directory */
static int validate_path(const char* base, const char* filename, char* resolved, size_t resolved_len) {
    char full_path[MAX_PATH_LEN];
    char base_real[MAX_PATH_LEN];
    char* dir = NULL;
    char* last_slash = NULL;
    char dir_real[MAX_PATH_LEN];
    int ret;
    
    /* Get real path of base directory */
    if (realpath(base, base_real) == NULL) {
        return -1;
    }
    
    /* Construct full path */
    ret = snprintf(full_path, sizeof(full_path), "%s/%s", base_real, filename);
    if (ret < 0 || (size_t)ret >= sizeof(full_path)) {
        return -1;
    }
    
    /* Resolve directory part */
    dir = strdup(full_path);
    if (!dir) return -1;
    
    last_slash = strrchr(dir, '/');
    if (last_slash) {
        *last_slash = '\\0';
        if (realpath(dir, dir_real) != NULL) {
            snprintf(resolved, resolved_len, "%s/%s", dir_real, last_slash + 1);
        } else {
            free(dir);
            return -1;
        }
    } else {
        free(dir);
        return -1;
    }
    free(dir);
    
    /* Verify resolved path starts with base directory */
    if (strncmp(resolved, base_real, strlen(base_real)) != 0) {
        return -1;
    }
    
    return 0;
}

char* upload_pdf(const unsigned char* file_data, size_t file_size, const char* original_filename) {
    char sanitized[256];
    char random_name[64];
    char target_path[MAX_PATH_LEN];
    char temp_path[MAX_PATH_LEN];
    int temp_fd = -1;
    ssize_t written = 0;
    ssize_t ret;
    MYSQL* conn = NULL;
    MYSQL_STMT* stmt = NULL;
    MYSQL_BIND bind[2];
    const char* query = "INSERT INTO uploaded_files (filepath, original_name, upload_time) VALUES (?, ?, NOW())";
    const char* db_pass = NULL;
    char* result = NULL;
    
    /* Input validation */
    if (!file_data || file_size == 0) {
        fprintf(stderr, "Error: File data is empty\\n");
        return NULL;
    }
    
    if (file_size > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: File size exceeds maximum\\n");
        return NULL;
    }
    
    if (!original_filename || strlen(original_filename) == 0) {
        fprintf(stderr, "Error: Invalid filename\\n");
        return NULL;
    }
    
    /* Validate PDF magic bytes */
    if (file_size < 5 || memcmp(file_data, "%PDF-", 5) != 0) {
        fprintf(stderr, "Error: Not a valid PDF file\\n");
        return NULL;
    }
    
    /* Sanitize original filename */
    sanitize_filename(original_filename, sanitized, sizeof(sanitized));
    
    /* Generate random filename */
    if (generate_random_filename(random_name, sizeof(random_name)) != 0) {
        fprintf(stderr, "Error: Failed to generate random filename\\n");
        return NULL;
    }
    
    /* Validate and resolve target path */
    if (validate_path(UPLOAD_BASE_DIR, random_name, target_path, sizeof(target_path)) != 0) {
        fprintf(stderr, "Error: Path validation failed\\n");
        return NULL;
    }
    
    /* Create upload directory if needed */
    mkdir(UPLOAD_BASE_DIR, 0700);
    
    /* Create temp file with mkstemp for atomic write */
    snprintf(temp_path, sizeof(temp_path), "%s/upload_XXXXXX", UPLOAD_BASE_DIR);
    temp_fd = mkstemp(temp_path);
    if (temp_fd < 0) {
        fprintf(stderr, "Error: Failed to create temp file\\n");
        return NULL;
    }
    
    /* Set restrictive permissions */
    if (fchmod(temp_fd, S_IRUSR | S_IWUSR) != 0) {
        close(temp_fd);
        unlink(temp_path);
        return NULL;
    }
    
    /* Write data with proper error handling */
    written = 0;
    while (written < (ssize_t)file_size) {
        ret = write(temp_fd, file_data + written, file_size - written);
        if (ret < 0) {
            if (errno == EINTR) continue;
            close(temp_fd);
            unlink(temp_path);
            return NULL;
        }
        written += ret;
    }
    
    /* Flush to disk */
    if (fsync(temp_fd) != 0) {
        close(temp_fd);
        unlink(temp_path);
        return NULL;
    }
    
    close(temp_fd);
    
    /* Atomic rename */
    if (rename(temp_path, target_path) != 0) {
        unlink(temp_path);
        return NULL;
    }
    
    /* Store in database with prepared statement */
    conn = mysql_init(NULL);
    if (!conn) {
        unlink(target_path);
        return NULL;
    }
    
    mysql_ssl_set(conn, NULL, NULL, "/path/to/ca.pem", NULL, NULL);
    
    db_pass = getenv("DB_PASSWORD");
    if (!mysql_real_connect(conn, "localhost", "appuser", db_pass, "filedb", 0, NULL, CLIENT_SSL)) {
        mysql_close(conn);
        unlink(target_path);
        return NULL;
    }
    
    stmt = mysql_stmt_init(conn);
    if (!stmt) {
        mysql_close(conn);
        unlink(target_path);
        return NULL;
    }
    
    if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0) {
        mysql_stmt_close(stmt);
        mysql_close(conn);
        unlink(target_path);
        return NULL;
    }
    
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
    
    result = strdup(random_name);
    return result;
}

unsigned char* download_pdf(const char* filepath, size_t* out_size) {
    char sanitized[256];
    char target_path[MAX_PATH_LEN];
    int fd = -1;
    struct stat st;
    unsigned char* buffer = NULL;
    ssize_t total_read = 0;
    ssize_t ret;
    
    *out_size = 0;
    
    /* Input validation */
    if (!filepath || strlen(filepath) == 0) {
        fprintf(stderr, "Error: Invalid filepath\\n");
        return NULL;
    }
    
    /* Sanitize filepath */
    sanitize_filename(filepath, sanitized, sizeof(sanitized));
    
    if (strlen(sanitized) < 5 || strcmp(&sanitized[strlen(sanitized) - 4], ".pdf") != 0) {
        fprintf(stderr, "Error: Invalid filepath format\\n");
        return NULL;
    }
    
    /* Validate path */
    if (validate_path(UPLOAD_BASE_DIR, sanitized, target_path, sizeof(target_path)) != 0) {
        fprintf(stderr, "Error: Path validation failed\\n");
        return NULL;
    }
    
    /* Open with O_NOFOLLOW to prevent symlink attacks */
    fd = open(target_path, O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
    if (fd < 0) {
        fprintf(stderr, "Error: File not found\\n");
        return NULL;
    }
    
    /* Use fstat on open descriptor */
    if (fstat(fd, &st) != 0) {
        close(fd);
        return NULL;
    }
    
    /* Validate regular file */
    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "Error: Not a regular file\\n");
        close(fd);
        return NULL;
    }
    
    /* Validate size */
    if (st.st_size > MAX_FILE_SIZE || st.st_size < 5) {
        fprintf(stderr, "Error: File size out of bounds\\n");
        close(fd);
        return NULL;
    }
    
    /* Allocate and read */
    buffer = (unsigned char*)malloc(st.st_size);
    if (!buffer) {
        close(fd);
        return NULL;
    }
    
    total_read = 0;
    while (total_read < st.st_size) {
        ret = read(fd, buffer + total_read, st.st_size - total_read);
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
    
    /* Validate PDF magic bytes */
    if (total_read < 5 || memcmp(buffer, "%PDF-", 5) != 0) {
        fprintf(stderr, "Error: File corruption detected\\n");
        free(buffer);
        return NULL;
    }
    
    *out_size = total_read;
    return buffer;
}

int main(void) {
    const unsigned char valid_pdf[] = "%PDF-1.4\\ntest content";
    char* path1 = NULL;
    size_t size;
    unsigned char* downloaded = NULL;
    char* path3 = NULL;
    char* path4 = NULL;
    unsigned char* data5 = NULL;
    size_t size5;
    
    /* Test 1: Valid upload */
    path1 = upload_pdf(valid_pdf, sizeof(valid_pdf) - 1, "document.pdf");
    if (path1) {
        printf("Test 1 - Upload valid PDF: %s\\n", path1);
    } else {
        printf("Test 1 - Upload failed\\n");
    }
    
    /* Test 2: Download */
    if (path1) {
        downloaded = download_pdf(path1, &size);
        if (downloaded) {
            printf("Test 2 - Download PDF: %zu bytes\\n", size);
            free(downloaded);
        } else {
            printf("Test 2 - Download failed\\n");
        }
        free(path1);
    }
    
    /* Test 3: Invalid PDF */
    path3 = upload_pdf((const unsigned char*)"\\x00\\x00\\x00", 3, "invalid.pdf");
    if (!path3) {
        printf("Test 3 - Correctly rejected invalid PDF\\n");
    } else {
        printf("Test 3 - Should have rejected invalid PDF\\n");
        free(path3);
    }
    
    /* Test 4: Path traversal in upload */
    path4 = upload_pdf(valid_pdf, sizeof(valid_pdf) - 1, "../../../etc/passwd.pdf");
    if (!path4) {
        printf("Test 4 - Correctly blocked path traversal\\n");
    } else {
        printf("Test 4 - Should have blocked path traversal\\n");
        free(path4);
    }
    
    /* Test 5: Path traversal in download */
    data5 = download_pdf("../../../etc/passwd", &size5);
    if (!data5) {
        printf("Test 5 - Correctly blocked path traversal\\n");
    } else {
        printf("Test 5 - Should have blocked path traversal\\n");
        free(data5);
    }
    
    return 0;
}
