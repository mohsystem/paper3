
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <time.h>

#define UPLOAD_DIR "uploads"
#define MAX_FILE_SIZE (10 * 1024 * 1024)
#define MAX_FILENAME_LENGTH 255
#define BUFFER_SIZE 8192

/* Secure random generation for unique filenames */
static void generate_random_id(char *output, size_t len) {
    FILE *f = fopen("/dev/urandom", "rb");
    if (!f) {
        /* Fallback to time-based seed if /dev/urandom unavailable */
        srand((unsigned int)time(NULL));
        size_t i;
        for (i = 0; i < len - 1; i++) {
            output[i] = "0123456789abcdef"[rand() % 16];
        }
        output[len - 1] = '\\0';
        return;
    }
    
    unsigned char bytes[16];
    if (fread(bytes, 1, sizeof(bytes), f) != sizeof(bytes)) {
        fclose(f);
        output[0] = '\\0';
        return;
    }
    fclose(f);
    
    size_t i;
    for (i = 0; i < sizeof(bytes) && i * 2 < len - 1; i++) {
        snprintf(output + i * 2, 3, "%02x", bytes[i]);
    }
}

/* Sanitize filename: remove path components and dangerous characters */
static void sanitize_filename(char *output, const char *input, size_t max_len) {
    if (!input || !output || max_len == 0) return;
    
    /* Extract basename only (remove directory path) */
    const char *basename = strrchr(input, '/');
    if (basename) {
        basename++;
    } else {
        basename = strrchr(input, '\\\\');\n        basename = basename ? basename + 1 : input;\n    }\n    \n    /* Copy only alphanumeric and safe characters */\n    size_t i, j = 0;\n    for (i = 0; basename[i] && j < max_len - 1; i++) {\n        char c = basename[i];\n        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||\n            (c >= '0' && c <= '9') || c == '.' || c == '_' || c == '-') {\n            output[j++] = c;\n        } else {\n            output[j++] = '_';\n        }\n    }\n    output[j] = '\\0';\n}\n\n/* Validate PDF magic bytes (PDF header: %PDF-) */\nstatic int is_valid_pdf(const unsigned char *data, size_t len) {\n    if (len < 5) return 0;\n    return (data[0] == 0x25 && data[1] == 0x50 && \n            data[2] == 0x44 && data[3] == 0x46 && data[4] == 0x2D);\n}\n\n/* Check if path is within base directory (prevent path traversal) */\nstatic int is_path_safe(const char *base_dir, const char *path) {\n    char real_base[PATH_MAX];\n    char real_path[PATH_MAX];\n    \n    /* Resolve base directory to canonical path */\n    if (!realpath(base_dir, real_base)) return 0;\n    \n    /* For non-existent files, check parent directory */\n    if (!realpath(path, real_path)) {\n        char parent[PATH_MAX];\n        strncpy(parent, path, sizeof(parent) - 1);\n        parent[sizeof(parent) - 1] = '\\0';\n        \n        char *last_slash = strrchr(parent, '/');\n        if (last_slash) {\n            *last_slash = '\\0';\n            if (!realpath(parent, real_path)) return 0;\n        } else {\n            return 0;\n        }\n    }\n    \n    /* Verify the resolved path starts with base directory */\n    size_t base_len = strlen(real_base);\n    return (strncmp(real_path, real_base, base_len) == 0);\n}\n\n/* Save file securely with comprehensive validation */\nchar* save_file(const char *filename, const unsigned char *content, size_t content_len) {\n    static char result[512];\n    result[0] = '\\0';\n    \n    /* Input validation: check all parameters */\n    if (!filename || !content || content_len == 0 || content_len > MAX_FILE_SIZE) {\n        snprintf(result, sizeof(result), "Invalid file data");\n        return result;\n    }\n    \n    /* Validate filename length */\n    if (strlen(filename) > MAX_FILENAME_LENGTH) {\n        snprintf(result, sizeof(result), "Filename too long");\n        return result;\n    }\n    \n    /* Sanitize filename to prevent path traversal and injection */\n    char sanitized[MAX_FILENAME_LENGTH + 1];\n    sanitize_filename(sanitized, filename, sizeof(sanitized));\n    \n    /* Validate PDF extension */\n    size_t len = strlen(sanitized);\n    if (len < 5 || strcasecmp(sanitized + len - 4, ".pdf") != 0) {\n        snprintf(result, sizeof(result), "Only PDF files allowed");\n        return result;\n    }\n    \n    /* Validate PDF magic bytes to ensure file type */\n    if (!is_valid_pdf(content, content_len)) {\n        snprintf(result, sizeof(result), "Not a valid PDF file");\n        return result;\n    }\n    \n    /* Create uploads directory with restrictive permissions (owner-only) */\n    struct stat st;\n    if (stat(UPLOAD_DIR, &st) != 0) {\n        if (mkdir(UPLOAD_DIR, 0700) != 0) {\n            snprintf(result, sizeof(result), "Failed to create upload directory");\n            return result;\n        }\n    }\n    \n    /* Generate unique filename to prevent overwrites */\n    char random_id[33];\n    generate_random_id(random_id, sizeof(random_id));\n    \n    char safe_filename[PATH_MAX];\n    snprintf(safe_filename, sizeof(safe_filename), "%s/%s_%s", \n             UPLOAD_DIR, random_id, sanitized);\n    \n    /* Verify path safety against traversal attacks */\n    if (!is_path_safe(UPLOAD_DIR, safe_filename)) {\n        snprintf(result, sizeof(result), "Path traversal attempt detected");\n        return result;\n    }\n    \n    /* Open file with secure flags:\n       O_WRONLY: write-only access\n       O_CREAT|O_EXCL: atomic creation, fail if exists\n       O_NOFOLLOW: prevent symlink attacks\n       O_CLOEXEC: close on exec for security\n       0600: owner read/write only */\n    int fd = open(safe_filename, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600);\n    if (fd < 0) {\n        snprintf(result, sizeof(result), "Failed to create file");\n        return result;\n    }\n    \n    /* Validate the opened file descriptor to ensure it's a regular file */
    struct stat file_stat;
    if (fstat(fd, &file_stat) != 0 || !S_ISREG(file_stat.st_mode)) {
        close(fd);
        unlink(safe_filename);
        snprintf(result, sizeof(result), "File validation failed");
        return result;
    }
    
    /* Write content in chunks, checking each write operation */
    size_t written = 0;
    while (written < content_len) {
        ssize_t w = write(fd, content + written, content_len - written);
        if (w <= 0) {
            close(fd);
            unlink(safe_filename);
            snprintf(result, sizeof(result), "Write failed");
            return result;
        }
        written += (size_t)w;
    }
    
    /* Ensure data is written to disk before closing */
    if (fsync(fd) != 0) {
        close(fd);
        unlink(safe_filename);
        snprintf(result, sizeof(result), "Sync failed");
        return result;
    }
    
    close(fd);
    
    snprintf(result, sizeof(result), "File uploaded successfully: %s_%s", 
             random_id, sanitized);
    return result;
}

int main(void) {
    printf("PDF Upload Server - Secure File Handler\\n\\n");
    printf("Test Cases:\\n");
    printf("1. Valid PDF upload\\n");
    printf("2. File too large\\n");
    printf("3. Invalid filename with path traversal\\n");
    printf("4. Non-PDF file\\n");
    printf("5. Empty file\\n\\n");
    
    /* Test case 1: Valid PDF with proper header */
    unsigned char valid_pdf[] = "%PDF-1.4\\n%Test content";
    char *result1 = save_file("test.pdf", valid_pdf, sizeof(valid_pdf) - 1);
    printf("Test 1: %s\\n", result1);
    
    /* Test case 2: File too large - validation prevents this */
    printf("Test 2: File too large (check implemented in validation)\\n");
    
    /* Test case 3: Path traversal attempt - should be blocked */
    char *result3 = save_file("../../../etc/passwd.pdf", valid_pdf, sizeof(valid_pdf) - 1);
    printf("Test 3: %s\\n", result3);
    
    /* Test case 4: Non-PDF file - should fail magic byte check */
    unsigned char invalid[] = "Not a PDF file";
    char *result4 = save_file("test.pdf", invalid, sizeof(invalid) - 1);
    printf("Test 4: %s\\n", result4);
    
    /* Test case 5: Empty file - should fail validation */
    char *result5 = save_file("test.pdf", NULL, 0);
    printf("Test 5: %s\\n", result5);
    
    return 0;
}
