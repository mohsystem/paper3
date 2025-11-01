#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libgen.h>
#include <errno.h>
#include <sys/types.h>

const char* BASE_IMAGE_DIR = "test_images_c";
const long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10 MB

char* img(const char* file, long* out_size) {
    if (out_size == NULL) return NULL;
    *out_size = 0;
    if (file == NULL || *file == '\0') {
        fprintf(stderr, "Error: Filename is null or empty.\n");
        return NULL;
    }

    // Rule #1 & #5: Sanitize input by taking only the basename.
    char* file_copy = strdup(file);
    if (!file_copy) { perror("strdup"); return NULL; }
    char* sanitized_filename_ptr = basename(file_copy);
    char* sanitized_filename = strdup(sanitized_filename_ptr);
    free(file_copy);
    if (!sanitized_filename) { perror("strdup"); return NULL; }

    if (strcmp(sanitized_filename, file) != 0 || strstr(sanitized_filename, "..") != NULL) {
        fprintf(stderr, "Error: Potential path traversal in filename '%s'.\n", file);
        free(sanitized_filename);
        return NULL;
    }
    
    // Rule #6: Open first, then validate (TOCTOU avoidance)
    int dir_fd = -1, fd = -1;
    char* buffer = NULL;

    dir_fd = open(BASE_IMAGE_DIR, O_RDONLY | O_DIRECTORY);
    if (dir_fd < 0) { perror("Error opening base directory"); goto cleanup_sanitized_name; }
    
    fd = openat(dir_fd, sanitized_filename, O_RDONLY | O_NOFOLLOW);
    close(dir_fd);
    
    if (fd < 0) { fprintf(stderr, "Error: Could not open file '%s'.\n", sanitized_filename); goto cleanup_sanitized_name; }
    
    struct stat st;
    if (fstat(fd, &st) != 0) { perror("Error getting file status"); goto cleanup_fd; }

    if (!S_ISREG(st.st_mode)) { fprintf(stderr, "Error: '%s' is not a regular file.\n", sanitized_filename); goto cleanup_fd; }

    // Rule #3: Check file size.
    if (st.st_size > MAX_FILE_SIZE) { fprintf(stderr, "Error: File size %ld exceeds limit.\n", (long)st.st_size); goto cleanup_fd; }
    
    if (st.st_size > 0) {
        buffer = (char*)malloc(st.st_size);
        if (!buffer) { perror("malloc"); goto cleanup_fd; }
        ssize_t bytes_read = read(fd, buffer, st.st_size);
        if (bytes_read < 0) { perror("read"); free(buffer); buffer = NULL; goto cleanup_fd; }
        if (bytes_read != st.st_size) { fprintf(stderr, "Error: Incomplete read.\n"); free(buffer); buffer = NULL; goto cleanup_fd; }
        *out_size = bytes_read;
    } else {
        *out_size = 0;
        buffer = malloc(1); 
        if (buffer) buffer[0] = '\0';
    }

cleanup_fd:
    close(fd);
cleanup_sanitized_name:
    free(sanitized_filename);
    return buffer;
}

void setup_test_environment() {
    mkdir(BASE_IMAGE_DIR, 0755);
    char path[256];
    snprintf(path, sizeof(path), "%s/test.txt", BASE_IMAGE_DIR);
    FILE* f1 = fopen(path, "w");
    if(f1) { fputs("valid content", f1); fclose(f1); }
    snprintf(path, sizeof(path), "%s/subdir", BASE_IMAGE_DIR);
    mkdir(path, 0755);
    snprintf(path, sizeof(path), "%s/subdir/nested.txt", BASE_IMAGE_DIR);
    FILE* f2 = fopen(path, "w");
    if(f2) { fputs("nested content", f2); fclose(f2); }
}

void cleanup_test_environment() {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", BASE_IMAGE_DIR);
    if (system(cmd) != 0) {
        fprintf(stderr, "Warning: Failed to cleanup test directory.\n");
    }
}

void run_test_case(int num, const char* desc, const char* input, bool success_expected) {
    printf("\n%d. Testing %s '%s'\n", num, desc, input);
    long size = 0;
    char* content = img(input, &size);
    
    if (success_expected) {
        if (content != NULL) printf("Success. Content: %.*s\n", (int)size, content);
        else printf("Failed.\n");
    } else {
        if (content == NULL) printf("Success. Request blocked as expected.\n");
        else printf("Failed.\n");
    }
    free(content);
}

int main() {
    setup_test_environment();
    printf("--- C Test Cases ---\n");
    
    run_test_case(1, "valid file", "test.txt", true);
    run_test_case(2, "non-existent file", "notfound.txt", false);
    run_test_case(3, "path traversal", "../test.txt", false);
    
    char abs_path_buf[1024] = {0};
    char resolved_path[1024] = {0};
    snprintf(abs_path_buf, sizeof(abs_path_buf) - 1, "%s/%s", BASE_IMAGE_DIR, "test.txt");
    if (realpath(abs_path_buf, resolved_path)) {
        run_test_case(4, "absolute path", resolved_path, false);
    } else {
        perror("realpath for test case 4");
    }
    
    run_test_case(5, "nested path", "subdir/nested.txt", false);
    
    cleanup_test_environment();
    return 0;
}