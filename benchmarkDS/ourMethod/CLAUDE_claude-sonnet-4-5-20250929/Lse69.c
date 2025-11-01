
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <limits.h>
#include <errno.h>

#define UPLOAD_DIR "/var/uploads"
#define MAX_FILE_SIZE (5 * 1024 * 1024)
#define MAX_FILENAME 256

/* Base64 encoding characters */
static const char base64_chars[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/**
 * Safe base64 encoding function
 * @param data input data to encode
 * @param len length of input data
 * @param out_len pointer to store output length
 * @return allocated string with base64 encoded data, caller must free
 */
char* base64_encode(const unsigned char* data, size_t len, size_t* out_len) {
    if (data == NULL || len == 0 || out_len == NULL) {
        return NULL;
    }
    
    /* Check for integer overflow */
    if (len > SIZE_MAX / 4 * 3) {
        return NULL;
    }
    
    size_t encoded_len = ((len + 2) / 3) * 4;
    char* encoded = (char*)calloc(encoded_len + 1, 1);
    if (encoded == NULL) {
        return NULL;
    }
    
    size_t j = 0;
    for (size_t i = 0; i < len; i += 3) {
        unsigned char b1 = data[i];
        unsigned char b2 = (i + 1 < len) ? data[i + 1] : 0;
        unsigned char b3 = (i + 2 < len) ? data[i + 2] : 0;
        
        encoded[j++] = base64_chars[b1 >> 2];
        encoded[j++] = base64_chars[((b1 & 0x03) << 4) | (b2 >> 4)];
        encoded[j++] = (i + 1 < len) ? base64_chars[((b2 & 0x0f) << 2) | (b3 >> 6)] : '=';
        encoded[j++] = (i + 2 < len) ? base64_chars[b3 & 0x3f] : '=';
    }
    
    encoded[j] = '\\0';
    *out_len = j;
    return encoded;
}

/**
 * Securely wipe memory
 */
void secure_zero(void* ptr, size_t len) {
    if (ptr == NULL) return;
    volatile unsigned char* p = (volatile unsigned char*)ptr;
    while (len--) {
        *p++ = 0;
    }
}

/**
 * Uploads and encodes an image file
 * @param image_data pointer to raw image bytes
 * @param data_len length of image data
 * @param out_filename buffer to store output filename (must be at least MAX_FILENAME bytes)
 * @return 0 on success, -1 on failure
 */
int upload_image(const unsigned char* image_data, size_t data_len, char* out_filename) {
    int fd = -1;
    char* encoded = NULL;
    size_t encoded_len = 0;
    char temp_path[PATH_MAX];
    char full_path[PATH_MAX];
    int ret = -1;
    
    /* Input validation: check for NULL pointer and size limits */
    if (image_data == NULL || data_len == 0 || out_filename == NULL) {
        fprintf(stderr, "Invalid image data\\n");
        return -1;
    }
    
    if (data_len > MAX_FILE_SIZE) {
        fprintf(stderr, "File size exceeds limit\\n");
        return -1;
    }
    
    /* Create upload directory with restrictive permissions (0700) */
    struct stat st;
    if (stat(UPLOAD_DIR, &st) != 0) {
        if (mkdir(UPLOAD_DIR, 0700) != 0) {
            fprintf(stderr, "Failed to create upload directory\\n");
            return -1;
        }
    }
    
    /* Generate a safe, random filename */
    srand((unsigned int)time(NULL));
    snprintf(out_filename, MAX_FILENAME, "image_%ld_%d.b64", 
             (long)time(NULL), rand() % 100000);
    
    /* Build paths safely using snprintf with size limits */
    if (snprintf(temp_path, sizeof(temp_path), "%s/temp_%ld_%d.tmp", 
                 UPLOAD_DIR, (long)time(NULL), rand() % 100000) >= (int)sizeof(temp_path)) {
        fprintf(stderr, "Path too long\\n");
        goto cleanup;
    }
    
    if (snprintf(full_path, sizeof(full_path), "%s/%s", 
                 UPLOAD_DIR, out_filename) >= (int)sizeof(full_path)) {
        fprintf(stderr, "Path too long\\n");
        goto cleanup;
    }
    
    /* Encode the image data using base64 */
    encoded = base64_encode(image_data, data_len, &encoded_len);
    if (encoded == NULL) {
        fprintf(stderr, "Base64 encoding failed\\n");
        goto cleanup;
    }
    
    /* Open temp file with O_CREAT|O_EXCL|O_WRONLY and restrictive permissions (0600) */
    fd = open(temp_path, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0600);
    if (fd < 0) {
        fprintf(stderr, "Failed to create temp file\\n");
        goto cleanup;
    }
    
    /* Validate the opened file descriptor */
    struct stat temp_st;
    if (fstat(fd, &temp_st) != 0 || !S_ISREG(temp_st.st_mode)) {
        fprintf(stderr, "Invalid file type\\n");
        goto cleanup;
    }
    
    /* Write encoded data to file with bounds checking */
    ssize_t written = write(fd, encoded, encoded_len);
    if (written < 0 || (size_t)written != encoded_len) {
        fprintf(stderr, "Failed to write encoded image\\n");
        goto cleanup;
    }
    
    /* Sync to disk */
    if (fsync(fd) != 0) {
        fprintf(stderr, "Failed to sync file\\n");
        goto cleanup;
    }
    
    close(fd);
    fd = -1;
    
    /* Atomically rename temp file to final destination */
    if (rename(temp_path, full_path) != 0) {
        fprintf(stderr, "Failed to rename file\\n");
        goto cleanup;
    }
    
    ret = 0;
    
cleanup:
    /* Clean up resources */
    if (fd >= 0) {
        close(fd);
        unlink(temp_path);
    }
    if (encoded != NULL) {
        secure_zero(encoded, encoded_len);
        free(encoded);
    }
    
    return ret;
}

int main(void) {
    char filename[MAX_FILENAME];
    
    /* Test case 1: Valid small image */
    const unsigned char test1[] = "Test image data 1";
    int result1 = upload_image(test1, sizeof(test1) - 1, filename);
    printf("Test 1 - Valid small image: %s\\n", (result1 == 0 ? "PASS" : "FAIL"));
    
    /* Test case 2: NULL input */
    int result2 = upload_image(NULL, 100, filename);
    printf("Test 2 - NULL input: %s\\n", (result2 == -1 ? "PASS" : "FAIL"));
    
    /* Test case 3: Zero length */
    const unsigned char test3[] = "Data";
    int result3 = upload_image(test3, 0, filename);
    printf("Test 3 - Zero length: %s\\n", (result3 == -1 ? "PASS" : "FAIL"));
    
    /* Test case 4: Valid binary data */
    unsigned char test4[1024];
    for (size_t i = 0; i < sizeof(test4); i++) {
        test4[i] = (unsigned char)(i % 256);
    }
    int result4 = upload_image(test4, sizeof(test4), filename);
    printf("Test 4 - Binary data: %s\\n", (result4 == 0 ? "PASS" : "FAIL"));
    
    /* Test case 5: Size boundary */
    unsigned char test5[2048];
    memset(test5, 'A', sizeof(test5));
    int result5 = upload_image(test5, sizeof(test5), filename);
    printf("Test 5 - Size boundary: %s\\n", (result5 == 0 ? "PASS" : "FAIL"));
    
    return 0;
}
