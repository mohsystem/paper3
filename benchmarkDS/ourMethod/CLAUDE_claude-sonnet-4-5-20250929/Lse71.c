
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#endif

/* Security: Define maximum file size to prevent DoS (10MB) */
#define MAX_FILE_SIZE (10 * 1024 * 1024)
/* Security: Define upload directory */
#define UPLOAD_DIR "uploads"
/* Security: Maximum filename length to prevent buffer issues */
#define MAX_FILENAME_LENGTH 255
/* Security: Maximum path length */
#define MAX_PATH_LENGTH 512

/* Security: Allowed file extensions whitelist */
static const char* ALLOWED_EXTENSIONS[] = {".txt", ".pdf", ".jpg", ".png", NULL};

/* Security: Check if extension is allowed */
static int is_extension_allowed(const char* extension) {
    if (extension == NULL) {
        return 0;
    }

    for (int i = 0; ALLOWED_EXTENSIONS[i] != NULL; i++) {
        if (strcasecmp(extension, ALLOWED_EXTENSIONS[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

/* Security: Extract basename to prevent path traversal */
static const char* get_basename(const char* path) {
    if (path == NULL) {
        return NULL;
    }

    const char* last_slash = strrchr(path, '/');
    const char* last_backslash = strrchr(path, '\\\\');\n    const char* basename = path;\n\n    if (last_slash != NULL && last_slash > basename) {\n        basename = last_slash + 1;\n    }\n    if (last_backslash != NULL && last_backslash > basename) {\n        basename = last_backslash + 1;\n    }\n\n    return basename;\n}\n\n/* Security: Get file extension */\nstatic const char* get_extension(const char* filename) {\n    if (filename == NULL) {\n        return NULL;\n    }\n\n    const char* dot = strrchr(filename, '.');\n    if (dot != NULL && dot > filename) {\n        return dot;\n    }\n    return NULL;\n}\n\n/* Security: Validate filename contains only safe characters */\nstatic int is_safe_filename(const char* filename) {\n    if (filename == NULL || strlen(filename) == 0) {\n        return 0;\n    }\n\n    for (size_t i = 0; i < strlen(filename); i++) {\n        char c = filename[i];\n        if (!isalnum((unsigned char)c) && c != '_' && c != '.' && c != '-') {\n            return 0;\n        }\n    }\n    return 1;\n}\n\n/* Security: Validate filename */\nstatic int is_valid_filename(const char* filename) {\n    if (filename == NULL || strlen(filename) == 0 || strlen(filename) > MAX_FILENAME_LENGTH) {\n        return 0;\n    }\n\n    /* Security: Extract basename to prevent path traversal */\n    const char* basename = get_basename(filename);\n    if (basename == NULL || strlen(basename) == 0) {\n        return 0;\n    }\n\n    /* Security: Validate filename contains only safe characters */\n    if (!is_safe_filename(basename)) {\n        return 0;\n    }\n\n    /* Security: Check file extension against whitelist */\n    const char* extension = get_extension(basename);\n    if (!is_extension_allowed(extension)) {\n        return 0;\n    }\n\n    return 1;\n}\n\n/* Security: Create upload directory with restrictive permissions */\nstatic int create_upload_directory(void) {\n    struct stat st = {0};\n\n    if (stat(UPLOAD_DIR, &st) == -1) {\n        /* Security: Create directory with mode 0700 (owner only) */\n        if (mkdir(UPLOAD_DIR, 0700) != 0) {\n            perror("Failed to create upload directory");\n            return 0;\n        }\n    }\n    return 1;\n}\n\n/* Security: Save file with validation and safe operations */\nstatic const char* save_file(const char* filename, const unsigned char* content, size_t content_size) {\n    if (filename == NULL || content == NULL) {\n        return "Error: Invalid input";\n    }\n\n    /* Security: Validate content size */\n    if (content_size > MAX_FILE_SIZE) {\n        return "Error: File too large";\n    }\n\n    /* Security: Validate filename */\n    if (!is_valid_filename(filename)) {\n        return "Error: Invalid filename";\n    }\n\n    /* Security: Generate random unique prefix */\n    unsigned char random_bytes[16];\n    if (RAND_bytes(random_bytes, sizeof(random_bytes)) != 1) {\n        return "Error: Failed to generate random data";\n    }\n\n    unsigned char hash[SHA256_DIGEST_LENGTH];\n    SHA256(random_bytes, sizeof(random_bytes), hash);\n\n    char hex_prefix[33];\n    memset(hex_prefix, 0, sizeof(hex_prefix));\n    for (int i = 0; i < 16; i++) {\n        snprintf(hex_prefix + (i * 2), 3, "%02x", hash[i]);\n    }\n\n    /* Security: Construct safe filename */\n    char safe_filename[MAX_PATH_LENGTH];\n    const char* basename = get_basename(filename);\n    int ret = snprintf(safe_filename, sizeof(safe_filename), "%s/%s_%s", UPLOAD_DIR, hex_prefix, basename);\n    if (ret < 0 || ret >= (int)sizeof(safe_filename)) {\n        return "Error: Filename too long";\n    }\n\n    /* Security: Create temp file with O_CREAT | O_EXCL | O_NOFOLLOW */\n    char temp_filename[MAX_PATH_LENGTH];\n    ret = snprintf(temp_filename, sizeof(temp_filename), "%s/upload_XXXXXX.tmp", UPLOAD_DIR);\n    if (ret < 0 || ret >= (int)sizeof(temp_filename)) {\n        return "Error: Temp filename too long";\n    }\n\n    /* Security: Open with restrictive permissions (0600) */\n    int fd = open(temp_filename, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW, 0600);\n    if (fd < 0) {\n        perror("Failed to create temp file");\n        return "Error: Failed to create temp file";\n    }\n\n    /* Security: Write content */\n    ssize_t written = write(fd, content, content_size);\n    if (written < 0 || (size_t)written != content_size) {\n        close(fd);\n        unlink(temp_filename);\n        return "Error: Failed to write file";\n    }\n\n    /* Security: Sync to disk */\n    if (fsync(fd) != 0) {\n        close(fd);\n        unlink(temp_filename);\n        return "Error: Failed to sync file";\n    }\n\n    close(fd);\n\n    /* Security: Atomic rename to final location */\n    if (rename(temp_filename, safe_filename) != 0) {\n        unlink(temp_filename);\n        return "Error: Failed to move file";\n    }\n\n    return "File uploaded successfully";\n}\n\nstatic void run_tests(void) {\n    printf("\\n=== Running Test Cases ===\\n");\n\n    /* Test 1: Valid filename */\n    printf("Test 1: Valid filename 'test.txt' - Expected: Pass\\n");\n    printf("Result: %s\\n", is_valid_filename("test.txt") ? "PASS" : "FAIL");\n\n    /* Test 2: Path traversal attempt */\n    printf("\\nTest 2: Path traversal '../etc/passwd' - Expected: Fail\\n");\n    printf("Result: %s\\n", !is_valid_filename("../etc/passwd") ? "PASS" : "FAIL");\n\n    /* Test 3: Invalid extension */\n    printf("\\nTest 3: Invalid extension 'file.exe' - Expected: Fail\\n");\n    printf("Result: %s\\n", !is_valid_filename("file.exe") ? "PASS" : "FAIL");\n\n    /* Test 4: Special characters */\n    printf("\\nTest 4: Special characters 'file<>.txt' - Expected: Fail\\n");\n    printf("Result: %s\\n", !is_valid_filename("file<>.txt") ? "PASS" : "FAIL");\n\n    /* Test 5: Maximum length */\n    printf("\\nTest 5: Filename exceeds max length - Expected: Fail\\n");\n    char long_name[300];\n    memset(long_name, 'a', 256);
    strcpy(long_name + 256, ".txt");
    printf("Result: %s\\n", !is_valid_filename(long_name) ? "PASS" : "FAIL");
}

int main(void) {
    /* Security: Initialize OpenSSL random number generator */
    if (RAND_status() != 1) {
        fprintf(stderr, "Failed to initialize random number generator\\n");
        return 1;
    }

    /* Security: Create upload directory */
    if (!create_upload_directory()) {
        fprintf(stderr, "Failed to create upload directory\\n");
        return 1;
    }

    printf("Secure File Upload Server Initialized\\n");
    printf("Upload directory: %s\\n", UPLOAD_DIR);
    printf("Max file size: %zu bytes\\n", (size_t)MAX_FILE_SIZE);

    /* Run test cases */
    run_tests();

    return 0;
}
