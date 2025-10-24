
#include <stdio.h>
#include <string.h>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <stdlib.h>

#define BLOCK_SIZE 16

static unsigned char STATIC_IV[16] = {0};

unsigned char* pad_data(const unsigned char* data, int data_len, int* padded_len) {
    int padding_length = BLOCK_SIZE - (data_len % BLOCK_SIZE);
    if (padding_length == 0) {
        padding_length = BLOCK_SIZE;
    }
    
    *padded_len = data_len + padding_length;
    unsigned char* padded_data = (unsigned char*)malloc(*padded_len);
    
    memcpy(padded_data, data, data_len);
    
    for (int i = data_len; i < *padded_len; i++) {
        padded_data[i] = (unsigned char)padding_length;
    }
    
    return padded_data;
}

unsigned char* encrypt_data(const unsigned char* data, int data_len, 
                            const unsigned char* key, int* encrypted_len) {
    int padded_len;
    unsigned char* padded_data = pad_data(data, data_len, &padded_len);
    
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    unsigned char* encrypted_data = (unsigned char*)malloc(padded_len + BLOCK_SIZE);
    
    int len;
    int ciphertext_len;
    
    EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, STATIC_IV);
    EVP_CIPHER_CTX_set_padding(ctx, 0);
    
    EVP_EncryptUpdate(ctx, encrypted_data, &len, padded_data, padded_len);
    ciphertext_len = len;
    
    EVP_EncryptFinal_ex(ctx, encrypted_data + len, &len);
    ciphertext_len += len;
    
    EVP_CIPHER_CTX_free(ctx);
    free(padded_data);
    
    *encrypted_len = ciphertext_len;
    return encrypted_data;
}

void print_hex(const unsigned char* data, int len) {
    for (int i = 0; i < len; i++) {
        printf("%02x", data[i]);
    }
    printf("\\n");
}

int main() {
    unsigned char key[16] = {'1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6'};
    
    // Test case 1
    const char* input1 = "Hello World!";
    int encrypted_len1;
    unsigned char* encrypted1 = encrypt_data((unsigned char*)input1, strlen(input1), key, &encrypted_len1);
    printf("Test 1 - Input: %s\\n", input1);
    printf("Encrypted: ");
    print_hex(encrypted1, encrypted_len1);
    free(encrypted1);
    
    // Test case 2
    const char* input2 = "SecureData";
    int encrypted_len2;
    unsigned char* encrypted2 = encrypt_data((unsigned char*)input2, strlen(input2), key, &encrypted_len2);
    printf("\\nTest 2 - Input: %s\\n", input2);
    printf("Encrypted: ");
    print_hex(encrypted2, encrypted_len2);
    free(encrypted2);
    
    // Test case 3
    const char* input3 = "A";
    int encrypted_len3;
    unsigned char* encrypted3 = encrypt_data((unsigned char*)input3, strlen(input3), key, &encrypted_len3);
    printf("\\nTest 3 - Input: %s\\n", input3);
    printf("Encrypted: ");
    print_hex(encrypted3, encrypted_len3);
    free(encrypted3);
    
    // Test case 4
    const char* input4 = "This is a longer message to test encryption";
    int encrypted_len4;
    unsigned char* encrypted4 = encrypt_data((unsigned char*)input4, strlen(input4), key, &encrypted_len4);
    printf("\\nTest 4 - Input: %s\\n", input4);
    printf("Encrypted: ");
    print_hex(encrypted4, encrypted_len4);
    free(encrypted4);
    
    // Test case 5
    const char* input5 = "1234567890123456";
    int encrypted_len5;
    unsigned char* encrypted5 = encrypt_data((unsigned char*)input5, strlen(input5), key, &encrypted_len5);
    printf("\\nTest 5 - Input: %s\\n", input5);
    printf("Encrypted: ");
    print_hex(encrypted5, encrypted_len5);
    free(encrypted5);
    
    return 0;
}
