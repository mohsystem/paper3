
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <openssl/sha.h>

#define MAX_FILE_SIZE (10 * 1024 * 1024)
#define BUFFER_SIZE 8192
#define PATH_BUFFER_SIZE 4096

/* Secure zeroization using memset_s or explicit_bzero where available */
static void secure_zero(void *ptr, size_t size) {
    if (ptr == NULL || size == 0) return;
#if defined(__STDC_LIB_EXT1__)
    memset_s(ptr, size, 0, size);
#elif defined(__OpenBSD__) || defined(__FreeBSD__)
    explicit_bzero(ptr, size);
#else
    /* Fallback: volatile pointer to prevent optimization */
    volatile unsigned char *p = (volatile unsigned char *)ptr;
    while (size--) {
        *p++ = 0;
    }
#endif
}

/* Convert bytes to hex string */
static void bytes_to_hex(const unsigned char *data, size_t len, char *output) {
    const char hex_chars[] = "0123456789abcdef";
    size_t i;
    for (i = 0; i < len; i++) {
        output[i * 2] = hex_chars[(data[i] >> 4) & 0x0F];
        output[i * 2 + 1] = hex_chars[data[i] & 0x0F];
    }
    output[len * 2] = '\\0';
}

typedef struct {
    ssize_t bytes_read;
    char checksum[SHA256_DIGEST_LENGTH * 2 + 1];
    int is_empty;
} FileResult;

int process_file(const char *base_dir, const char *input_rel_path, 
                 const char *output_rel_path, FileResult *result) {
    int fd = -1;
    int temp_fd = -1;
    unsigned char *buffer = NULL;
    char resolved_base[PATH_MAX] = {0};
    char resolved_input[PATH_MAX] = {0};
    char input_full[PATH_BUFFER_SIZE] = {0};
    char output_full[PATH_BUFFER_SIZE] = {0};
    char temp_path[PATH_BUFFER_SIZE] = {0};
    struct stat st;
    ssize_t bytes_read = 0;
    ssize_t total_read = 0;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    int i;
    int is_empty = 1;
    char checksum_hex[SHA256_DIGEST_LENGTH * 2 + 1];
    FILE *temp_file = NULL;
    
    if (result == NULL || base_dir == NULL || input_rel_path == NULL || output_rel_path == NULL) {
        return -1;
    }
    
    /* Initialize result */
    memset(result, 0, sizeof(FileResult));
    result->bytes_read = -1;
    
    /* Resolve base directory */
    if (realpath(base_dir, resolved_base) == NULL) {
        fprintf(stderr, "Error: Invalid base directory\\n");
        return -1;
    }
    
    /* Construct input path */
    if (snprintf(input_full, sizeof(input_full), "%s/%s", resolved_base, input_rel_path) >= (int)sizeof(input_full)) {
        fprintf(stderr, "Error: Input path too long\\n");
        return -1;
    }
    
    /* Resolve input path */
    if (realpath(input_full, resolved_input) == NULL) {
        fprintf(stderr, "Error: Cannot resolve input path\\n");
        return -1;
    }
    
    /* Ensure input path is within base directory */
    if (strncmp(resolved_input, resolved_base, strlen(resolved_base)) != 0) {
        fprintf(stderr, "Error: Path traversal attempt detected\\n");
        return -1;
    }
    
    /* Open file with O_NOFOLLOW to prevent symlink following */
    fd = open(resolved_input, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (fd == -1) {
        fprintf(stderr, "Error: Cannot open input file\\n");
        return -1;
    }
    
    /* Validate file descriptor */
    if (fstat(fd, &st) == -1) {
        fprintf(stderr, "Error: Cannot stat input file\\n");
        close(fd);
        return -1;
    }
    
    /* Ensure it's a regular file */
    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "Error: Input must be a regular file\\n");
        close(fd);
        return -1;
    }
    
    /* Check file size */
    if (st.st_size > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: File size exceeds maximum allowed\\n");
        close(fd);
        return -1;
    }
    
    /* Allocate buffer */
    buffer = (unsigned char *)malloc(st.st_size);
    if (buffer == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        close(fd);
        return -1;
    }
    
    /* Read file contents with bounds checking */
    while (total_read < st.st_size) {
        bytes_read = read(fd, buffer + total_read, st.st_size - total_read);
        if (bytes_read == -1) {
            fprintf(stderr, "Error: Read failed\\n");
            secure_zero(buffer, st.st_size);
            free(buffer);
            close(fd);
            return -1;
        }
        if (bytes_read == 0) break;
        total_read += bytes_read;
    }
    
    close(fd);
    fd = -1;
    
    /* Calculate SHA-256 checksum */
    SHA256(buffer, total_read, hash);
    
    /* Check if checksum is all zeros */
    for (i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        if (hash[i] != 0) {
            is_empty = 0;
            break;
        }
    }
    
    bytes_to_hex(hash, SHA256_DIGEST_LENGTH, checksum_hex);
    
    /* Construct output path */
    if (snprintf(output_full, sizeof(output_full), "%s/%s", resolved_base, output_rel_path) >= (int)sizeof(output_full)) {
        fprintf(stderr, "Error: Output path too long\\n");
        secure_zero(buffer, st.st_size);
        free(buffer);
        return -1;
    }
    
    /* Create temp file */
    if (snprintf(temp_path, sizeof(temp_path), "%s/.tmpXXXXXX", resolved_base) >= (int)sizeof(temp_path)) {
        fprintf(stderr, "Error: Temp path too long\\n");
        secure_zero(buffer, st.st_size);
        free(buffer);
        return -1;
    }
    
    temp_fd = mkstemp(temp_path);
    if (temp_fd == -1) {
        fprintf(stderr, "Error: Cannot create temp file\\n");
        secure_zero(buffer, st.st_size);
        free(buffer);
        return -1;
    }
    
    /* Write to temp file */
    temp_file = fdopen(temp_fd, "w");
    if (temp_file == NULL) {
        fprintf(stderr, "Error: Cannot open temp file for writing\\n");
        close(temp_fd);
        unlink(temp_path);
        secure_zero(buffer, st.st_size);
        free(buffer);
        return -1;
    }
    
    fprintf(temp_file, "Bytes Read: %zd\\n", total_read);
    fprintf(temp_file, "Checksum: %s\\n", checksum_hex);
    fprintf(temp_file, "Buffer Empty: %s\\n", is_empty ? "true" : "false");
    fflush(temp_file);
    fsync(fileno(temp_file));
    fclose(temp_file);
    
    /* Atomic rename */
    if (rename(temp_path, output_full) == -1) {
        fprintf(stderr, "Error: Cannot rename output file\\n");
        unlink(temp_path);
        secure_zero(buffer, st.st_size);
        free(buffer);
        return -1;
    }
    
    /* Set result */
    result->bytes_read = total_read;
    strncpy(result->checksum, checksum_hex, sizeof(result->checksum) - 1);
    result->is_empty = is_empty;
    
    /* Clear sensitive data */
    secure_zero(buffer, st.st_size);
    free(buffer);
    
    return 0;
}

int main(void) {
    const char *base_dir = "/tmp";
    FileResult result;
    FILE *f = NULL;
    unsigned char data[] = {1, 2, 3, 4, 5};
    int i;
    
    /* Test case 1: Normal file with content */
    f = fopen("/tmp/test1.txt", "w");
    if (f != NULL) {
        fprintf(f, "Hello World");
        fclose(f);
        if (process_file(base_dir, "test1.txt", "output1.txt", &result) == 0) {
            printf("Test 1 - Bytes: %zd, Empty: %d\\n", result.bytes_read, result.is_empty);
        }
    }
    
    /* Test case 2: Empty file */
    f = fopen("/tmp/test2.txt", "w");
    if (f != NULL) {
        fclose(f);
        if (process_file(base_dir, "test2.txt", "output2.txt", &result) == 0) {
            printf("Test 2 - Bytes: %zd, Empty: %d\\n", result.bytes_read, result.is_empty);
        }
    }
    
    /* Test case 3: File with binary data */
    f = fopen("/tmp/test3.bin", "wb");
    if (f != NULL) {
        fwrite(data, 1, sizeof(data), f);
        fclose(f);
        if (process_file(base_dir, "test3.bin", "output3.txt", &result) == 0) {
            printf("Test 3 - Bytes: %zd, Empty: %d\\n", result.bytes_read, result.is_empty);
        }
    }
    
    /* Test case 4: Larger file */
    f = fopen("/tmp/test4.txt", "wb");
    if (f != NULL) {
        for (i = 0; i < 1024; i++) {
            unsigned char byte = i % 256;
            fwrite(&byte, 1, 1, f);
        }
        fclose(f);
        if (process_file(base_dir, "test4.txt", "output4.txt", &result) == 0) {
            printf("Test 4 - Bytes: %zd, Empty: %d\\n", result.bytes_read, result.is_empty);
        }
    }
    
    /* Test case 5: File with special characters */
    f = fopen("/tmp/test5.txt", "w");
    if (f != NULL) {
        fprintf(f, "Special: \\n\\t\\r");
        fclose(f);
        if (process_file(base_dir, "test5.txt", "output5.txt", &result) == 0) {
            printf("Test 5 - Bytes: %zd, Empty: %d\\n", result.bytes_read, result.is_empty);
        }
    }
    
    return 0;
}
