
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#define UPLOAD_DIR "uploads"
#define MAX_FILE_SIZE (10 * 1024 * 1024)
#define SAFE_FILENAME_LEN 32

static const unsigned char PDF_MAGIC[] = {0x25, 0x50, 0x44, 0x46}; /* %PDF */

/* Generate base64url character from 6-bit value */
static char base64url_char(unsigned char c) {
    if (c < 26) return 'A' + c;
    if (c < 52) return 'a' + (c - 26);
    if (c < 62) return '0' + (c - 52);
    if (c == 62) return '-';
    return '_';
}

/* Generate secure random filename using random data */
static int generate_safe_filename(char *buffer, size_t buffer_size) {
    FILE *urandom = NULL;
    unsigned char random_bytes[16];
    size_t i, j;
    
    if (buffer == NULL || buffer_size < SAFE_FILENAME_LEN) {
        return -1;
    }
    
    /* Initialize buffer */
    memset(buffer, 0, buffer_size);
    
    /* Read random bytes from /dev/urandom or use fallback */
    urandom = fopen("/dev/urandom", "rb");
    if (urandom != NULL) {
        if (fread(random_bytes, 1, sizeof(random_bytes), urandom) != sizeof(random_bytes)) {
            fclose(urandom);
            return -1;
        }
        fclose(urandom);
        urandom = NULL;
    } else {
        /* Fallback: use time and rand (less secure, for portability) */
        srand((unsigned int)time(NULL));
        for (i = 0; i < sizeof(random_bytes); i++) {
            random_bytes[i] = (unsigned char)(rand() % 256);
        }
    }
    
    /* Base64url encode */
    j = 0;
    for (i = 0; i < sizeof(random_bytes) && j < buffer_size - 5; i += 3) {
        unsigned char b1 = random_bytes[i];
        unsigned char b2 = (i + 1 < sizeof(random_bytes)) ? random_bytes[i + 1] : 0;
        unsigned char b3 = (i + 2 < sizeof(random_bytes)) ? random_bytes[i + 2] : 0;
        
        buffer[j++] = base64url_char(b1 >> 2);
        buffer[j++] = base64url_char(((b1 & 0x03) << 4) | (b2 >> 4));
        if (i + 1 < sizeof(random_bytes)) {
            buffer[j++] = base64url_char(((b2 & 0x0f) << 2) | (b3 >> 6));
        }
        if (i + 2 < sizeof(random_bytes)) {
            buffer[j++] = base64url_char(b3 & 0x3f);
        }
    }
    
    /* Add .pdf extension */
    if (j + 5 < buffer_size) {
        strncpy(buffer + j, ".pdf", 5);
        return 0;
    }
    
    return -1;
}

/* Validate filename doesn't contain dangerous characters */\nstatic int validate_filename(const char *filename) {\n    if (filename == NULL || strlen(filename) == 0) {\n        return -1;\n    }\n    \n    /* Check for path traversal */\n    if (strstr(filename, "..") != NULL || \n        strchr(filename, '/') != NULL || \n        strchr(filename, '\\\\') != NULL) {
        return -1;
    }
    
    /* Check extension */
    size_t len = strlen(filename);
    if (len < 4 || strcmp(filename + len - 4, ".pdf") != 0) {
        return -1;
    }
    
    return 0;
}

/* Secure PDF upload with TOCTOU prevention and validation */
char* upload_pdf(const unsigned char *file_content, size_t file_size, 
                 const char *original_filename) {
    char *safe_filename = NULL;
    char target_path[256];
    char temp_path[256];
    int temp_fd = -1;
    struct stat st;
    size_t total_written;
    ssize_t written;
    
    /* Input validation: check file size */
    if (file_content == NULL || file_size == 0 || file_size > MAX_FILE_SIZE) {
        fprintf(stderr, "Invalid file size\\n");
        return NULL;
    }
    
    /* Validate original filename */
    if (validate_filename(original_filename) != 0) {
        fprintf(stderr, "Invalid filename\\n");
        return NULL;
    }
    
    /* Validate PDF magic bytes */
    if (file_size < 4 || memcmp(file_content, PDF_MAGIC, 4) != 0) {
        fprintf(stderr, "File is not a valid PDF\\n");
        return NULL;
    }
    
    /* Create upload directory with restricted permissions if needed */
    if (stat(UPLOAD_DIR, &st) != 0) {
        if (mkdir(UPLOAD_DIR, 0700) != 0) {
            fprintf(stderr, "Failed to create upload directory\\n");
            return NULL;
        }
    }
    
    /* Allocate and generate secure filename */
    safe_filename = (char*)malloc(SAFE_FILENAME_LEN);
    if (safe_filename == NULL) {
        fprintf(stderr, "Memory allocation failed\\n");
        return NULL;
    }
    
    if (generate_safe_filename(safe_filename, SAFE_FILENAME_LEN) != 0) {
        free(safe_filename);
        fprintf(stderr, "Failed to generate safe filename\\n");
        return NULL;
    }
    
    /* Construct paths with bounds checking */
    if (snprintf(target_path, sizeof(target_path), "%s/%s", 
                 UPLOAD_DIR, safe_filename) >= (int)sizeof(target_path)) {
        free(safe_filename);
        fprintf(stderr, "Path too long\\n");
        return NULL;
    }
    
    if (snprintf(temp_path, sizeof(temp_path), "%s/temp_%s", 
                 UPLOAD_DIR, safe_filename) >= (int)sizeof(temp_path)) {
        free(safe_filename);
        fprintf(stderr, "Path too long\\n");
        return NULL;
    }
    
    /* Open temp file with O_CREAT | O_EXCL to prevent TOCTOU, O_NOFOLLOW to prevent symlink attacks */
    temp_fd = open(temp_path, O_CREAT | O_EXCL | O_WRONLY | O_CLOEXEC | O_NOFOLLOW, 0600);
    if (temp_fd < 0) {
        free(safe_filename);
        fprintf(stderr, "Failed to create temporary file: %s\\n", strerror(errno));
        return NULL;
    }
    
    /* Validate opened file with fstat (TOCTOU safe) */
    if (fstat(temp_fd, &st) != 0 || !S_ISREG(st.st_mode)) {
        close(temp_fd);
        unlink(temp_path);
        free(safe_filename);
        fprintf(stderr, "File validation failed\\n");
        return NULL;
    }
    
    /* Write content to temp file */
    total_written = 0;
    while (total_written < file_size) {
        written = write(temp_fd, file_content + total_written, file_size - total_written);
        if (written < 0) {
            close(temp_fd);
            unlink(temp_path);
            free(safe_filename);
            fprintf(stderr, "Failed to write to file: %s\\n", strerror(errno));
            return NULL;
        }
        total_written += (size_t)written;
    }
    
    /* Sync to disk */
    if (fsync(temp_fd) != 0) {
        close(temp_fd);
        unlink(temp_path);
        free(safe_filename);
        fprintf(stderr, "Failed to sync file\\n");
        return NULL;
    }
    
    close(temp_fd);
    temp_fd = -1;
    
    /* Atomic move to final location */
    if (rename(temp_path, target_path) != 0) {
        unlink(temp_path);
        free(safe_filename);
        fprintf(stderr, "Failed to move file to final location: %s\\n", strerror(errno));
        return NULL;
    }
    
    return safe_filename;
}

int main(void) {
    unsigned char *valid_pdf = NULL;
    unsigned char *invalid_pdf = NULL;
    char *result = NULL;
    
    printf("PDF Upload Test Cases:\\n");
    
    /* Test case 1: Valid PDF file */
    valid_pdf = (unsigned char*)malloc(100);
    if (valid_pdf != NULL) {
        memset(valid_pdf, 0, 100);
        valid_pdf[0] = 0x25; valid_pdf[1] = 0x50; 
        valid_pdf[2] = 0x44; valid_pdf[3] = 0x46; /* %PDF */
        result = upload_pdf(valid_pdf, 100, "document.pdf");
        if (result != NULL) {
            printf("Test 1 PASSED: %s\\n", result);
            free(result);
        } else {
            printf("Test 1 FAILED\\n");
        }
        free(valid_pdf);
    }
    
    /* Test case 2: File too large */
    result = upload_pdf((unsigned char*)"", 0, "large.pdf");
    if (result == NULL) {
        printf("Test 2 PASSED: Rejected empty file\\n");
    } else {
        printf("Test 2 FAILED: Should reject empty file\\n");
        free(result);
    }
    
    /* Test case 3: Path traversal attempt */
    valid_pdf = (unsigned char*)malloc(100);
    if (valid_pdf != NULL) {
        memset(valid_pdf, 0, 100);
        valid_pdf[0] = 0x25; valid_pdf[1] = 0x50; 
        valid_pdf[2] = 0x44; valid_pdf[3] = 0x46;
        result = upload_pdf(valid_pdf, 100, "../../../etc/passwd.pdf");
        if (result == NULL) {
            printf("Test 3 PASSED: Rejected path traversal\\n");
        } else {
            printf("Test 3 FAILED: Should reject path traversal\\n");
            free(result);
        }
        free(valid_pdf);
    }
    
    /* Test case 4: Invalid file extension */
    valid_pdf = (unsigned char*)malloc(100);
    if (valid_pdf != NULL) {
        memset(valid_pdf, 0, 100);
        valid_pdf[0] = 0x25; valid_pdf[1] = 0x50; 
        valid_pdf[2] = 0x44; valid_pdf[3] = 0x46;
        result = upload_pdf(valid_pdf, 100, "document.exe");
        if (result == NULL) {
            printf("Test 4 PASSED: Rejected non-PDF extension\\n");
        } else {
            printf("Test 4 FAILED: Should reject non-PDF extension\\n");
            free(result);
        }
        free(valid_pdf);
    }
    
    /* Test case 5: Invalid PDF magic bytes */
    invalid_pdf = (unsigned char*)malloc(100);
    if (invalid_pdf != NULL) {
        memset(invalid_pdf, 0, 100);
        result = upload_pdf(invalid_pdf, 100, "fake.pdf");
        if (result == NULL) {
            printf("Test 5 PASSED: Rejected invalid PDF magic bytes\\n");
        } else {
            printf("Test 5 FAILED: Should reject invalid PDF\\n");
            free(result);
        }
        free(invalid_pdf);
    }
    
    return 0;
}
