#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#define MKDIR(path) _mkdir(path)
#define REALPATH(path, resolved) _fullpath(resolved, path, FILENAME_MAX)
#define PATH_MAX FILENAME_MAX
#else
#include <unistd.h>
#include <limits.h>
#define MKDIR(path) mkdir(path, 0755)
#define REALPATH(path, resolved) realpath(path, resolved)
#endif


// --- Start of Base64 utility ---
// This Base64 implementation is a simplified version based on public domain examples.
static const char b64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char *base64_encode(const unsigned char *data, size_t input_length) {
    size_t output_length = 4 * ((input_length + 2) / 3);
    char *encoded_data = malloc(output_length + 1);
    if (encoded_data == NULL) return NULL;

    for (size_t i = 0, j = 0; i < input_length;) {
        uint32_t octet_a = i < input_length ? data[i++] : 0;
        uint32_t octet_b = i < input_length ? data[i++] : 0;
        uint32_t octet_c = i < input_length ? data[i++] : 0;
        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = b64_table[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = b64_table[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = b64_table[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = b64_table[(triple >> 0 * 6) & 0x3F];
    }

    // Add padding
    size_t mod_table[] = {0, 2, 1};
    for (size_t i = 0; i < mod_table[input_length % 3]; i++) {
        encoded_data[output_length - 1 - i] = '=';
    }

    encoded_data[output_length] = '\0';
    return encoded_data;
}
// --- End of Base64 utility ---

int upload_image(const char* imagePath) {
    if (imagePath == NULL) {
        fprintf(stderr, "Error: Image path is NULL.\n");
        return -1;
    }
    
    char upload_dir_path[PATH_MAX];
    if (REALPATH("uploads", upload_dir_path) == NULL) {
        // If the directory doesn't exist, create it and resolve path again.
        MKDIR("uploads");
        if (REALPATH("uploads", upload_dir_path) == NULL) {
            perror("Error resolving upload directory path");
            return -1;
        }
    }

    char input_realpath[PATH_MAX];
    if (REALPATH(imagePath, input_realpath) == NULL) {
        // Can fail if file does not exist, which is a valid check.
        fprintf(stderr, "Error: File not found or path is invalid: %s\n", imagePath);
        return -1;
    }

    // --- Security Check: Path Traversal ---
    if (strncmp(upload_dir_path, input_realpath, strlen(upload_dir_path)) != 0) {
        fprintf(stderr, "Error: Path traversal attempt detected. Access denied for: %s\n", imagePath);
        return -1;
    }
    
    // --- Sanity Check: Is it a regular file? ---
    struct stat path_stat;
    stat(input_realpath, &path_stat);
    if (!S_ISREG(path_stat.st_mode)) {
        fprintf(stderr, "Error: Path is not a regular file: %s\n", input_realpath);
        return -1;
    }

    FILE *file = fopen(input_realpath, "rb");
    if (!file) {
        fprintf(stderr, "Error: Could not open file: %s\n", input_realpath);
        return -1;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    unsigned char *buffer = malloc(file_size);
    if (!buffer) {
        fprintf(stderr, "Error: Could not allocate memory for file buffer.\n");
        fclose(file);
        return -1;
    }

    if (fread(buffer, 1, file_size, file) != (size_t)file_size) {
        fprintf(stderr, "Error: Could not read file content.\n");
        free(buffer);
        fclose(file);
        return -1;
    }
    fclose(file);

    char *encoded_data = base64_encode(buffer, file_size);
    free(buffer);
    if (!encoded_data) {
        fprintf(stderr, "Error: Failed to encode data.\n");
        return -1;
    }

    // Construct output path
    const char *filename = strrchr(imagePath, '/');
    if (filename == NULL) filename = strrchr(imagePath, '\\');
    filename = (filename == NULL) ? imagePath : filename + 1;

    char output_path[PATH_MAX];
    snprintf(output_path, sizeof(output_path), "%s/%s.b64", "uploads", filename);

    FILE *out_file = fopen(output_path, "w");
    if (!out_file) {
        fprintf(stderr, "Error: Could not create output file: %s\n", output_path);
        free(encoded_data);
        return -1;
    }

    fprintf(out_file, "%s", encoded_data);
    fclose(out_file);
    free(encoded_data);

    printf("Image successfully encoded and saved to: %s\n", output_path);
    return 0;
}

void setup_test_environment() {
    printf("--- Setting up test environment ---\n");
    MKDIR("uploads");
    FILE* f1 = fopen("uploads/test_image.jpg", "w");
    if (f1) {
        fprintf(f1, "dummy image data");
        fclose(f1);
    }
    FILE* f2 = fopen("sensitive.txt", "w");
    if (f2) {
        fprintf(f2, "secret data");
        fclose(f2);
    }
    printf("Test environment created.\n");
    printf("------------------------------------\n");
}


int main() {
    setup_test_environment();

    printf("\n--- Running Test Cases ---\n");
    
    // Test Case 1: Valid image file
    printf("\n[Test 1] Processing a valid image file...\n");
    upload_image("uploads/test_image.jpg");

    // Test Case 2: File does not exist
    printf("\n[Test 2] Processing a non-existent file...\n");
    upload_image("uploads/not_found.jpg");

    // Test Case 3: Path traversal attempt
    printf("\n[Test 3] Processing with a path traversal attempt...\n");
    upload_image("uploads/../sensitive.txt");
    
    // Test Case 4: Accessing a file outside the allowed directory
    printf("\n[Test 4] Processing a file outside the upload directory...\n");
    upload_image("sensitive.txt");

    // Test Case 5: Path is a directory, not a file
    printf("\n[Test 5] Processing a directory path...\n");
    upload_image("uploads");
    
    printf("\n--- Test Cases Finished ---\n");

    return 0;
}