
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char* base64_encode(const unsigned char* data, size_t input_length) {
    size_t output_length = 4 * ((input_length + 2) / 3);
    char* encoded_data = (char*)malloc(output_length + 1);
    if (encoded_data == NULL) return NULL;

    size_t i, j;
    for (i = 0, j = 0; i < input_length;) {
        unsigned int octet_a = i < input_length ? data[i++] : 0;
        unsigned int octet_b = i < input_length ? data[i++] : 0;
        unsigned int octet_c = i < input_length ? data[i++] : 0;

        unsigned int triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = base64_chars[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 0 * 6) & 0x3F];
    }

    int mod_table[] = {0, 2, 1};
    for (i = 0; i < mod_table[input_length % 3]; i++)
        encoded_data[output_length - 1 - i] = '=';

    encoded_data[output_length] = '\\0';
    return encoded_data;
}

void upload_image(const char* input_image_path, const char* output_file_path) {
    FILE* image_file = fopen(input_image_path, "rb");
    if (image_file == NULL) {
        printf("Error: Could not open input file\\n");
        return;
    }

    // Get file size
    fseek(image_file, 0, SEEK_END);
    long file_size = ftell(image_file);
    fseek(image_file, 0, SEEK_SET);

    // Read the image file
    unsigned char* image_data = (unsigned char*)malloc(file_size);
    if (image_data == NULL) {
        printf("Error: Memory allocation failed\\n");
        fclose(image_file);
        return;
    }

    fread(image_data, 1, file_size, image_file);
    fclose(image_file);

    // Encode the image using base64
    char* encoded_image = base64_encode(image_data, file_size);
    free(image_data);

    if (encoded_image == NULL) {
        printf("Error: Encoding failed\\n");
        return;
    }

    // Write the encoded image to a file
    FILE* output_file = fopen(output_file_path, "w");
    if (output_file == NULL) {
        printf("Error: Could not open output file\\n");
        free(encoded_image);
        return;
    }

    fprintf(output_file, "%s", encoded_image);
    fclose(output_file);
    free(encoded_image);

    printf("Image uploaded and encoded successfully!\\n");
}

int main() {
    // Test cases
    printf("Test Case 1:\\n");
    upload_image("test_image1.jpg", "encoded_image1.txt");

    printf("\\nTest Case 2:\\n");
    upload_image("test_image2.png", "encoded_image2.txt");

    printf("\\nTest Case 3:\\n");
    upload_image("test_image3.gif", "encoded_image3.txt");

    printf("\\nTest Case 4:\\n");
    upload_image("test_image4.bmp", "encoded_image4.txt");

    printf("\\nTest Case 5:\\n");
    upload_image("test_image5.jpeg", "encoded_image5.txt");

    return 0;
}
