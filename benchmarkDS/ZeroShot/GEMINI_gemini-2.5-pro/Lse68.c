#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <ctype.h>

// Define a reasonable max file size to prevent DoS attacks (e.g., 5MB)
#define MAX_FILE_SIZE (5 * 1024 * 1024)

// Helper function to encode binary data to Base64
// The caller is responsible for freeing the returned string.
char* base64_encode(const unsigned char *data, size_t input_length) {
    static const char encoding_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t output_length = 4 * ((input_length + 2) / 3);
    char *encoded_data = malloc(output_length + 1);
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

    size_t mod_table[] = {0, 2, 1};
    for (size_t i = 0; i < mod_table[input_length % 3]; i++)
        encoded_data[output_length - 1 - i] = '=';

    encoded_data[output_length] = '\0';
    return encoded_data;
}

// Security: Sanitize filename by extracting the basename and checking characters
// Returns a new allocated string that must be freed by the caller, or NULL on failure.
char* sanitize_filename(const char* filepath) {
    // Find the last path separator
    const char* last_slash = strrchr(filepath, '/');
    const char* last_backslash = strrchr(filepath, '\\');
    const char* basename = filepath;
    if (last_slash && last_backslash) {
        basename = (last_slash > last_backslash ? last_slash : last_backslash) + 1;
    } else if (last_slash) {
        basename = last_slash + 1;
    } else if (last_backslash) {
        basename = last_backslash + 1;
    }
    
    // Check for invalid characters in the basename
    for (const char* p = basename; *p; ++p) {
        if (!isalnum((unsigned char)*p) && *p != '.' && *p != '_' && *p != '-') {
            fprintf(stderr, "Error: Filename contains invalid characters.\n");
            return NULL;
        }
    }

    char* sanitized = strdup(basename);
    if (!sanitized) {
        fprintf(stderr, "Error: Memory allocation failed for filename.\n");
    }
    return sanitized;
}

/**
 * Securely handles image upload: validates, reads, base64 encodes, and simulates a secure DB insert.
 * @param file_path The path to the image file provided by the user.
 * @return true if the operation was successful, false otherwise.
 */
bool upload_image(const char* file_path) {
    printf("--- Processing: %s ---\n", file_path);
    bool success = false;
    char* sanitized_filename = NULL;
    unsigned char* file_buffer = NULL;
    char* base64_string = NULL;

    // 1. Input Validation: File Existence
    FILE* file = fopen(file_path, "rb");
    if (!file) {
        fprintf(stderr, "Error: File does not exist or cannot be opened.\n");
        goto cleanup;
    }

    // 2. Security: Sanitize Filename
    sanitized_filename = sanitize_filename(file_path);
    if (!sanitized_filename) {
        goto cleanup;
    }
    
    // 3. Security: Check File Size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size < 0) {
        fprintf(stderr, "Error: Could not determine file size.\n");
        goto cleanup;
    }
    if (file_size > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: File size (%ld bytes) exceeds the limit of %ld bytes.\n", file_size, (long)MAX_FILE_SIZE);
        goto cleanup;
    }
    if (file_size == 0) {
        fprintf(stderr, "Error: File is empty.\n");
        goto cleanup;
    }

    // 4. Read file and convert to Base64
    file_buffer = malloc(file_size);
    if (!file_buffer) {
        fprintf(stderr, "Error: Failed to allocate memory for file buffer.\n");
        goto cleanup;
    }
    if (fread(file_buffer, 1, file_size, file) != (size_t)file_size) {
        fprintf(stderr, "Error: Failed to read file content.\n");
        goto cleanup;
    }

    base64_string = base64_encode(file_buffer, file_size);
    if (!base64_string) {
        fprintf(stderr, "Error: Failed to encode file to Base64.\n");
        goto cleanup;
    }

    // 5. Security: Simulate Secure Database Insertion
    printf("Simulating secure database insert...\n");
    printf("  Query Template: INSERT INTO images (name, data) VALUES (?, ?);\n");
    printf("  Binding Param 1 (name): %s\n", sanitized_filename);
    // Truncate for display
    size_t len = strlen(base64_string);
    printf("  Binding Param 2 (data): ");
    for(int i = 0; i < 40 && i < len; ++i) {
        putchar(base64_string[i]);
    }
    if (len > 40) {
        printf("...");
    }
    printf("\n");
    printf("  Execution successful.\n");

    success = true;

cleanup:
    if (file) fclose(file);
    if (sanitized_filename) free(sanitized_filename);
    if (file_buffer) free(file_buffer);
    if (base64_string) free(base64_string);
    printf("--- Finished processing: %s ---\n\n", file_path);
    return success;
}

// Helper to create a dummy file
void create_test_file(const char* filename, const char* content) {
    FILE* f = fopen(filename, "w");
    if (f) {
        fputs(content, f);
        fclose(f);
    }
}

int main() {
    // --- Test Case Setup ---
    const char* valid_file = "test_image.jpg";
    const char* large_file = "large_image.png";
    const char* bad_name_file = "bad'name;--.txt";
    create_test_file(valid_file, "This is a dummy image file.");
    create_test_file(large_file, "This is to test size check logic.");
    create_test_file(bad_name_file, "bad content");

    // --- Running 5 Test Cases ---
    printf("====== Running C Test Cases ======\n");

    // 1. Valid upload
    upload_image(valid_file);

    // 2. File does not exist
    upload_image("non_existent_file.gif");

    // 3. File path with traversal attempt
    upload_image("../../etc/passwd");

    // 4. Filename with invalid characters
    upload_image(bad_name_file);

    // 5. File is too large (conceptual test)
    printf("Testing large file (will pass if its actual size is small, but demonstrates the check is in place)...\n");
    upload_image(large_file);

    // --- Test Case Cleanup ---
    remove(valid_file);
    remove(large_file);
    remove(bad_name_file);

    return 0;
}