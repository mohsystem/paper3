#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Start of Base64 encoding utility ---
// This implementation is a common, public-domain one, adapted for C.
// It's included here to make the file self-contained.
static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};
static int mod_table[] = {0, 2, 1};

char *base64_encode(const unsigned char *data, size_t input_length, size_t *output_length) {
    *output_length = 4 * ((input_length + 2) / 3);
    char *encoded_data = malloc(*output_length + 1); // +1 for null terminator
    if (encoded_data == NULL) return NULL;

    for (size_t i = 0, j = 0; i < input_length;) {
        unsigned int octet_a = i < input_length ? data[i++] : 0;
        unsigned int octet_b = i < input_length ? data[i++] : 0;
        unsigned int octet_c = i < input_length ? data[i++] : 0;
        unsigned int triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;
        encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
    }

    for (int i = 0; i < mod_table[input_length % 3]; i++) {
        encoded_data[*output_length - 1 - i] = '=';
    }
    
    encoded_data[*output_length] = '\0';
    return encoded_data;
}
// --- End of Base64 encoding utility ---

/**
 * Reads an image file, encodes it to Base64, and writes the result to an output file.
 *
 * @param imagePath The path to the input image file.
 * @param outputPath The path to the output file for the Base64 string.
 * @return 0 on success, -1 on failure.
 */
int upload_image(const char* imagePath, const char* outputPath) {
    // Security: Basic path traversal check.
    if (strstr(imagePath, "..") != NULL || strstr(outputPath, "..") != NULL) {
        fprintf(stderr, "Error: Path traversal attempt detected.\n");
        return -1;
    }

    FILE *inFile = NULL;
    FILE *outFile = NULL;
    unsigned char *buffer = NULL;
    char *encoded_data = NULL;
    int ret_val = -1; // Default to failure

    inFile = fopen(imagePath, "rb");
    if (inFile == NULL) {
        perror("Error opening input file");
        goto cleanup;
    }

    fseek(inFile, 0, SEEK_END);
    long file_size = ftell(inFile);
    if (file_size < 0) {
        perror("Error getting file size");
        goto cleanup;
    }
    fseek(inFile, 0, SEEK_SET);

    // Reading the entire file into memory can be a security risk (resource exhaustion) for large files.
    if (file_size > 0) {
        buffer = (unsigned char *)malloc(file_size);
        if (buffer == NULL) {
            fprintf(stderr, "Error: Failed to allocate memory for file content.\n");
            goto cleanup;
        }
        if (fread(buffer, 1, file_size, inFile) != (size_t)file_size) {
            fprintf(stderr, "Error: Failed to read input file.\n");
            goto cleanup;
        }
    }

    size_t encoded_size;
    encoded_data = base64_encode(buffer, file_size, &encoded_size);
    if (encoded_data == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for encoded data.\n");
        goto cleanup;
    }
    
    outFile = fopen(outputPath, "w");
    if (outFile == NULL) {
        // Security: Calling system() is a risk. Used here for test convenience.
        #if defined(_WIN32)
            char cmd[512];
            char drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
            _splitpath_s(outputPath, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);
            snprintf(cmd, sizeof(cmd), "mkdir \"%s%s\"", drive, dir);
            system(cmd);
        #else
            char cmd[512];
            char* last_slash = strrchr(outputPath, '/');
            if (last_slash) {
                size_t dir_len = last_slash - outputPath;
                char dir_path[256];
                strncpy(dir_path, outputPath, dir_len);
                dir_path[dir_len] = '\0';
                snprintf(cmd, sizeof(cmd), "mkdir -p \"%s\"", dir_path);
                system(cmd);
            }
        #endif
        outFile = fopen(outputPath, "w"); // Try again
        if (outFile == NULL) {
            perror("Error opening output file");
            goto cleanup;
        }
    }

    if (fputs(encoded_data, outFile) == EOF) {
        fprintf(stderr, "Error: Failed to write to output file.\n");
        goto cleanup;
    }
    
    ret_val = 0; // Success

cleanup:
    if (inFile) fclose(inFile);
    if (outFile) fclose(outFile);
    free(buffer);
    free(encoded_data);
    return ret_val;
}

// --- Main for Test Cases ---
void create_dummy_c_file(const char* path, const char* content, size_t len) {
    FILE* f = fopen(path, "wb");
    if (f) {
        if (len > 0) fwrite(content, 1, len, f);
        fclose(f);
    }
}

void read_and_print_file(const char* path) {
    FILE* f = fopen(path, "r");
    if (f) {
        char buffer[256] = {0};
        size_t n = fread(buffer, 1, sizeof(buffer) - 1, f);
        printf("Content: %s\n", n > 0 ? buffer : "(empty)");
        fclose(f);
    }
}

int main() {
    const char* temp_dir = "temp_test_c";
    #if defined(_WIN32)
        system("if exist temp_test_c ( rmdir /s /q temp_test_c )");
        system("mkdir temp_test_c");
    #else
        system("rm -rf temp_test_c && mkdir -p temp_test_c");
    #endif

    char dummy_image_path[256], empty_image_path[256];
    snprintf(dummy_image_path, sizeof(dummy_image_path), "%s/test_image.jpg", temp_dir);
    snprintf(empty_image_path, sizeof(empty_image_path), "%s/empty.jpg", temp_dir);

    char dummy_content[] = { (char)0xff, (char)0xd8, (char)0xff, (char)0xe0, 0x01, 0x02, 0x03, 0x04 };
    create_dummy_c_file(dummy_image_path, dummy_content, sizeof(dummy_content));
    create_dummy_c_file(empty_image_path, "", 0);

    printf("--- C Test Cases ---\n");
    
    // Test Case 1: Successful upload
    printf("Test Case 1: Normal valid image path\n");
    char output_path1[256];
    snprintf(output_path1, sizeof(output_path1), "%s/output1.txt", temp_dir);
    int result1 = upload_image(dummy_image_path, output_path1);
    printf("%s\n", result1 == 0 ? "Success" : "Failed");
    if (result1 == 0) read_and_print_file(output_path1);
    printf("\n");

    // Test Case 2: Non-existent image path
    printf("Test Case 2: Non-existent image path\n");
    char output_path2[256];
    snprintf(output_path2, sizeof(output_path2), "%s/output2.txt", temp_dir);
    printf("%s\n", upload_image("non_existent_image.jpg", output_path2) == 0 ? "Success" : "Failed");
    printf("\n");

    // Test Case 3: Output to non-existent directory
    printf("Test Case 3: Output to non-existent directory\n");
    char output_path3[256];
    snprintf(output_path3, sizeof(output_path3), "%s/new_dir/output3.txt", temp_dir);
    printf("%s\n", upload_image(dummy_image_path, output_path3) == 0 ? "Success" : "Failed");
    printf("\n");

    // Test Case 4: Path traversal attempt
    printf("Test Case 4: Path traversal attempt\n");
    printf("%s\n", upload_image(dummy_image_path, "../output4.txt") == 0 ? "Success" : "Failed");
    printf("\n");

    // Test Case 5: Empty input file
    printf("Test Case 5: Empty input file\n");
    char output_path5[256];
    snprintf(output_path5, sizeof(output_path5), "%s/output5.txt", temp_dir);
    int result5 = upload_image(empty_image_path, output_path5);
    printf("%s\n", result5 == 0 ? "Success" : "Failed");
    if (result5 == 0) read_and_print_file(output_path5);
    printf("\n");

    return 0;
}