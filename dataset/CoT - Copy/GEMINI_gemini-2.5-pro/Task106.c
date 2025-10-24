#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

void handle_errors(const char* message) {
    fprintf(stderr, "Error: %s\n", message);
    ERR_print_errors_fp(stderr);
    exit(1);
}

// Helper to convert binary data to a Base64 string for printing
char* to_base64(const unsigned char* data, size_t data_len) {
    BIO* bio = BIO_new(BIO_s_mem());
    BIO* b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, data, data_len);
    BIO_flush(bio);
    BUF_MEM* buffer_ptr;
    BIO_get_mem_ptr(bio, &buffer_ptr);
    char* base64_str = (char*)malloc(buffer_ptr->length + 1);
    if (base64_str) {
        memcpy(base64_str, buffer_ptr->data, buffer_ptr->length);
        base64_str[buffer_ptr->length] = '\0';
    }
    BIO_free_all(bio);
    return base64_str;
}

EVP_PKEY* generateRsaKeyPair() {
    EVP_PKEY* pkey = NULL;
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    if (!ctx) handle_errors("EVP_PKEY_CTX_new_id failed.");

    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        handle_errors("EVP_PKEY_keygen_init failed.");
    }
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        handle_errors("EVP_PKEY_CTX_set_rsa_keygen_bits failed.");
    }
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        handle_errors("EVP_PKEY_keygen failed.");
    }

    EVP_PKEY_CTX_free(ctx);
    return pkey;
}

// Encrypt data, caller must free the returned buffer
unsigned char* encrypt_data(EVP_PKEY* pkey, const char* plain_text, size_t* encrypted_len) {
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pkey, NULL);
    if (!ctx) handle_errors("EVP_PKEY_CTX_new failed for encryption.");

    if (EVP_PKEY_encrypt_init(ctx) <= 0) handle_errors("EVP_PKEY_encrypt_init failed.");
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) handle_errors("EVP_PKEY_CTX_set_rsa_padding failed.");
    
    size_t plain_text_len = strlen(plain_text);

    // Get the required buffer size
    if (EVP_PKEY_encrypt(ctx, NULL, encrypted_len, (const unsigned char*)plain_text, plain_text_len) <= 0) handle_errors("EVP_PKEY_encrypt size check failed.");
    
    unsigned char* encrypted_text = malloc(*encrypted_len);
    if (!encrypted_text) handle_errors("Malloc failed for encrypted text.");

    // Perform encryption
    if (EVP_PKEY_encrypt(ctx, encrypted_text, encrypted_len, (const unsigned char*)plain_text, plain_text_len) <= 0) handle_errors("EVP_PKEY_encrypt failed.");

    EVP_PKEY_CTX_free(ctx);
    return encrypted_text;
}

// Decrypt data, caller must free the returned buffer
char* decrypt_data(EVP_PKEY* pkey, const unsigned char* encrypted_text, size_t encrypted_len, size_t* decrypted_len) {
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pkey, NULL);
    if (!ctx) handle_errors("EVP_PKEY_CTX_new failed for decryption.");
    
    if (EVP_PKEY_decrypt_init(ctx) <= 0) handle_errors("EVP_PKEY_decrypt_init failed.");
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) handle_errors("EVP_PKEY_CTX_set_rsa_padding failed.");

    // Get the required buffer size
    if (EVP_PKEY_decrypt(ctx, NULL, decrypted_len, encrypted_text, encrypted_len) <= 0) handle_errors("EVP_PKEY_decrypt size check failed.");
    
    char* decrypted_text = malloc(*decrypted_len + 1); // +1 for null terminator
    if (!decrypted_text) handle_errors("Malloc failed for decrypted text.");
    
    // Perform decryption
    if (EVP_PKEY_decrypt(ctx, (unsigned char*)decrypted_text, decrypted_len, encrypted_text, encrypted_len) <= 0) handle_errors("EVP_PKEY_decrypt failed.");
    
    decrypted_text[*decrypted_len] = '\0'; // Null-terminate the string

    EVP_PKEY_CTX_free(ctx);
    return decrypted_text;
}

int main() {
    // Note: To compile, you need to link against OpenSSL, e.g., gcc your_file.c -o your_app -lssl -lcrypto
    const char* testCases[] = {
        "This is a secret message.",
        "RSA is a public-key cryptosystem.",
        "Security is important.",
        "Test case 4 with numbers 12345.",
        "Final test case with symbols !@#$%"
    };
    int num_test_cases = sizeof(testCases) / sizeof(testCases[0]);

    printf("--- C RSA Encryption/Decryption ---\n");

    for (int i = 0; i < num_test_cases; ++i) {
        const char* original_message = testCases[i];
        printf("\n--- Test Case %d ---\n", i + 1);
        printf("Original Message: %s\n", original_message);

        EVP_PKEY* pkey = NULL;
        unsigned char* encrypted_message = NULL;
        char* decrypted_message = NULL;
        char* encoded_message = NULL;

        // 1. Generate Key Pair
        pkey = generateRsaKeyPair();

        // 2. Encrypt the message
        size_t encrypted_len;
        encrypted_message = encrypt_data(pkey, original_message, &encrypted_len);
        encoded_message = to_base64(encrypted_message, encrypted_len);
        printf("Encrypted (Base64): %s\n", encoded_message);

        // 3. Decrypt the message
        size_t decrypted_len;
        decrypted_message = decrypt_data(pkey, encrypted_message, encrypted_len, &decrypted_len);
        printf("Decrypted Message: %s\n", decrypted_message);

        if (strcmp(original_message, decrypted_message) != 0) {
            fprintf(stderr, "Error: Decrypted message does not match original.\n");
        }

        // Clean up
        EVP_PKEY_free(pkey);
        free(encrypted_message);
        free(decrypted_message);
        free(encoded_message);
    }

    return 0;
}