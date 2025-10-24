#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

// Note: To compile, you need to link against OpenSSL libraries.
// Example: gcc your_source_file.c -o your_program -lssl -lcrypto

void handle_errors() {
    fprintf(stderr, "An OpenSSL error occurred.\n");
    ERR_print_errors_fp(stderr);
    exit(EXIT_FAILURE);
}

void print_hex(const char* label, const unsigned char* data, int len) {
    printf("%s", label);
    for (int i = 0; i < len; i++) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

char* base64_encode(const unsigned char *input, int length) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); // No newlines
    BIO_write(bio, input, length);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);

    // +1 for null terminator
    char* encoded = (char*)malloc(bufferPtr->length + 1);
    if (encoded == NULL) {
        BIO_free_all(bio);
        fprintf(stderr, "Failed to allocate memory for Base64 string.\n");
        exit(EXIT_FAILURE);
    }
    memcpy(encoded, bufferPtr->data, bufferPtr->length);
    encoded[bufferPtr->length] = '\0';

    BIO_free_all(bio);
    return encoded;
}


/**
 * Encrypts plaintext using AES-256-CBC.
 *
 * @param plaintext The data to encrypt.
 * @param plaintext_len The length of the plaintext.
 * @param key The 256-bit (32-byte) encryption key.
 * @param iv The 128-bit (16-byte) initialization vector.
 * @param ciphertext Buffer to store the resulting ciphertext.
 * @return The length of the ciphertext.
 */
int encrypt(const unsigned char *plaintext, int plaintext_len, const unsigned char *key,
            const unsigned char *iv, unsigned char *ciphertext) {
    EVP_CIPHER_CTX *ctx;
    int len = 0;
    int ciphertext_len = 0;

    // Create and initialize the context
    if (!(ctx = EVP_CIPHER_CTX_new())) handle_errors();

    // Initialize the encryption operation
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) handle_errors();

    // Provide the message to be encrypted, and obtain the encrypted output
    if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len)) handle_errors();
    ciphertext_len = len;

    // Finalize the encryption. More ciphertext may be written at this stage
    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) handle_errors();
    ciphertext_len += len;

    // Clean up
    EVP_CIPHER_CTX_free(ctx);

    return ciphertext_len;
}

void run_test_cases() {
    unsigned char key[32];
    
    // Generate a single, reusable key
    if (!RAND_bytes(key, sizeof(key))) handle_errors();
    print_hex("Generated AES Key: ", key, sizeof(key));
    printf("----------------------------------------\n");

    const char* test_cases[] = {
        "This is a secret message.",
        "The quick brown fox jumps over the lazy dog.",
        "AES encryption is secure!",
        "1234567890",
        "Another test case with some special characters !@#$%^&*()"
    };
    int num_test_cases = sizeof(test_cases) / sizeof(test_cases[0]);

    for (int i = 0; i < num_test_cases; ++i) {
        const char* plain_text = test_cases[i];
        int plain_text_len = strlen(plain_text);

        printf("Test Case %d\n", i + 1);
        printf("Plaintext: %s\n", plain_text);

        // Generate a new, random IV for each encryption
        unsigned char iv[16];
        if (!RAND_bytes(iv, sizeof(iv))) handle_errors();
        print_hex("IV: ", iv, sizeof(iv));

        // The output buffer needs to be large enough for the ciphertext and padding
        unsigned char ciphertext[plain_text_len + EVP_MAX_BLOCK_LENGTH];
        int ciphertext_len = encrypt((unsigned char*)plain_text, plain_text_len, key, iv, ciphertext);

        // Prepend IV to ciphertext for storage/transmission
        int encrypted_data_len = sizeof(iv) + ciphertext_len;
        unsigned char *encrypted_data = (unsigned char *)malloc(encrypted_data_len);
        if (encrypted_data == NULL) {
            fprintf(stderr, "Malloc failed\n");
            exit(EXIT_FAILURE);
        }
        memcpy(encrypted_data, iv, sizeof(iv));
        memcpy(encrypted_data + sizeof(iv), ciphertext, ciphertext_len);

        // Base64 encode the result for printing
        char* b64_text = base64_encode(encrypted_data, encrypted_data_len);
        printf("Encrypted (Base64): %s\n", b64_text);

        free(encrypted_data);
        free(b64_text);
        printf("----------------------------------------\n");
    }
}

int main(void) {
    // Load the error strings for good error reporting
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();

    run_test_cases();

    // Clean up
    EVP_cleanup();
    ERR_free_strings();

    return 0;
}