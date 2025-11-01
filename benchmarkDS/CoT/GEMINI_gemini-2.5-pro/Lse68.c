#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Base64 encoding utility ---
// This implementation is self-contained to adhere to the single-file requirement.
// It is a common and widely-used implementation style.
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

    size_t mod_table[] = {0, 2, 1};
    for (size_t i = 0; i < mod_table[input_length % 3]; i++) {
        encoded_data[output_length - 1 - i] = '=';
    }
    
    encoded_data[output_length] = '\0';
    return encoded_data;
}
// --- End Base64 utility ---


/**
 * Reads an image file, converts it to a Base64 string, and "inserts" it into a database
 * by printing the result.
 * @param filePath The path to the image file.
 */
void upload_image(const char* filePath) {
    unsigned char *buffer = NULL;
    char *base64_string = NULL;
    FILE *imageFile = NULL;
    
    // Security: Prevent path traversal by extracting only the filename.
    // Find the last occurrence of a path separator.
    const char *last_slash = strrchr(filePath, '/');
    const char *last_bslash = strrchr(filePath, '\\');
    const char *fileName = filePath;

    if (last_slash && last_bslash) {
        fileName = (last_slash > last_bslash ? last_slash : last_bslash) + 1;
    } else if (last_slash) {
        fileName = last_slash + 1;
    } else if (last_bslash) {
        fileName = last_bslash + 1;
    }
    
    // Open file in binary read mode.
    imageFile = fopen(filePath, "rb");
    if (imageFile == NULL) {
        fprintf(stderr, "Error: Could not open file: %s\n", filePath);
        return; // No resources to clean up yet.
    }

    // Get file size to allocate a correctly-sized buffer.
    fseek(imageFile, 0, SEEK_END);
    long fileSize = ftell(imageFile);
    rewind(imageFile);

    if (fileSize < 0) {
        fprintf(stderr, "Error: Could not determine file size of: %s\n", fileName);
        goto cleanup;
    }
    
    if (fileSize == 0) {
        // Handle empty file case
        base64_string = base64_encode(NULL, 0);
    } else {
        // Allocate memory for the file content.
        buffer = (unsigned char *)malloc(fileSize);
        if (buffer == NULL) {
            fprintf(stderr, "Error: Failed to allocate memory for file: %s\n", fileName);
            goto cleanup;
        }

        // Read the file into the buffer.
        size_t bytesRead = fread(buffer, 1, fileSize, imageFile);
        if (bytesRead != (size_t)fileSize) {
            fprintf(stderr, "Error: Failed to read file: %s\n", fileName);
            goto cleanup;
        }

        // Encode the buffer to a Base64 string.
        base64_string = base64_encode(buffer, bytesRead);
    }
    
    if (base64_string == NULL) {
        fprintf(stderr, "Error: Failed to encode file to Base64: %s\n", fileName);
        goto cleanup;
    }
    
    // "Insert" into the database (by printing).
    // Previewing data for clean printing.
    size_t len = strlen(base64_string);
    printf("Successfully uploaded and encoded '%s'.\n", fileName);
    printf(" -> DB insert: FileName: %s, Base64: %.*s%s\n", 
            fileName, 
            (int)(len > 30 ? 30 : len), 
            base64_string,
            len > 30 ? "..." : "");

cleanup:
    if (imageFile) fclose(imageFile);
    free(buffer);
    free(base64_string);
}

int main() {
    // --- Test Case Setup ---
    FILE* f;
    if ((f = fopen("test1.txt", "w"))) {
        fputs("Hello World", f);
        fclose(f);
    }
    if ((f = fopen("test2.jpg", "wb"))) {
        unsigned char data[] = {0xFF, 0xD8, 0xFF, 0xE0};
        fwrite(data, sizeof(unsigned char), 4, f);
        fclose(f);
    }
    if ((f = fopen("test3.txt", "w"))) {
        fclose(f);
    }

    printf("--- Running Test Cases ---\n");
    // Test Case 1: A valid text file.
    upload_image("test1.txt");

    // Test Case 2: A valid (simulated) image file.
    upload_image("test2.jpg");

    // Test Case 3: An empty file.
    upload_image("test3.txt");

    // Test Case 4: A non-existent file.
    upload_image("nonexistent.png");

    // Test Case 5: A path traversal attempt.
    upload_image("../test1.txt");

    printf("--------------------------------\n\n");

    // --- Test Case Cleanup ---
    remove("test1.txt");
    remove("test2.jpg");
    remove("test3.txt");

    return 0;
}