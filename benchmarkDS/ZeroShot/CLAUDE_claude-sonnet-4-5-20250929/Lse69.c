
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char base64_chars[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

char* base64_encode(const unsigned char* data, size_t input_length) {
    size_t output_length = 4 * ((input_length + 2) / 3);
    char* encoded_data = (char*)malloc(output_length + 1);
    if (encoded_data == NULL) return NULL;
    
    size_t i, j;
    for (i = 0, j = 0; i < input_length;) {
        uint32_t octet_a = i < input_length ? data[i++] : 0;
        uint32_t octet_b = i < input_length ? data[i++] : 0;
        uint32_t octet_c = i < input_length ? data[i++] : 0;
        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;
        
        encoded_data[j++] = base64_chars[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 0 * 6) & 0x3F];
    }
    
    for (i = 0; i < (3 - input_length % 3) % 3; i++) {
        encoded_data[output_length - 1 - i] = '=';
    }
    
    encoded_data[output_length] = '\\0';
    return encoded_data;
}

const char* upload_image(const char* input_image_path, const char* output_file_path) {
    FILE* input_file = fopen(input_image_path, "rb");
    if (input_file == NULL) {
        return "Error: Cannot open input file";
    }
    
    fseek(input_file, 0, SEEK_END);
    long file_size = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);
    
    unsigned char* image_data = (unsigned char*)malloc(file_size);
    if (image_data == NULL) {
        fclose(input_file);
        return "Error: Memory allocation failed";
    }
    
    fread(image_data, 1, file_size, input_file);
    fclose(input_file);
    
    char* encoded_image = base64_encode(image_data, file_size);
    free(image_data);
    
    if (encoded_image == NULL) {
        return "Error: Base64 encoding failed";
    }
    
    FILE* output_file = fopen(output_file_path, "w");
    if (output_file == NULL) {
        free(encoded_image);
        return "Error: Cannot open output file";
    }
    
    fprintf(output_file, "%s", encoded_image);
    fclose(output_file);
    free(encoded_image);
    
    return "Image uploaded and encoded successfully";
}

int main() {
    printf("%s\\n", upload_image("test1.jpg", "encoded1.txt"));
    printf("%s\\n", upload_image("test2.png", "encoded2.txt"));
    printf("%s\\n", upload_image("test3.gif", "encoded3.txt"));
    printf("%s\\n", upload_image("test4.bmp", "encoded4.txt"));
    printf("%s\\n", upload_image("test5.jpeg", "encoded5.txt"));
    
    return 0;
}
