/*
 * Compile with: gcc Task83.c -o Task83 -lssl -lcrypto
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

const int AES_KEY_SIZE_BYTES = 32; // 256 bits
const int AES_IV_SIZE_BYTES = 16;  // 128 bits for AES block size

// Helper function to print a byte array as a hex string
void print_hex(const char* label, const unsigned char* data, int len) {
    printf("%s", label);
    for (int i = 0; i < len; ++i) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

// Helper function to handle OpenSSL errors and terminate
void handle_openssl_errors() {
    fprintf(stderr, "An OpenSSL error occurred:\n");
    ERR_print_errors_fp(stderr);
    abort(); // Terminate to prevent insecure continuation
}

/**
 * Creates a Cipher using AES-256-CBC, encrypts the given plaintext,
 * and returns the IV prepended to the ciphertext.
 *
 * @param plainText     The plaintext buffer to encrypt.
 * @param plainText_len The length of the plaintext.
 * @param key           The 256-bit (32-byte) key for encryption.
 * @param out_len       A pointer to an integer where the output length will be stored.
 * @return A dynamically allocated buffer containing the IV followed by the ciphertext.
 *         The caller is responsible for freeing this buffer with free().
 */
unsigned char* encrypt(const unsigned char* plainText, int plainText_len, const unsigned char* key, int* out_len) {
    // 1. Generate a cryptographically strong random IV
    unsigned char iv[AES_IV_SIZE_BYTES];
    if (RAND_bytes(iv, sizeof(iv)) != 1) {
        handle_openssl_errors();
    }

    EVP_CIPHER_CTX *ctx;
    int len;
    int ciphertext_len;

    // Allocate buffer for output: IV + ciphertext. Ciphertext can be up to one block larger.
    int max_out_len = AES_IV_SIZE_BYTES + plainText_len + AES_IV_SIZE_BYTES;
    unsigned char *encrypted_data = malloc(max_out_len);
    if (!encrypted_data) {
        perror("malloc failed");
        abort();
    }

    // Copy IV to the beginning of the output buffer
    memcpy(encrypted_data, iv, AES_IV_SIZE_BYTES);
    unsigned char *ciphertext_ptr = encrypted_data + AES_IV_SIZE_BYTES;

    // Create and initialise the context
    if (!(ctx = EVP_CIPHER_CTX_new())) {
        free(encrypted_data);
        handle_openssl_errors();
    }

    // 2. Initialize the encryption operation with AES-256-CBC
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) {
        free(encrypted_data);
        EVP_CIPHER_CTX_free(ctx);
        handle_openssl_errors();
    }

    // 3. Encrypt the data
    if (1 != EVP_EncryptUpdate(ctx, ciphertext_ptr, &len, plainText, plainText_len)) {
        free(encrypted_data);
        EVP_CIPHER_CTX_free(ctx);
        handle_openssl_errors();
    }
    ciphertext_len = len;

    // Finalize the encryption (this handles padding)
    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext_ptr + len, &len)) {
        free(encrypted_data);
        EVP_CIPHER_CTX_free(ctx);
        handle_openssl_errors();
    }
    ciphertext_len += len;
    
    EVP_CIPHER_CTX_free(ctx);
    
    // Set the final total length (IV + ciphertext)
    *out_len = AES_IV_SIZE_BYTES + ciphertext_len;

    return encrypted_data;
}

int main() {
    ERR_load_crypto_strings();

    // Generate a secure AES key
    unsigned char secret_key[AES_KEY_SIZE_BYTES];
    if (RAND_bytes(secret_key, sizeof(secret_key)) != 1) {
        handle_openssl_errors();
    }

    print_hex("Using AES Key (Hex): ", secret_key, sizeof(secret_key));
    printf("--- Running 5 Test Cases ---\n");

    const char* testCases[] = {
        "This is the first test case.",
        "Encrypting another piece of data.",
        "AES in CBC mode is secure with a random IV.",
        "A short string.",
        "A much longer string to test padding and block chaining properly, ensuring everything works as expected for various lengths of input data."
    };
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < numTestCases; ++i) {
        const char* plainText = testCases[i];
        printf("\n--- Test Case %d ---\n", i + 1);
        printf("Plaintext: %s\n", plainText);
        
        int encrypted_len = 0;
        unsigned char* encrypted_data = encrypt((const unsigned char*)plainText, strlen(plainText), secret_key, &encrypted_len);
        
        print_hex("Encrypted (Hex): ", encrypted_data, encrypted_len);
        
        // IMPORTANT: Free the memory allocated by the encrypt function
        free(encrypted_data);
    }
    
    ERR_free_strings();
    EVP_cleanup(); // Deprecated in OpenSSL 1.1.0+, but harmless
    
    return 0;
}