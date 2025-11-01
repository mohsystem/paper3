
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>

#define MAX_FILE_SIZE (10 * 1024 * 1024)
#define MAX_FILENAME_LEN 255
#define UPLOAD_DIR "uploads"
#define BUFFER_SIZE 8192

/* Allowed file extensions */
static const char* ALLOWED_EXTENSIONS[] = {
    "txt", "pdf", "jpg", "jpeg", "png", "doc", "docx", NULL
};

/* Function prototypes */
char* sanitize_filename(const char* filename);
char* get_file_extension(const char* filename);
int is_extension_allowed(const char* extension);
char* generate_unique_filename(const char* original_filename);
void create_upload_directory(void);
char* normalize_and_validate_path(const char* base_path, const char* filename);
int upload_file(const char* filename, const unsigned char* content, size_t content_size);

/* Secure file upload handler */
int upload_file(const char* filename, const unsigned char* content, size_t content_size) {
    char* sanitized_filename = NULL;
    char* extension = NULL;
    char* unique_filename = NULL;
    char* target_path = NULL;
    int fd = -1;
    int result = -1;
    struct stat st;
    ssize_t bytes_written = 0;
    
    /* Input validation: reject NULL or empty filename */
    if (filename == NULL || filename[0] == '\\0') {
        fprintf(stderr, "Error: Invalid filename\\n");
        goto cleanup;
    }
    
    /* Validate content pointer and size */
    if (content == NULL) {
        fprintf(stderr, "Error: Invalid content\\n");
        goto cleanup;
    }
    
    /* Validate file size */
    if (content_size > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: File size exceeds maximum allowed size\\n");
        goto cleanup;
    }
    
    /* Sanitize filename to prevent path traversal */
    sanitized_filename = sanitize_filename(filename);
    if (sanitized_filename == NULL || sanitized_filename[0] == '\\0') {
        fprintf(stderr, "Error: Invalid filename after sanitization\\n");
        goto cleanup;
    }
    
    /* Validate file extension */
    extension = get_file_extension(sanitized_filename);
    if (extension == NULL || !is_extension_allowed(extension)) {
        fprintf(stderr, "Error: File type not allowed\\n");
        goto cleanup;
    }
    
    /* Create uploads directory with restrictive permissions */
    create_upload_directory();
    
    /* Generate unique filename */
    unique_filename = generate_unique_filename(sanitized_filename);
    if (unique_filename == NULL) {
        fprintf(stderr, "Error: Failed to generate unique filename\\n");
        goto cleanup;
    }
    
    /* Normalize and validate target path */
    target_path = normalize_and_validate_path(UPLOAD_DIR, unique_filename);
    if (target_path == NULL) {
        fprintf(stderr, "Error: Path validation failed\\n");
        goto cleanup;
    }
    
    /* Open file with O_CREAT|O_EXCL|O_WRONLY for atomic creation
     * O_EXCL prevents TOCTOU by failing if file exists
     * O_NOFOLLOW prevents following symlinks */
    fd = open(target_path, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        fprintf(stderr, "Error: Failed to create file: %s\\n", strerror(errno));
        goto cleanup;
    }
    
    /* Validate opened file descriptor using fstat */
    if (fstat(fd, &st) != 0) {
        fprintf(stderr, "Error: Failed to stat file\\n");
        close(fd);
        unlink(target_path);
        goto cleanup;
    }
    
    /* Ensure it's a regular file */\n    if (!S_ISREG(st.st_mode)) {\n        fprintf(stderr, "Error: Target is not a regular file\\n");\n        close(fd);\n        unlink(target_path);\n        goto cleanup;\n    }\n    \n    /* Write content to file */\n    bytes_written = write(fd, content, content_size);\n    if (bytes_written < 0 || (size_t)bytes_written != content_size) {\n        fprintf(stderr, "Error: Failed to write file content\\n");\n        close(fd);\n        unlink(target_path);\n        goto cleanup;\n    }\n    \n    /* Flush to disk */\n    if (fsync(fd) != 0) {\n        fprintf(stderr, "Error: Failed to sync file to disk\\n");\n        close(fd);\n        unlink(target_path);\n        goto cleanup;\n    }\n    \n    close(fd);\n    printf("File uploaded successfully: %s\\n", unique_filename);\n    result = 0;\n    \ncleanup:\n    /* Free allocated memory */\n    if (sanitized_filename != NULL) free(sanitized_filename);\n    if (extension != NULL) free(extension);\n    if (unique_filename != NULL) free(unique_filename);\n    if (target_path != NULL) free(target_path);\n    \n    return result;\n}\n\nchar* sanitize_filename(const char* filename) {\n    if (filename == NULL) return NULL;\n    \n    size_t len = strlen(filename);\n    if (len == 0 || len > MAX_FILENAME_LEN) return NULL;\n    \n    /* Allocate memory for sanitized filename */\n    char* sanitized = (char*)malloc(len + 1);\n    if (sanitized == NULL) return NULL;\n    \n    size_t j = 0;\n    /* Remove path separators and null bytes */\n    for (size_t i = 0; i < len; i++) {\n        if (filename[i] != '/' && filename[i] != '\\\\' && filename[i] != '\\0') {\n            sanitized[j++] = filename[i];\n        }\n    }\n    sanitized[j] = '\\0';\n    \n    /* Remove leading/trailing dots and spaces */\n    size_t start = 0;\n    while (sanitized[start] == '.' || sanitized[start] == ' ') start++;\n    \n    size_t end = strlen(sanitized);\n    while (end > start && (sanitized[end - 1] == '.' || sanitized[end - 1] == ' ')) end--;\n    \n    if (start >= end) {\n        free(sanitized);\n        return NULL;\n    }\n    \n    memmove(sanitized, sanitized + start, end - start);\n    sanitized[end - start] = '\\0';\n    \n    return sanitized;\n}\n\nchar* get_file_extension(const char* filename) {\n    if (filename == NULL) return NULL;\n    \n    const char* dot = strrchr(filename, '.');\n    if (dot == NULL || dot == filename || *(dot + 1) == '\\0') {\n        return NULL;\n    }\n    \n    size_t ext_len = strlen(dot + 1);\n    char* extension = (char*)malloc(ext_len + 1);\n    if (extension == NULL) return NULL;\n    \n    strcpy(extension, dot + 1);\n    \n    /* Convert to lowercase for comparison */\n    for (size_t i = 0; i < ext_len; i++) {\n        extension[i] = tolower((unsigned char)extension[i]);\n    }\n    \n    return extension;\n}\n\nint is_extension_allowed(const char* extension) {\n    if (extension == NULL) return 0;\n    \n    for (int i = 0; ALLOWED_EXTENSIONS[i] != NULL; i++) {\n        if (strcmp(extension, ALLOWED_EXTENSIONS[i]) == 0) {\n            return 1;\n        }\n    }\n    return 0;\n}\n\nchar* generate_unique_filename(const char* original_filename) {\n    if (original_filename == NULL) return NULL;\n    \n    /* Generate random hex string */\n    unsigned char random_bytes[16];\n    FILE* urandom = fopen("/dev/urandom", "rb");\n    if (urandom == NULL) {\n        /* Fallback to time-based seed */\n        srand((unsigned int)time(NULL));\n        for (int i = 0; i < 16; i++) {\n            random_bytes[i] = (unsigned char)(rand() % 256);\n        }\n    } else {\n        if (fread(random_bytes, 1, 16, urandom) != 16) {\n            fclose(urandom);\n            return NULL;\n        }\n        fclose(urandom);\n    }\n    \n    char random_hex[33];\n    for (int i = 0; i < 16; i++) {\n        snprintf(random_hex + i * 2, 3, "%02x", random_bytes[i]);\n    }\n    random_hex[32] = '\\0';\n    \n    /* Extract base name and extension */\n    const char* dot = strrchr(original_filename, '.');\n    size_t base_len = dot ? (size_t)(dot - original_filename) : strlen(original_filename);\n    if (base_len > 50) base_len = 50;\n    \n    /* Allocate memory for unique filename */\n    size_t unique_len = base_len + 1 + 32 + (dot ? strlen(dot) : 0) + 1;\n    char* unique_filename = (char*)malloc(unique_len);\n    if (unique_filename == NULL) return NULL;\n    \n    /* Construct unique filename */\n    snprintf(unique_filename, unique_len, "%.*s_%s%s", \n             (int)base_len, original_filename, random_hex, dot ? dot : "");\n    \n    return unique_filename;\n}\n\nvoid create_upload_directory(void) {\n    struct stat st;\n    if (stat(UPLOAD_DIR, &st) != 0) {\n        /* Directory doesn't exist, create it with restrictive permissions */
        if (mkdir(UPLOAD_DIR, S_IRWXU) != 0) {
            fprintf(stderr, "Warning: Failed to create upload directory\\n");
        }
    }
}

char* normalize_and_validate_path(const char* base_path, const char* filename) {
    if (base_path == NULL || filename == NULL) return NULL;
    
    /* Check for path traversal patterns */
    if (strstr(filename, "..") != NULL) {
        fprintf(stderr, "Error: Path traversal attempt detected\\n");
        return NULL;
    }
    
    /* Construct full path */
    size_t path_len = strlen(base_path) + 1 + strlen(filename) + 1;
    char* full_path = (char*)malloc(path_len);
    if (full_path == NULL) return NULL;
    
    snprintf(full_path, path_len, "%s/%s", base_path, filename);
    
    /* Ensure path starts with base directory */
    if (strncmp(full_path, base_path, strlen(base_path)) != 0) {
        free(full_path);
        return NULL;
    }
    
    return full_path;
}

int main(void) {
    /* Test case 1: Valid text file */
    {
        const char* content1 = "Test file content";
        if (upload_file("test.txt", (const unsigned char*)content1, strlen(content1)) == 0) {
            printf("Test 1 passed\\n");
        } else {
            printf("Test 1 failed\\n");
        }
    }
    
    /* Test case 2: Path traversal attempt (should fail) */
    {
        const char* content2 = "Malicious content";
        if (upload_file("../../../etc/passwd.txt", (const unsigned char*)content2, strlen(content2)) != 0) {
            printf("Test 2 passed: Rejected path traversal\\n");
        } else {
            printf("Test 2 failed: Should have rejected path traversal\\n");
        }
    }
    
    /* Test case 3: Invalid file extension (should fail) */
    {
        const char* content3 = "<?php system($_GET['cmd']); ?>";
        if (upload_file("shell.php", (const unsigned char*)content3, strlen(content3)) != 0) {
            printf("Test 3 passed: Rejected invalid extension\\n");
        } else {
            printf("Test 3 failed: Should have rejected .php extension\\n");
        }
    }
    
    /* Test case 4: Valid PDF file */
    {
        const char* content4 = "%PDF-1.4 test";
        if (upload_file("document.pdf", (const unsigned char*)content4, strlen(content4)) == 0) {
            printf("Test 4 passed\\n");
        } else {
            printf("Test 4 failed\\n");
        }
    }
    
    /* Test case 5: Empty filename (should fail) */
    {
        const char* content5 = "Content";
        if (upload_file("", (const unsigned char*)content5, strlen(content5)) != 0) {
            printf("Test 5 passed: Rejected empty filename\\n");
        } else {
            printf("Test 5 failed: Should have rejected empty filename\\n");
        }
    }
    
    return 0;
}
