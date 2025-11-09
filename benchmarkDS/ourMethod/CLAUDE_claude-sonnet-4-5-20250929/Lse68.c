
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define MAX_IMAGE_SIZE (10 * 1024 * 1024)  /* 10MB limit to prevent excessive memory usage */
#define MAX_FILENAME_LEN 255
#define BASE64_ENCODE_OUT_SIZE(s) (((s) + 2) / 3 * 4)

/* Base64 encoding table */
static const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/* Secure base64 encode function with bounds checking */
static char* base64_encode(const unsigned char *data, size_t input_length, size_t *output_length) {
    if (!data || input_length == 0 || !output_length) {
        return NULL;
    }
    
    /* Check for integer overflow in output size calculation - Rules#8, Rules#35 */
    if (input_length > SIZE_MAX / 4 * 3) {
        return NULL;
    }
    
    *output_length = BASE64_ENCODE_OUT_SIZE(input_length);
    
    /* Check allocation size - Rules#8 */
    if (*output_length >= SIZE_MAX - 1) {
        return NULL;
    }
    
    /* Allocate output buffer - Rules#8 */
    char *encoded_data = (char*)calloc(*output_length + 1, 1);
    if (!encoded_data) {
        return NULL;
    }
    
    size_t i, j;
    for (i = 0, j = 0; i < input_length;) {
        uint32_t octet_a = i < input_length ? data[i++] : 0;
        uint32_t octet_b = i < input_length ? data[i++] : 0;
        uint32_t octet_c = i < input_length ? data[i++] : 0;
        uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;
        
        /* Bounds check before writing - Rules#34 */
        if (j + 3 >= *output_length) {
            break;
        }
        
        encoded_data[j++] = base64_table[(triple >> 18) & 0x3F];
        encoded_data[j++] = base64_table[(triple >> 12) & 0x3F];
        encoded_data[j++] = base64_table[(triple >> 6) & 0x3F];
        encoded_data[j++] = base64_table[triple & 0x3F];
    }
    
    /* Add padding */
    size_t mod = input_length % 3;
    if (mod == 1) {
        if (j >= 2) encoded_data[j - 1] = '=';
    } else if (mod == 2) {
        /* No padding needed for mod 2 in standard base64 */
    }
    
    encoded_data[*output_length] = '\0'; /* Null terminate - Rules#36 */
    return encoded_data;
}

/* Validate filename to prevent path traversal - Rules#2 */
static int validate_filename(const char *filename) {
    if (!filename) return 0;
    
    size_t len = strnlen(filename, MAX_FILENAME_LEN + 1);
    if (len == 0 || len > MAX_FILENAME_LEN) {
        return 0; /* Filename too long or empty - Rules#24 */
    }
    
    /* Check for path traversal patterns - Rules#2, Rules#26 */
    if (strstr(filename, "..") || strchr(filename, '/') || strchr(filename, '\\\\')) {\n        return 0;\n    }\n    \n    /* Allow only alphanumeric, underscore, hyphen, and dot - Rules#25 */\n    for (size_t i = 0; i < len; i++) {\n        char c = filename[i];\n        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || \n              (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.')) {\n            return 0;\n        }\n    }\n    \n    return 1;\n}\n\n/* Safe file reading with TOCTOU prevention - Rules#7, Rules#45-52 */\nstatic unsigned char* read_file_safely(int dirfd, const char *filename, size_t *file_size) {\n    if (!filename || !file_size) {\n        return NULL;\n    }\n    \n    *file_size = 0;\n    \n    /* Open file with O_NOFOLLOW to prevent symlink attacks - Rules#46, Rules#47, Rules#49 */\n    int fd = openat(dirfd, filename, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);\n    if (fd < 0) {\n        return NULL;\n    }\n    \n    /* Validate after opening using fstat - Rules#48 */\n    struct stat st;\n    if (fstat(fd, &st) != 0) {\n        close(fd);\n        return NULL;\n    }\n    \n    /* Ensure it's a regular file - Rules#48 */
    if (!S_ISREG(st.st_mode)) {
        close(fd);
        return NULL;
    }
    
    /* Validate size limits - Rules#35, Rules#38 */
    if (st.st_size <= 0 || st.st_size > MAX_IMAGE_SIZE) {
        close(fd);
        return NULL;
    }
    
    *file_size = (size_t)st.st_size;
    
    /* Allocate buffer with size check - Rules#8 */
    unsigned char *buffer = (unsigned char*)calloc(*file_size, 1);
    if (!buffer) {
        close(fd);
        *file_size = 0;
        return NULL;
    }
    
    /* Read file in chunks with bounds checking - Rules#34, Rules#41 */
    size_t total_read = 0;
    while (total_read < *file_size) {
        size_t remaining = *file_size - total_read;
        ssize_t bytes_read = read(fd, buffer + total_read, remaining);
        
        if (bytes_read < 0) {
            if (errno == EINTR) continue;
            free(buffer);
            close(fd);
            *file_size = 0;
            return NULL;
        }
        
        if (bytes_read == 0) break; /* EOF */
        total_read += (size_t)bytes_read;
    }
    
    close(fd);
    
    if (total_read != *file_size) {
        free(buffer);
        *file_size = 0;
        return NULL;
    }
    
    return buffer;
}

/* Mock database insert function - in production use parameterized queries - Rules#32 */
static int db_insert_image(const char *name, const char *base64_data) {
    if (!name || !base64_data) {
        return -1;
    }
    
    /* This is a placeholder - in production:
     * - Use parameterized queries/prepared statements - Rules#32
     * - Never concatenate user input into queries - Rules#32
     * - Use proper database library APIs
     */
    printf("DB INSERT: name='%s', data_length=%zu\\n", name, strlen(base64_data));
    return 0;
}

/* Main upload function with comprehensive security checks */
int upload_image(int dirfd, const char *filename) {
    /* Input validation - Rules#24, Rules#28 */
    if (!filename) {
        fprintf(stderr, "Invalid input\\n"); /* Generic error - Rules#11 */
        return -1;
    }
    
    /* Validate filename for security - Rules#2, Rules#24 */
    if (!validate_filename(filename)) {
        fprintf(stderr, "Invalid filename\\n"); /* Generic error - Rules#11 */
        return -1;
    }
    
    /* Read file safely with TOCTOU prevention - Rules#7, Rules#45-52 */
    size_t file_size = 0;
    unsigned char *image_data = read_file_safely(dirfd, filename, &file_size);
    if (!image_data) {
        fprintf(stderr, "Failed to read file\\n"); /* Generic error - Rules#11 */
        return -1;
    }
    
    /* Encode to base64 with overflow protection - Rules#35 */
    size_t encoded_length = 0;
    char *base64_data = base64_encode(image_data, file_size, &encoded_length);
    
    /* Clear sensitive image data from memory - Rules#9, Rules#40 */
    memset(image_data, 0, file_size);
    free(image_data);
    image_data = NULL;
    
    if (!base64_data) {
        fprintf(stderr, "Failed to encode image\\n"); /* Generic error - Rules#11 */
        return -1;
    }
    
    /* Insert into database using safe API - Rules#32 */
    int result = db_insert_image(filename, base64_data);
    
    /* Clear base64 data from memory - Rules#9, Rules#40 */
    memset(base64_data, 0, encoded_length);
    free(base64_data);
    base64_data = NULL;
    
    if (result != 0) {
        fprintf(stderr, "Database error\\n"); /* Generic error - Rules#11 */
        return -1;
    }
    
    return 0;
}

/* Test driver with 5 test cases */
int main(void) {
    int dirfd;
    
    /* Open trusted directory for file operations - Rules#46 */
    dirfd = open(".", O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (dirfd < 0) {
        perror("Failed to open directory");
        return 1;
    }
    
    printf("=== Test Case 1: Valid image file ===\\n");
    /* Create a test file */
    FILE *f1 = fopen("test_image1.jpg", "wb");
    if (f1) {
        unsigned char test_data[] = {0xFF, 0xD8, 0xFF, 0xE0}; /* JPEG header */
        fwrite(test_data, 1, sizeof(test_data), f1);
        fclose(f1);
        upload_image(dirfd, "test_image1.jpg");
        unlink("test_image1.jpg");
    }
    
    printf("\\n=== Test Case 2: NULL filename ===\\n");
    upload_image(dirfd, NULL);
    
    printf("\\n=== Test Case 3: Path traversal attempt ===\\n");
    upload_image(dirfd, "../etc/passwd");
    
    printf("\\n=== Test Case 4: Invalid characters in filename ===\\n");
    upload_image(dirfd, "test@image#.jpg");
    
    printf("\\n=== Test Case 5: Filename too long ===\\n");
    char long_name[300];
    memset(long_name, 'a', sizeof(long_name) - 5);
    memcpy(long_name + sizeof(long_name) - 5, ".jpg", 5);
    upload_image(dirfd, long_name);
    
    close(dirfd);
    return 0;
}
