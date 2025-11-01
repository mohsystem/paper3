#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper function to encode binary data to Base64.
// The caller is responsible for freeing the returned memory.
char *base64_encode(const unsigned char *data, size_t input_length) {
    static const char encoding_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                        "abcdefghijklmnopqrstuvwxyz"
                                        "0123456789+/";
    
    size_t output_length = 4 * ((input_length + 2) / 3);
    char *encoded_data = malloc(output_length + 1); // +1 for the null terminator
    if (encoded_data == NULL) return NULL;

    for (size_t i = 0, j = 0; i < input_length;) {
        uint32_t octet_a = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_b = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_c = i < input_length ? (unsigned char)data[i++] : 0;

        uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;

        encoded_data[j++] = encoding_table[(triple >> 18) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 12) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 6) & 0x3F];
        encoded_data[j++] = encoding_table[triple & 0x3F];
    }
    
    // Add padding characters
    static const int mod_table[] = {0, 2, 1};
    for (int i = 0; i < mod_table[input_length % 3]; i++) {
        encoded_data[output_length - 1 - i] = '=';
    }
    
    encoded_data[output_length] = '\0'; // Null-terminate the string
    return encoded_data;
}


/**
 * @brief Reads an image file, encodes it to Base64, and writes the encoded string to a text file.
 *
 * @param imagePath The path to the input image file.
 * @param outputPath The path to the output file where the Base64 string will be saved.
 */
void upload_image(const char* imagePath, const char* outputPath) {
    FILE *inputFile = fopen(imagePath, "rb");
    if (!inputFile) {
        fprintf(stderr, "Error: Could not open input file %s\n", imagePath);
        return;
    }

    // Get file size
    fseek(inputFile, 0, SEEK_END);
    long fileSize = ftell(inputFile);
    fseek(inputFile, 0, SEEK_SET);

    // Read file content
    unsigned char *buffer = (unsigned char *)malloc(fileSize);
    if (!buffer) {
        fprintf(stderr, "Error: Could not allocate memory for file content.\n");
        fclose(inputFile);
        return;
    }
    fread(buffer, 1, fileSize, inputFile);
    fclose(inputFile);

    // Encode data
    char *encoded_data = base64_encode(buffer, fileSize);
    free(buffer);
    if (!encoded_data) {
        fprintf(stderr, "Error: Base64 encoding failed.\n");
        return;
    }

    // Write encoded data to output file
    FILE *outputFile = fopen(outputPath, "w");
    if (!outputFile) {
        fprintf(stderr, "Error: Could not open output file %s\n", outputPath);
        free(encoded_data);
        return;
    }
    fprintf(outputFile, "%s", encoded_data);
    fclose(outputFile);

    free(encoded_data);
}


// Helper function to create a dummy file for testing
void createTestFile(const char* fileName, const char* content, size_t len) {
    FILE* file = fopen(fileName, "wb");
    if (file) {
        fwrite(content, 1, len, file);
        fclose(file);
    }
}

// Helper function to read a file's content
char* readTestFile(const char* fileName) {
    FILE* file = fopen(fileName, "r");
    if (!file) return NULL;
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* content = (char*)malloc(size + 1);
    fread(content, 1, size, file);
    content[size] = '\0';
    fclose(file);
    return content;
}

int main() {
    // --- Test Cases ---
    const char* test_inputs[] = {
        "Man",          // Test case 1: Length is a multiple of 3
        "Ma",           // Test case 2: Length % 3 == 2
        "M",            // Test case 3: Length % 3 == 1
        "",             // Test case 4: Empty content
        "Hello, World!" // Test case 5: A longer string
    };
    int num_tests = sizeof(test_inputs) / sizeof(test_inputs[0]);

    for (int i = 0; i < num_tests; ++i) {
        char input_filename[20];
        char output_filename[20];
        sprintf(input_filename, "test_input_%d.bin", i);
        sprintf(output_filename, "test_output_%d.txt", i);

        printf("--- Running Test Case %d ---\n", i + 1);
        printf("Input data: \"%s\"\n", test_inputs[i]);
        
        // 1. Create dummy input file
        createTestFile(input_filename, test_inputs[i], strlen(test_inputs[i]));

        // 2. Call the function
        upload_image(input_filename, output_filename);
        printf("upload_image() called. Encoded data written to %s\n", output_filename);

        // 3. (Optional) Verify output
        char* encoded_content = readTestFile(output_filename);
        if (encoded_content) {
            printf("Encoded output: %s\n", encoded_content);
            free(encoded_content);
        }
        printf("Test Case %d completed.\n\n", i + 1);

        // 4. Clean up files
        remove(input_filename);
        remove(output_filename);
    }

    return 0;
}