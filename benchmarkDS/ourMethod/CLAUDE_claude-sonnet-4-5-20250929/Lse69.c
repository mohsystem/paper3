
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <limits.h>

/* Secure base64 encoding table */
static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/* Maximum allowed file size: 10MB to prevent resource exhaustion */
#define MAX_FILE_SIZE (10 * 1024 * 1024)
/* Maximum output path length */
#define MAX_PATH_LEN 256
/* Secure output directory - must exist and have restrictive permissions */
#define SECURE_OUTPUT_DIR "/tmp/secure_uploads"

/**
 * Securely clear memory before free to prevent information leakage
 * Uses explicit_bzero if available, otherwise volatile pointer to prevent optimization
 */
static void secure_zero(void *ptr, size_t len) {
    if (ptr == NULL || len == 0) return;
    #ifdef explicit_bzero
        explicit_bzero(ptr, len);
    #else
        volatile unsigned char *p = ptr;
        while (len--) *p++ = 0;
    #endif
}

/**
 * Validate filename to prevent path traversal attacks
 * Only allows alphanumeric, underscore, hyphen, and single dot
 * Rejects: .., /, \\, null bytes, and other special characters
 */
static int validate_filename(const char *filename) {
    if (filename == NULL || filename[0] == '\0') {
        return 0; /* Reject null or empty */
    }
    
    size_t len = strnlen(filename, MAX_PATH_LEN);
    if (len == 0 || len >= MAX_PATH_LEN) {
        return 0; /* Reject too long or empty */
    }
    
    /* Reject path traversal attempts */
    if (strstr(filename, "..") != NULL || 
        strchr(filename, '/') != NULL || 
        strchr(filename, '\\\\') != NULL) {\n        return 0;\n    }\n    \n    /* Only allow safe characters: alphanumeric, underscore, hyphen, dot */\n    for (size_t i = 0; i < len; i++) {\n        char c = filename[i];\n        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || \n              (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.')) {\n            return 0;\n        }\n    }\n    \n    return 1;\n}\n\n/**\n * Encode binary data to base64\n * Returns allocated string on success, NULL on failure\n * Caller must free the returned string and zero it before free\n */\nstatic char* base64_encode(const unsigned char *data, size_t input_len, size_t *output_len) {\n    if (data == NULL || input_len == 0 || output_len == NULL) {\n        return NULL;\n    }\n    \n    /* Check for integer overflow in output size calculation */\n    if (input_len > (SIZE_MAX / 4) * 3 - 3) {\n        return NULL; /* Input too large */\n    }\n    \n    size_t encoded_len = 4 * ((input_len + 2) / 3);\n    \n    /* Allocate with null terminator */\n    char *encoded = calloc(encoded_len + 1, 1);\n    if (encoded == NULL) {\n        return NULL;\n    }\n    \n    size_t i, j;\n    for (i = 0, j = 0; i < input_len;) {\n        uint32_t octet_a = i < input_len ? data[i++] : 0;\n        uint32_t octet_b = i < input_len ? data[i++] : 0;\n        uint32_t octet_c = i < input_len ? data[i++] : 0;\n        \n        uint32_t triple = (octet_a << 16) | (octet_b << 8) | octet_c;\n        \n        encoded[j++] = base64_chars[(triple >> 18) & 0x3F];\n        encoded[j++] = base64_chars[(triple >> 12) & 0x3F];\n        encoded[j++] = base64_chars[(triple >> 6) & 0x3F];\n        encoded[j++] = base64_chars[triple & 0x3F];\n    }\n    \n    /* Add padding */\n    size_t mod = input_len % 3;\n    if (mod == 1) {\n        encoded[encoded_len - 1] = '=';\n        encoded[encoded_len - 2] = '=';\n    } else if (mod == 2) {\n        encoded[encoded_len - 1] = '=';\n    }\n    \n    encoded[encoded_len] = '\0';
    *output_len = encoded_len;
    return encoded;
}

/**
 * Safely read file into buffer with size validation
 * Returns allocated buffer on success, NULL on failure
 * Caller must free and zero the buffer
 */
static unsigned char* read_file_safe(const char *filepath, size_t *file_size) {
    if (filepath == NULL || file_size == NULL) {
        return NULL;
    }
    
    *file_size = 0;
    
    /* Open file with O_NOFOLLOW to prevent symlink attacks */
    int fd = open(filepath, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (fd < 0) {
        fprintf(stderr, "Error: Failed to open file\\n");
        return NULL;
    }
    
    /* Validate file using fstat (not lstat) to avoid TOCTOU */
    struct stat st;
    if (fstat(fd, &st) != 0) {
        fprintf(stderr, "Error: Failed to stat file\\n");
        close(fd);
        return NULL;
    }
    
    /* Validate file type - must be regular file */
    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "Error: Not a regular file\\n");
        close(fd);
        return NULL;
    }
    
    /* Validate file size to prevent resource exhaustion */
    if (st.st_size <= 0 || st.st_size > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: File size invalid or exceeds limit\\n");
        close(fd);
        return NULL;
    }
    
    size_t size = (size_t)st.st_size;
    
    /* Allocate buffer with extra byte for safety */
    unsigned char *buffer = calloc(size + 1, 1);
    if (buffer == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        close(fd);
        return NULL;
    }
    
    /* Read file with bounds checking */
    size_t total_read = 0;
    while (total_read < size) {
        ssize_t bytes_read = read(fd, buffer + total_read, size - total_read);
        if (bytes_read < 0) {
            if (errno == EINTR) continue; /* Retry on interrupt */
            fprintf(stderr, "Error: Read failed\\n");
            secure_zero(buffer, size + 1);
            free(buffer);
            close(fd);
            return NULL;
        }
        if (bytes_read == 0) break; /* EOF */
        total_read += bytes_read;
    }
    
    close(fd);
    
    if (total_read != size) {
        fprintf(stderr, "Error: Incomplete read\\n");
        secure_zero(buffer, size + 1);
        free(buffer);
        return NULL;
    }
    
    *file_size = size;
    return buffer;
}

/**
 * Write data to file atomically and securely
 * Uses temporary file with atomic rename
 */
static int write_file_safe(const char *output_path, const char *data, size_t data_len) {
    if (output_path == NULL || data == NULL || data_len == 0) {
        return -1;
    }
    
    char temp_path[MAX_PATH_LEN];
    int ret = snprintf(temp_path, sizeof(temp_path), "%s.tmp.XXXXXX", output_path);
    if (ret < 0 || (size_t)ret >= sizeof(temp_path)) {
        fprintf(stderr, "Error: Path too long\\n");
        return -1;
    }
    
    /* Create temporary file with restrictive permissions (0600) */
    int fd = mkstemp(temp_path);
    if (fd < 0) {
        fprintf(stderr, "Error: Failed to create temp file\\n");
        return -1;
    }
    
    /* Ensure restrictive permissions */
    if (fchmod(fd, 0600) != 0) {
        fprintf(stderr, "Error: Failed to set permissions\\n");
        close(fd);
        unlink(temp_path);
        return -1;
    }
    
    /* Write data with bounds checking */
    size_t total_written = 0;
    while (total_written < data_len) {
        ssize_t written = write(fd, data + total_written, data_len - total_written);
        if (written < 0) {
            if (errno == EINTR) continue; /* Retry on interrupt */
            fprintf(stderr, "Error: Write failed\\n");
            close(fd);
            unlink(temp_path);
            return -1;
        }
        total_written += written;
    }
    
    /* Ensure data is written to disk */
    if (fsync(fd) != 0) {
        fprintf(stderr, "Error: fsync failed\\n");
        close(fd);
        unlink(temp_path);
        return -1;
    }
    
    close(fd);
    
    /* Atomic rename to final destination */
    if (rename(temp_path, output_path) != 0) {
        fprintf(stderr, "Error: Rename failed\\n");
        unlink(temp_path);
        return -1;
    }
    
    return 0;
}

/**
 * Main function: upload_image
 * Reads an image file, encodes it in base64, and writes to output file
 * Returns 0 on success, -1 on failure
 */
int upload_image(const char *input_filepath, const char *output_filename) {
    /* Validate inputs are not NULL */
    if (input_filepath == NULL || output_filename == NULL) {
        fprintf(stderr, "Error: Invalid parameters\\n");
        return -1;
    }
    
    /* Validate output filename to prevent path traversal */
    if (!validate_filename(output_filename)) {
        fprintf(stderr, "Error: Invalid output filename\\n");
        return -1;
    }
    
    /* Construct full output path with secure directory */
    char output_path[MAX_PATH_LEN];
    int ret = snprintf(output_path, sizeof(output_path), "%s/%s", SECURE_OUTPUT_DIR, output_filename);
    if (ret < 0 || (size_t)ret >= sizeof(output_path)) {
        fprintf(stderr, "Error: Output path too long\\n");
        return -1;
    }
    
    /* Read input file securely */
    size_t file_size = 0;
    unsigned char *file_data = read_file_safe(input_filepath, &file_size);
    if (file_data == NULL) {
        return -1;
    }
    
    /* Encode to base64 */
    size_t encoded_len = 0;
    char *encoded_data = base64_encode(file_data, file_size, &encoded_len);
    
    /* Clear and free original file data immediately */
    secure_zero(file_data, file_size);
    free(file_data);
    file_data = NULL;
    
    if (encoded_data == NULL) {
        fprintf(stderr, "Error: Base64 encoding failed\\n");
        return -1;
    }
    
    /* Write encoded data to output file */
    int write_result = write_file_safe(output_path, encoded_data, encoded_len);
    
    /* Clear and free encoded data */
    secure_zero(encoded_data, encoded_len);
    free(encoded_data);
    encoded_data = NULL;
    
    if (write_result != 0) {
        return -1;
    }
    
    return 0;
}

/* Test cases */
int main(void) {
    /* Test 1: Valid small file */
    FILE *test1 = fopen("/tmp/test1.bin", "wb");
    if (test1) {
        const unsigned char data1[] = {0x48, 0x65, 0x6C, 0x6C, 0x6F}; /* "Hello" */
        fwrite(data1, 1, sizeof(data1), test1);
        fclose(test1);
        mkdir(SECURE_OUTPUT_DIR, 0700);
        int result1 = upload_image("/tmp/test1.bin", "output1.b64");
        printf("Test 1 (Valid file): %s\\n", result1 == 0 ? "PASS" : "FAIL");
        unlink("/tmp/test1.bin");
    }
    
    /* Test 2: Invalid filename with path traversal */
    int result2 = upload_image("/tmp/test1.bin", "../etc/passwd");
    printf("Test 2 (Path traversal): %s\\n", result2 == -1 ? "PASS" : "FAIL");
    
    /* Test 3: NULL input */
    int result3 = upload_image(NULL, "output3.b64");
    printf("Test 3 (NULL input): %s\\n", result3 == -1 ? "PASS" : "FAIL");
    
    /* Test 4: Invalid filename with special chars */
    int result4 = upload_image("/tmp/test1.bin", "output;rm -rf /.b64");
    printf("Test 4 (Special chars): %s\\n", result4 == -1 ? "PASS" : "FAIL");
    
    /* Test 5: Non-existent input file */
    int result5 = upload_image("/tmp/nonexistent.bin", "output5.b64");
    printf("Test 5 (Non-existent file): %s\\n", result5 == -1 ? "PASS" : "FAIL");
    
    return 0;
}
