
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define KEY 123
#define MAX_SIZE 10000

// Base64 encoding table
static const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void encrypt_decrypt(unsigned char* data, int size, int key) {
    for (int i = 0; i < size; i++) {
        data[i] ^= key;
    }
}

void base64_encode(const unsigned char* input, int input_len, char* output) {
    int i = 0, j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    int out_pos = 0;

    while (input_len--) {
        char_array_3[i++] = *(input++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for(i = 0; i < 4; i++)
                output[out_pos++] = base64_table[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for(j = i; j < 3; j++)
            char_array_3[j] = '\\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

        for (j = 0; j < i + 1; j++)
            output[out_pos++] = base64_table[char_array_4[j]];

        while(i++ < 3)
            output[out_pos++] = '=';
    }
    output[out_pos] = '\\0';
}

int base64_decode_value(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

int base64_decode(const char* input, unsigned char* output) {
    int input_len = strlen(input);
    int i = 0, j = 0, out_pos = 0;
    unsigned char char_array_4[4], char_array_3[3];

    while (input_len-- && input[j] != '=') {
        int val = base64_decode_value(input[j++]);
        if (val == -1) break;
        char_array_4[i++] = val;
        if (i == 4) {
            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; i < 3; i++)
                output[out_pos++] = char_array_3[i];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++)
            char_array_4[j] = 0;

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);

        for (j = 0; j < i - 1; j++)
            output[out_pos++] = char_array_3[j];
    }

    return out_pos;
}

void encrypt_file(const char* input_file, const char* output_file, int key) {
    FILE *in = fopen(input_file, "rb");
    if (!in) return;
    
    fseek(in, 0, SEEK_END);
    long size = ftell(in);
    fseek(in, 0, SEEK_SET);
    
    unsigned char* data = (unsigned char*)malloc(size);
    fread(data, 1, size, in);
    fclose(in);
    
    encrypt_decrypt(data, size, key);
    
    FILE *out = fopen(output_file, "wb");
    fwrite(data, 1, size, out);
    fclose(out);
    free(data);
}

void decrypt_file(const char* input_file, const char* output_file, int key) {
    encrypt_file(input_file, output_file, key);
}

void encrypt_string(const char* plaintext, int key, char* output) {
    int len = strlen(plaintext);
    unsigned char* data = (unsigned char*)malloc(len);
    memcpy(data, plaintext, len);
    
    encrypt_decrypt(data, len, key);
    base64_encode(data, len, output);
    
    free(data);
}

void decrypt_string(const char* encrypted, int key, char* output) {
    unsigned char decoded[MAX_SIZE];
    int decoded_len = base64_decode(encrypted, decoded);
    
    encrypt_decrypt(decoded, decoded_len, key);
    memcpy(output, decoded, decoded_len);
    output[decoded_len] = '\\0';
}

int main() {
    printf("File Encryption/Decryption Program\\n");
    printf("===================================\\n\\n");
    
    // Test Case 1: Simple String Encryption
    printf("Test Case 1: Simple String Encryption\\n");
    char text1[] = "Hello World!";
    char encrypted1[MAX_SIZE];
    char decrypted1[MAX_SIZE];
    encrypt_string(text1, KEY, encrypted1);
    decrypt_string(encrypted1, KEY, decrypted1);
    printf("Original: %s\\n", text1);
    printf("Encrypted: %s\\n", encrypted1);
    printf("Decrypted: %s\\n", decrypted1);
    printf("Match: %s\\n\\n", strcmp(text1, decrypted1) == 0 ? "true" : "false");
    
    // Test Case 2: Longer Text Encryption
    printf("Test Case 2: Longer Text Encryption\\n");
    char text2[] = "This is a test message with numbers 12345 and symbols @#$%";
    char encrypted2[MAX_SIZE];
    char decrypted2[MAX_SIZE];
    encrypt_string(text2, KEY, encrypted2);
    decrypt_string(encrypted2, KEY, decrypted2);
    printf("Original: %s\\n", text2);
    printf("Encrypted: %s\\n", encrypted2);
    printf("Decrypted: %s\\n", decrypted2);
    printf("Match: %s\\n\\n", strcmp(text2, decrypted2) == 0 ? "true" : "false");
    
    // Test Case 3: Different Key
    printf("Test Case 3: Different Key\\n");
    char text3[] = "Secret Message";
    int custom_key = 255;
    char encrypted3[MAX_SIZE];
    char decrypted3[MAX_SIZE];
    encrypt_string(text3, custom_key, encrypted3);
    decrypt_string(encrypted3, custom_key, decrypted3);
    printf("Original: %s\\n", text3);
    printf("Encrypted: %s\\n", encrypted3);
    printf("Decrypted: %s\\n", decrypted3);
    printf("Match: %s\\n\\n", strcmp(text3, decrypted3) == 0 ? "true" : "false");
    
    // Test Case 4: Empty String
    printf("Test Case 4: Empty String\\n");
    char text4[] = "";
    char encrypted4[MAX_SIZE];
    char decrypted4[MAX_SIZE];
    encrypt_string(text4, KEY, encrypted4);
    decrypt_string(encrypted4, KEY, decrypted4);
    printf("Original: '%s'\\n", text4);
    printf("Encrypted: '%s'\\n", encrypted4);
    printf("Decrypted: '%s'\\n", decrypted4);
    printf("Match: %s\\n\\n", strcmp(text4, decrypted4) == 0 ? "true" : "false");
    
    // Test Case 5: File Encryption/Decryption
    printf("Test Case 5: File Encryption/Decryption\\n");
    FILE *f = fopen("test_input.txt", "w");
    fprintf(f, "This is a test file content.\\nLine 2 with more data.");
    fclose(f);
    
    encrypt_file("test_input.txt", "test_encrypted.bin", KEY);
    printf("File encrypted: test_encrypted.bin\\n");
    
    decrypt_file("test_encrypted.bin", "test_decrypted.txt", KEY);
    printf("File decrypted: test_decrypted.txt\\n");
    
    f = fopen("test_decrypted.txt", "r");
    char content[MAX_SIZE];
    fread(content, 1, MAX_SIZE, f);
    content[ftell(f)] = '\\0';
    fclose(f);
    printf("Decrypted content: %s\\n", content);
    
    return 0;
}
