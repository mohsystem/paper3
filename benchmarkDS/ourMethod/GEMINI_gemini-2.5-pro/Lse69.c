#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>

#define MAX_FILE_SIZE (10 * 1024 * 1024) // 10 MB
#define MAX_PATH_LEN 4096

// A simple Base64 implementation
char *base64_encode(const unsigned char *data, size_t input_length, size_t *output_length) {
    static const char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                        'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                        'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                        'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                        'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                        'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                        'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                        '4', '5', '6', '7', '8', '9', '+', '/'};
    static const int mod_table[] = {0, 2, 1};

    *output_length = 4 * ((input_length + 2) / 3);
    char *encoded_data = malloc(*output_length + 1);
    if (encoded_data == NULL) return NULL;

    for (size_t i = 0, j = 0; i < input_length;) {
        uint32_t octet_a = i < input_length ? data[i++] : 0;
        uint32_t octet_b = i < input_length ? data[i++] : 0;
        uint32_t octet_c = i < input_length ? data[i++] : 0;
        uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;

        encoded_data[j++] = encoding_table[(triple >> 18) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 12) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 6) & 0x3F];
        encoded_data[j++] = encoding_table[triple & 0x3F];
    }

    for (int i = 0; i < mod_table[input_length % 3]; i++) {
        encoded_data[*output_length - 1 - i] = '=';
    }
    encoded_data[*output_length] = '\0';
    return encoded_data;
}

bool upload_image(const char *input_base_dir, const char *input_filename, const char *output_base_dir, const char *output_filename) {
    char real_input_base[PATH_MAX];
    char real_output_base[PATH_MAX];
    char full_input_path[MAX_PATH_LEN];
    char full_output_path[MAX_PATH_LEN];

    // Rule #5: Validate paths to prevent traversal
    if (realpath(input_base_dir, real_input_base) == NULL) {
        perror("Error: Invalid input base directory");
        return false;
    }
    if (realpath(output_base_dir, real_output_base) == NULL) {
        perror("Error: Invalid output base directory");
        return false;
    }
    
    snprintf(full_input_path, sizeof(full_input_path), "%s/%s", real_input_base, input_filename);
    char resolved_input_path[PATH_MAX];
    if (realpath(full_input_path, resolved_input_path) == NULL) {
        // This is expected if file doesn't exist, will be handled by openat
    } else {
        if (strncmp(resolved_input_path, real_input_base, strlen(real_input_base)) != 0) {
            fprintf(stderr, "Error: Input path is outside the allowed directory.\n");
            return false;
        }
    }

    // Prepare variables and cleanup labels for goto
    bool result = false;
    int input_dir_fd = -1, output_dir_fd = -1, input_fd = -1, temp_fd = -1;
    unsigned char *file_buffer = NULL;
    char *encoded_buffer = NULL;
    char temp_filename[256];

    // Rule #6: Open base directories to use openat
    input_dir_fd = open(real_input_base, O_RDONLY | O_DIRECTORY);
    if (input_dir_fd < 0) {
        perror("Error opening input directory");
        goto cleanup;
    }

    output_dir_fd = open(real_output_base, O_RDONLY | O_DIRECTORY);
    if (output_dir_fd < 0) {
        perror("Error opening output directory");
        goto cleanup;
    }

    // Rule #6: Open the file first (TOCTOU-safe), preventing symlinks
    input_fd = openat(input_dir_fd, input_filename, O_RDONLY | O_NOFOLLOW);
    if (input_fd < 0) {
        perror("Error opening input file");
        goto cleanup;
    }

    // Then, validate the handle
    struct stat st;
    if (fstat(input_fd, &st) < 0) {
        perror("Error getting file stats");
        goto cleanup;
    }
    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "Error: Input is not a regular file.\n");
        goto cleanup;
    }

    // Rule #3: Check file size
    if (st.st_size > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: File size exceeds the limit.\n");
        goto cleanup;
    }
    if (st.st_size == 0) {
        fprintf(stderr, "Error: Input file is empty.\n");
        goto cleanup;
    }

    file_buffer = malloc(st.st_size);
    if (!file_buffer) {
        perror("Error allocating memory for file");
        goto cleanup;
    }
    if (read(input_fd, file_buffer, st.st_size) != st.st_size) {
        perror("Error reading file");
        goto cleanup;
    }

    // Base64 encode
    size_t encoded_len;
    encoded_buffer = base64_encode(file_buffer, st.st_size, &encoded_len);
    if (!encoded_buffer) {
        perror("Error during base64 encoding");
        goto cleanup;
    }
    
    // Rule #6: Safe and atomic write
    snprintf(temp_filename, sizeof(temp_filename), "%s.%d.tmp", output_filename, getpid());
    temp_fd = openat(output_dir_fd, temp_filename, O_WRONLY | O_CREAT | O_EXCL, 0600);
    if (temp_fd < 0) {
        perror("Error creating temporary file");
        goto cleanup;
    }

    if (write(temp_fd, encoded_buffer, encoded_len) != (ssize_t)encoded_len) {
        perror("Error writing to temporary file");
        goto cleanup;
    }
    
    if (fsync(temp_fd) != 0) {
        perror("Error fsyncing temporary file");
        goto cleanup;
    }

    // Atomically move the file
    if (renameat(output_dir_fd, temp_filename, output_dir_fd, output_filename) < 0) {
        perror("Error renaming temporary file");
        // Try to remove the temp file on failure
        unlinkat(output_dir_fd, temp_filename, 0);
        goto cleanup;
    }

    // Sync the directory to ensure rename is persisted
    if (fsync(output_dir_fd) != 0) {
        perror("Error fsyncing output directory");
        // This is not a fatal error for the upload itself but indicates a potential issue.
    }
    
    result = true;

cleanup:
    if (input_dir_fd >= 0) close(input_dir_fd);
    if (output_dir_fd >= 0) close(output_dir_fd);
    if (input_fd >= 0) close(input_fd);
    if (temp_fd >= 0) close(temp_fd);
    free(file_buffer);
    free(encoded_buffer);
    return result;
}

void run_tests() {
    const char* input_base = "test_images_c";
    const char* output_base = "test_output_c";

    mkdir(input_base, 0755);
    mkdir(output_base, 0755);

    char valid_image_path[256], large_image_path[256], symlink_image_path[256];
    snprintf(valid_image_path, sizeof(valid_image_path), "%s/valid_image.jpg", input_base);
    snprintf(large_image_path, sizeof(large_image_path), "%s/large_image.jpg", input_base);
    snprintf(symlink_image_path, sizeof(symlink_image_path), "%s/symlink.jpg", input_base);
    
    FILE* f = fopen(valid_image_path, "wb");
    if (f) { fwrite("\x01\x02\x03\x04\x05", 1, 5, f); fclose(f); }
    
    f = fopen(large_image_path, "wb");
    if (f) {
        char zero_buf[1024] = {0};
        for (int i = 0; i < (MAX_FILE_SIZE / 1024) + 2; ++i) {
            fwrite(zero_buf, 1, sizeof(zero_buf), f);
        }
        fclose(f);
    }

    bool symlink_supported = (symlink(valid_image_path, symlink_image_path) == 0);
    if (!symlink_supported) {
        printf("Skipping symlink test: Not supported on this system.\n");
    }

    printf("--- Running C Test Cases ---\n");

    // 1. Valid upload
    printf("1. Testing valid upload... ");
    bool success = upload_image(input_base, "valid_image.jpg", output_base, "encoded.txt");
    printf("%s\n", success ? "SUCCESS" : "FAILED");

    // 2. File too large
    printf("2. Testing file too large... ");
    success = upload_image(input_base, "large_image.jpg", output_base, "encoded_large.txt");
    printf("%s\n", !success ? "SUCCESS (Rejected as expected)" : "FAILED");
    
    // 3. Symbolic link
    if (symlink_supported) {
        printf("3. Testing symbolic link... ");
        success = upload_image(input_base, "symlink.jpg", output_base, "encoded_symlink.txt");
        printf("%s\n", !success ? "SUCCESS (Rejected as expected)" : "FAILED");
    }

    // 4. Path traversal
    printf("4. Testing path traversal... ");
    success = upload_image(input_base, "../test_traversal.jpg", output_base, "encoded_traversal.txt");
    printf("%s\n", !success ? "SUCCESS (Rejected as expected)" : "FAILED");

    // 5. Non-existent file
    printf("5. Testing non-existent file... ");
    success = upload_image(input_base, "not_real.jpg", output_base, "encoded_not_real.txt");
    printf("%s\n", !success ? "SUCCESS (Rejected as expected)" : "FAILED");

    // Cleanup
    remove(valid_image_path);
    remove(large_image_path);
    if(symlink_supported) remove(symlink_image_path);
    char out_file[256];
    snprintf(out_file, sizeof(out_file), "%s/encoded.txt", output_base);
    remove(out_file);
    rmdir(input_base);
    rmdir(output_base);
    printf("--- C Tests Complete ---\n");
}


int main() {
    run_tests();
    return 0;
}