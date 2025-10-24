
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

#define KEY_SIZE 32
#define IV_SIZE 16
#define BLOCK_SIZE 16

typedef struct {
    unsigned char* data;
    size_t size;
} Buffer;

void generate_key(const char* password, unsigned char* key) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password, strlen(password), hash);
    memcpy(key, hash, KEY_SIZE);
}

Buffer encrypt_data(const unsigned char* data, size_t data_len, const char* password) {
    Buffer result = {NULL, 0};
    unsigned char key[KEY_SIZE];
    unsigned char iv[IV_SIZE];
    
    generate_key(password, key);
    
    if (RAND_bytes(iv, IV_SIZE) != 1) {
        fprintf(stderr, "Failed to generate IV\\n");
        return result;
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        fprintf(stderr, "Failed to create cipher context\\n");
        return result;
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        fprintf(stderr, "Failed to initialize encryption\\n");
        return result;
    }

    size_t max_encrypted_size = data_len + BLOCK_SIZE;
    unsigned char* encrypted = (unsigned char*)malloc(max_encrypted_size);
    if (!encrypted) {
        EVP_CIPHER_CTX_free(ctx);
        return result;
    }

    int len = 0;
    int ciphertext_len = 0;

    if (EVP_EncryptUpdate(ctx, encrypted, &len, data, data_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(encrypted);
        fprintf(stderr, "Encryption failed\\n");
        return result;
    }
    ciphertext_len = len;

    if (EVP_EncryptFinal_ex(ctx, encrypted + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(encrypted);
        fprintf(stderr, "Encryption finalization failed\\n");
        return result;
    }
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    result.size = IV_SIZE + ciphertext_len;
    result.data = (unsigned char*)malloc(result.size);
    if (!result.data) {
        free(encrypted);
        result.size = 0;
        return result;
    }

    memcpy(result.data, iv, IV_SIZE);
    memcpy(result.data + IV_SIZE, encrypted, ciphertext_len);
    
    free(encrypted);
    return result;
}

Buffer decrypt_data(const unsigned char* encrypted_data, size_t encrypted_len, const char* password) {
    Buffer result = {NULL, 0};
    unsigned char key[KEY_SIZE];
    unsigned char iv[IV_SIZE];
    
    if (encrypted_len < IV_SIZE) {
        fprintf(stderr, "Invalid encrypted data\\n");
        return result;
    }
    
    generate_key(password, key);
    memcpy(iv, encrypted_data, IV_SIZE);
    
    const unsigned char* ciphertext = encrypted_data + IV_SIZE;
    size_t ciphertext_len = encrypted_len - IV_SIZE;

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        fprintf(stderr, "Failed to create cipher context\\n");
        return result;
    }

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        fprintf(stderr, "Failed to initialize decryption\\n");
        return result;
    }

    unsigned char* decrypted = (unsigned char*)malloc(ciphertext_len);
    if (!decrypted) {
        EVP_CIPHER_CTX_free(ctx);
        return result;
    }

    int len = 0;
    int plaintext_len = 0;

    if (EVP_DecryptUpdate(ctx, decrypted, &len, ciphertext, ciphertext_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(decrypted);
        fprintf(stderr, "Decryption failed\\n");
        return result;
    }
    plaintext_len = len;

    if (EVP_DecryptFinal_ex(ctx, decrypted + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(decrypted);
        fprintf(stderr, "Decryption finalization failed\\n");
        return result;
    }
    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    result.data = decrypted;
    result.size = plaintext_len;
    return result;
}

int encrypt_file(const char* input_file, const char* output_file, const char* password) {
    FILE* in = fopen(input_file, "rb");
    if (!in) {
        fprintf(stderr, "Cannot open input file\\n");
        return 0;
    }

    fseek(in, 0, SEEK_END);
    long file_size = ftell(in);
    fseek(in, 0, SEEK_SET);

    unsigned char* file_data = (unsigned char*)malloc(file_size);
    if (!file_data) {
        fclose(in);
        return 0;
    }

    fread(file_data, 1, file_size, in);
    fclose(in);

    Buffer encrypted = encrypt_data(file_data, file_size, password);
    free(file_data);

    if (!encrypted.data) {
        return 0;
    }

    FILE* out = fopen(output_file, "wb");
    if (!out) {
        free(encrypted.data);
        fprintf(stderr, "Cannot open output file\\n");
        return 0;
    }

    fwrite(encrypted.data, 1, encrypted.size, out);
    fclose(out);
    free(encrypted.data);

    return 1;
}

int decrypt_file(const char* input_file, const char* output_file, const char* password) {
    FILE* in = fopen(input_file, "rb");
    if (!in) {
        fprintf(stderr, "Cannot open input file\\n");
        return 0;
    }

    fseek(in, 0, SEEK_END);
    long file_size = ftell(in);
    fseek(in, 0, SEEK_SET);

    unsigned char* encrypted_data = (unsigned char*)malloc(file_size);
    if (!encrypted_data) {
        fclose(in);
        return 0;
    }

    fread(encrypted_data, 1, file_size, in);
    fclose(in);

    Buffer decrypted = decrypt_data(encrypted_data, file_size, password);
    free(encrypted_data);

    if (!decrypted.data) {
        return 0;
    }

    FILE* out = fopen(output_file, "wb");
    if (!out) {
        free(decrypted.data);
        fprintf(stderr, "Cannot open output file\\n");
        return 0;
    }

    fwrite(decrypted.data, 1, decrypted.size, out);
    fclose(out);
    free(decrypted.data);

    return 1;
}

int main() {
    // Test Case 1: Simple text encryption/decryption
    printf("Test Case 1: Simple text encryption/decryption\\n");
    const char* text1 = "Hello, World!";
    const char* password1 = "SecurePassword123";
    Buffer encrypted1 = encrypt_data((unsigned char*)text1, strlen(text1), password1);
    Buffer decrypted1 = decrypt_data(encrypted1.data, encrypted1.size, password1);
    printf("Original: %s\\n", text1);
    printf("Decrypted: %.*s\\n", (int)decrypted1.size, decrypted1.data);
    printf("Match: %s\\n\\n", (strncmp(text1, (char*)decrypted1.data, strlen(text1)) == 0) ? "true" : "false");
    free(encrypted1.data);
    free(decrypted1.data);

    // Test Case 2: Longer text with special characters
    printf("Test Case 2: Longer text with special characters\\n");
    const char* text2 = "This is a secure message with special chars: @#$%^&*()";
    const char* password2 = "AnotherSecurePass456";
    Buffer encrypted2 = encrypt_data((unsigned char*)text2, strlen(text2), password2);
    Buffer decrypted2 = decrypt_data(encrypted2.data, encrypted2.size, password2);
    printf("Original: %s\\n", text2);
    printf("Decrypted: %.*s\\n", (int)decrypted2.size, decrypted2.data);
    printf("Match: %s\\n\\n", (strncmp(text2, (char*)decrypted2.data, strlen(text2)) == 0) ? "true" : "false");
    free(encrypted2.data);
    free(decrypted2.data);

    // Test Case 3: Empty string
    printf("Test Case 3: Empty string\\n");
    const char* text3 = "";
    const char* password3 = "password";
    Buffer encrypted3 = encrypt_data((unsigned char*)text3, strlen(text3), password3);
    Buffer decrypted3 = decrypt_data(encrypted3.data, encrypted3.size, password3);
    printf("Original length: %zu\\n", strlen(text3));
    printf("Decrypted length: %zu\\n", decrypted3.size);
    printf("Match: %s\\n\\n", (strlen(text3) == decrypted3.size) ? "true" : "false");
    free(encrypted3.data);
    free(decrypted3.data);

    // Test Case 4: Numeric data
    printf("Test Case 4: Numeric data\\n");
    const char* text4 = "1234567890";
    const char* password4 = "NumericPass999";
    Buffer encrypted4 = encrypt_data((unsigned char*)text4, strlen(text4), password4);
    Buffer decrypted4 = decrypt_data(encrypted4.data, encrypted4.size, password4);
    printf("Original: %s\\n", text4);
    printf("Decrypted: %.*s\\n", (int)decrypted4.size, decrypted4.data);
    printf("Match: %s\\n\\n", (strncmp(text4, (char*)decrypted4.data, strlen(text4)) == 0) ? "true" : "false");
    free(encrypted4.data);
    free(decrypted4.data);

    // Test Case 5: File encryption/decryption
    printf("Test Case 5: File encryption/decryption\\n");
    const char* test_content = "This is a test file content for encryption.";
    
    FILE* f = fopen("test_input.txt", "w");
    if (f) {
        fprintf(f, "%s", test_content);
        fclose(f);
    }

    encrypt_file("test_input.txt", "test_encrypted.bin", "FilePassword123");
    decrypt_file("test_encrypted.bin", "test_decrypted.txt", "FilePassword123");

    f = fopen("test_decrypted.txt", "r");
    if (f) {
        char buffer[256];
        fgets(buffer, sizeof(buffer), f);
        fclose(f);
        printf("Original file content: %s\\n", test_content);
        printf("Decrypted file content: %s\\n", buffer);
        printf("Match: %s\\n", (strcmp(test_content, buffer) == 0) ? "true" : "false");
    }

    // Cleanup
    remove("test_input.txt");
    remove("test_encrypted.bin");
    remove("test_decrypted.txt");

    return 0;
}
