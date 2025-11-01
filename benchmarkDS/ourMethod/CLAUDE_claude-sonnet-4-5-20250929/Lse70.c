
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define PATH_MAX MAX_PATH
#endif

#define MAX_FILE_SIZE (10 * 1024 * 1024) /* 10MB max file size */
#define MAX_FILENAME_LENGTH 255
#define UPLOAD_DIR "uploads"

/* Generate cryptographically secure random filename */
char* generate_secure_filename(void) {
    static char filename[MAX_FILENAME_LENGTH];
    FILE* urandom = NULL;
    unsigned char random_bytes[16];
    size_t i;
    
#ifdef _WIN32
    /* Windows: use CryptGenRandom for secure random */
    HCRYPTPROV hCryptProv;
    if (!CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        return NULL;
    }
    if (!CryptGenRandom(hCryptProv, sizeof(random_bytes), random_bytes)) {
        CryptReleaseContext(hCryptProv, 0);
        return NULL;
    }
    CryptReleaseContext(hCryptProv, 0);
#else
    /* POSIX: use /dev/urandom for secure random */
    urandom = fopen("/dev/urandom", "rb");
    if (urandom == NULL) {
        return NULL;
    }
    
    if (fread(random_bytes, 1, sizeof(random_bytes), urandom) != sizeof(random_bytes)) {
        fclose(urandom);
        return NULL;
    }
    fclose(urandom);
#endif
    
    /* Convert to hex string */
    for (i = 0; i < sizeof(random_bytes); i++) {
        snprintf(filename + (i * 2), 3, "%02x", random_bytes[i]);
    }
    strncat(filename, ".bin", MAX_FILENAME_LENGTH - strlen(filename) - 1);
    
    return filename;
}

/* Create directory with restrictive permissions */
int create_upload_directory(void) {
#ifdef _WIN32
    if (_mkdir(UPLOAD_DIR) != 0 && errno != EEXIST) {
        return 0;
    }
#else
    /* Create directory with restrictive permissions (0700 = owner only) */
    if (mkdir(UPLOAD_DIR, 0700) != 0 && errno != EEXIST) {
        return 0;
    }
#endif
    return 1;
}

/* Validate path to prevent directory traversal */
int is_safe_path(const char* base_dir, const char* filename, char* out_path, size_t out_size) {
    char full_path[PATH_MAX];
    
    /* Check filename length */
    if (strlen(filename) > MAX_FILENAME_LENGTH) {
        return 0;
    }
    
    /* Check for path traversal patterns */
    if (strstr(filename, "..") != NULL || strstr(filename, "/") != NULL || 
        strstr(filename, "\\\\") != NULL) {\n        return 0;\n    }\n    \n    /* Construct full path */\n    if (snprintf(full_path, sizeof(full_path), "%s/%s", base_dir, filename) >= sizeof(full_path)) {\n        return 0;\n    }\n    \n#ifdef _WIN32\n    char resolved[PATH_MAX];\n    if (_fullpath(resolved, full_path, PATH_MAX) == NULL) {\n        return 0;\n    }\n    \n    char base_resolved[PATH_MAX];\n    if (_fullpath(base_resolved, base_dir, PATH_MAX) == NULL) {\n        return 0;\n    }\n    \n    if (strncmp(resolved, base_resolved, strlen(base_resolved)) != 0) {\n        return 0;\n    }\n    \n    strncpy(out_path, resolved, out_size - 1);\n    out_path[out_size - 1] = '\\0';\n#else\n    /* For POSIX, verify base directory exists and construct path */\n    char base_resolved[PATH_MAX];\n    if (realpath(base_dir, base_resolved) == NULL) {\n        return 0;\n    }\n    \n    /* Manually construct target path */\n    if (snprintf(out_path, out_size, "%s/%s", base_resolved, filename) >= (int)out_size) {\n        return 0;\n    }\n#endif\n    \n    return 1;\n}\n\n/* Explicit zero memory to clear sensitive data */\nvoid explicit_zero(void* ptr, size_t size) {\n    volatile unsigned char* p = (volatile unsigned char*)ptr;\n    while (size--) {\n        *p++ = 0;\n    }\n}\n\nchar* upload_file(const unsigned char* file_content, size_t content_size, const char* content_type) {\n    static char result_msg[256];\n    char* safe_filename = NULL;\n    char target_path[PATH_MAX];\n    int fd = -1;\n    ssize_t written;\n    \n    /* Initialize result message */\n    memset(result_msg, 0, sizeof(result_msg));\n    \n    /* Validate content type */\n    if (content_type == NULL || strstr(content_type, "multipart/form-data") == NULL) {\n        strncpy(result_msg, "Error: Invalid content type", sizeof(result_msg) - 1);\n        return result_msg;\n    }\n    \n    /* Enforce maximum file size to prevent DoS attacks */\n    if (content_size > MAX_FILE_SIZE) {\n        strncpy(result_msg, "Error: File too large", sizeof(result_msg) - 1);\n        return result_msg;\n    }\n    \n    /* Create upload directory if it doesn't exist */\n    if (!create_upload_directory()) {\n        strncpy(result_msg, "Error: Failed to create upload directory", sizeof(result_msg) - 1);\n        return result_msg;\n    }\n    \n    /* Generate cryptographically secure random filename */\n    safe_filename = generate_secure_filename();\n    if (safe_filename == NULL) {\n        strncpy(result_msg, "Error: Failed to generate filename", sizeof(result_msg) - 1);\n        return result_msg;\n    }\n    \n    /* Validate and normalize path to prevent directory traversal */\n    if (!is_safe_path(UPLOAD_DIR, safe_filename, target_path, sizeof(target_path))) {\n        strncpy(result_msg, "Error: Invalid file path", sizeof(result_msg) - 1);\n        return result_msg;\n    }\n    \n#ifdef _WIN32\n    /* Windows implementation */\n    HANDLE hFile = CreateFileA(\n        target_path,\n        GENERIC_WRITE,\n        0,\n        NULL,\n        CREATE_NEW,\n        FILE_ATTRIBUTE_NORMAL,\n        NULL\n    );\n    \n    if (hFile == INVALID_HANDLE_VALUE) {\n        strncpy(result_msg, "Error: Upload failed", sizeof(result_msg) - 1);\n        return result_msg;\n    }\n    \n    DWORD bytesWritten;\n    BOOL write_result = WriteFile(hFile, file_content, (DWORD)content_size, &bytesWritten, NULL);\n    FlushFileBuffers(hFile);\n    CloseHandle(hFile);\n    \n    if (!write_result || bytesWritten != content_size) {\n        DeleteFileA(target_path);\n        strncpy(result_msg, "Error: Upload failed", sizeof(result_msg) - 1);\n        return result_msg;\n    }\n#else\n    /* POSIX: use open with O_CREAT|O_EXCL|O_NOFOLLOW to prevent race conditions */\n    fd = open(target_path, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600);\n    if (fd < 0) {\n        strncpy(result_msg, "Error: Upload failed", sizeof(result_msg) - 1);\n        return result_msg;\n    }\n    \n    /* Validate that we opened a regular file using fstat on the opened fd */\n    struct stat st;\n    if (fstat(fd, &st) != 0 || !S_ISREG(st.st_mode)) {\n        close(fd);\n        unlink(target_path);\n        strncpy(result_msg, "Error: Invalid file type", sizeof(result_msg) - 1);\n        return result_msg;\n    }\n    \n    /* Write data to the opened file descriptor */\n    written = write(fd, file_content, content_size);\n    \n    /* Ensure data is written to disk */\n    if (fsync(fd) != 0) {\n        close(fd);\n        unlink(target_path);\n        strncpy(result_msg, "Error: Upload failed", sizeof(result_msg) - 1);\n        return result_msg;\n    }\n    \n    close(fd);\n    \n    if (written < 0 || (size_t)written != content_size) {\n        unlink(target_path);\n        strncpy(result_msg, "Error: Upload failed", sizeof(result_msg) - 1);\n        return result_msg;\n    }\n#endif\n    \n    snprintf(result_msg, sizeof(result_msg), "File uploaded successfully: %s", safe_filename);\n    return result_msg;\n}\n\nvoid run_test_cases(void) {\n    printf("\
=== Running Test Cases ===\
");\n    \n    /* Test case 1: Valid small file upload */\n    const char* test1_data = "Test content 1";\n    char* result1 = upload_file((const unsigned char*)test1_data, strlen(test1_data), "multipart/form-data");\n    printf("Test 1 (valid upload): %s\
", result1);\n    \n    /* Test case 2: Another valid upload */\n    const char* test2_data = "Test content 2";\n    char* result2 = upload_file((const unsigned char*)test2_data, strlen(test2_data), "multipart/form-data");\n    printf("Test 2 (valid upload): %s\
", result2);\n    \n    /* Test case 3: Invalid content type */\n    const char* test3_data = "Test content 3";\n    char* result3 = upload_file((const unsigned char*)test3_data, strlen(test3_data), "text/plain");\n    printf("Test 3 (invalid content type): %s\
", result3);\n    \n    /* Test case 4: Large file (1KB) */\n    unsigned char* test4_data = (unsigned char*)malloc(1024);\n    if (test4_data != NULL) {\n        memset(test4_data, 'x', 1024);\n        char* result4 = upload_file(test4_data, 1024, "multipart/form-data");\n        printf("Test 4 (1KB file): %s\
", result4);\n        explicit_zero(test4_data, 1024);\n        free(test4_data);\n    }\n    \n    /* Test case 5: Empty file */\n    char* result5 = upload_file((const unsigned char*)"", 0, "multipart/form-data");\n    printf("Test 5 (empty file): %s\
", result5);
}

int main(void) {
    run_test_cases();
    return 0;
}
