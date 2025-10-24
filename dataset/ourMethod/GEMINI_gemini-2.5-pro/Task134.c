#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/kdf.h>
#include <openssl/err.h>

// Constants
#define AES_KEY_SIZE_BYTES 32 // 256-bit
#define GCM_IV_LENGTH_BYTES 12
#define GCM_TAG_LENGTH_BYTES 16
#define SALT_LENGTH_BYTES 16
#define PBKDF2_ITERATIONS 600000

typedef struct { unsigned char *data; size_t len; } encrypted_data_t;
typedef struct { char *data; size_t len; } decrypted_data_t;

int derive_key(const char *passphrase, const unsigned char *salt, unsigned char *key) {
    if (PKCS5_PBKDF2_HMAC(passphrase, strlen(passphrase), salt, SALT_LENGTH_BYTES,
                          PBKDF2_ITERATIONS, EVP_sha256(), AES_KEY_SIZE_BYTES, key) != 1) {
        return 0;
    }
    return 1;
}

encrypted_data_t* encrypt_data(const char *plaintext, const char *passphrase) {
    unsigned char salt[SALT_LENGTH_BYTES], key[AES_KEY_SIZE_BYTES], iv[GCM_IV_LENGTH_BYTES], tag[GCM_TAG_LENGTH_BYTES];
    EVP_CIPHER_CTX *ctx = NULL;
    int len = 0, ciphertext_len = 0;
    unsigned char *ciphertext = NULL;
    encrypted_data_t *result = NULL;

    if (RAND_bytes(salt, sizeof(salt)) != 1) return NULL;
    if (!derive_key(passphrase, salt, key)) return NULL;
    if (RAND_bytes(iv, sizeof(iv)) != 1) { OPENSSL_cleanse(key, sizeof(key)); return NULL; }
    
    size_t plaintext_len = strlen(plaintext);
    ciphertext = malloc(plaintext_len + 16); // Block size for final
    if (!ciphertext) goto cleanup;

    if (!(ctx = EVP_CIPHER_CTX_new())) goto cleanup;
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key, iv) != 1) goto cleanup;
    if (EVP_EncryptUpdate(ctx, ciphertext, &len, (const unsigned char *)plaintext, plaintext_len) != 1) goto cleanup;
    ciphertext_len = len;
    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) goto cleanup;
    ciphertext_len += len;
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, GCM_TAG_LENGTH_BYTES, tag) != 1) goto cleanup;

    size_t total_len = SALT_LENGTH_BYTES + GCM_IV_LENGTH_BYTES + ciphertext_len + GCM_TAG_LENGTH_BYTES;
    result = malloc(sizeof(encrypted_data_t));
    if (!result) goto cleanup;
    result->data = malloc(total_len);
    if (!result->data) { free(result); result = NULL; goto cleanup; }
    result->len = total_len;

    memcpy(result->data, salt, SALT_LENGTH_BYTES);
    memcpy(result->data + SALT_LENGTH_BYTES, iv, GCM_IV_LENGTH_BYTES);
    memcpy(result->data + SALT_LENGTH_BYTES + GCM_IV_LENGTH_BYTES, ciphertext, ciphertext_len);
    memcpy(result->data + SALT_LENGTH_BYTES + GCM_IV_LENGTH_BYTES + ciphertext_len, tag, GCM_TAG_LENGTH_BYTES);

cleanup:
    if (ctx) EVP_CIPHER_CTX_free(ctx);
    OPENSSL_cleanse(key, sizeof(key));
    if (ciphertext) free(ciphertext);
    return result;
}

decrypted_data_t* decrypt_data(const encrypted_data_t *encrypted, const char *passphrase) {
    if (!encrypted || encrypted->len < SALT_LENGTH_BYTES + GCM_IV_LENGTH_BYTES + GCM_TAG_LENGTH_BYTES) return NULL;

    const unsigned char *salt = encrypted->data;
    const unsigned char *iv = encrypted->data + SALT_LENGTH_BYTES;
    const unsigned char *tag = encrypted->data + (encrypted->len - GCM_TAG_LENGTH_BYTES);
    size_t ciphertext_len = encrypted->len - SALT_LENGTH_BYTES - GCM_IV_LENGTH_BYTES - GCM_TAG_LENGTH_BYTES;
    const unsigned char *ciphertext = encrypted->data + SALT_LENGTH_BYTES + GCM_IV_LENGTH_BYTES;
    unsigned char key[AES_KEY_SIZE_BYTES];
    if (!derive_key(passphrase, salt, key)) return NULL;

    EVP_CIPHER_CTX *ctx = NULL;
    int len = 0, plaintext_len = 0;
    char *plaintext = NULL;
    decrypted_data_t *result = NULL;

    plaintext = malloc(ciphertext_len);
    if (!plaintext) goto cleanup;

    if (!(ctx = EVP_CIPHER_CTX_new())) goto cleanup;
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key, iv) != 1) goto cleanup;
    if (EVP_DecryptUpdate(ctx, (unsigned char*)plaintext, &len, ciphertext, ciphertext_len) != 1) goto cleanup;
    plaintext_len = len;
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, GCM_TAG_LENGTH_BYTES, (void *)tag) != 1) goto cleanup;
    if (EVP_DecryptFinal_ex(ctx, (unsigned char*)plaintext + len, &len) > 0) {
        plaintext_len += len;
        result = malloc(sizeof(decrypted_data_t));
        if (!result) goto cleanup;
        result->data = malloc(plaintext_len + 1);
        if (!result->data) { free(result); result = NULL; goto cleanup; }
        memcpy(result->data, plaintext, plaintext_len);
        result->data[plaintext_len] = '\0';
        result->len = plaintext_len;
    }

cleanup:
    if (ctx) EVP_CIPHER_CTX_free(ctx);
    OPENSSL_cleanse(key, sizeof(key));
    if (plaintext) free(plaintext);
    return result;
}

void print_hex(const char* label, const unsigned char* data, size_t len) {
    printf("%s: ", label);
    if (len == 0) { printf("(empty)\n"); return; }
    size_t display_len = len > 32 ? 32 : len;
    for (size_t i = 0; i < display_len; ++i) printf("%02x", data[i]);
    if (len > 32) printf("...");
    printf("\n");
}

void run_test_c(const char* original_plaintext, const char* passphrase, int test_num) {
    printf("\n--- Test Case %d ---\n", test_num);
    printf("Original:  \"%s\"\n", original_plaintext);
    
    encrypted_data_t* encrypted = encrypt_data(original_plaintext, passphrase);
    if (!encrypted) { printf("Result: FAILURE (Encryption failed)\n"); return; }
    print_hex("Encrypted", encrypted->data, encrypted->len);

    decrypted_data_t* decrypted = decrypt_data(encrypted, passphrase);
    if (decrypted) {
        printf("Decrypted: \"%s\"\n", decrypted->data);
        printf("Result: %s\n", strcmp(original_plaintext, decrypted->data) == 0 ? "SUCCESS" : "FAILURE");
        free(decrypted->data); free(decrypted);
    } else {
        printf("Decrypted: <DECRYPTION FAILED>\n");
        printf("Result: FAILURE (but expected for bad data)\n");
    }
    free(encrypted->data); free(encrypted);
}

int main() {
    printf("--- Cryptographic Key Management Demo ---\n");
    const char *passphrase = "a_very-s3cur3-p@ssphr@se!";
    const char *test_cases[] = { "Hello, World!", "This is a secret message.",
        "Another test case with some special characters: !@#$%^&*()", "Short", "" };
    int num_test_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    for (int i = 0; i < num_test_cases; ++i) run_test_c(test_cases[i], passphrase, i + 1);

    printf("\n--- Failure Test Cases ---\n");
    const char* original_text = "Test for failure cases.";
    encrypted_data_t* encrypted = encrypt_data(original_text, passphrase);
    if (!encrypted) { fprintf(stderr, "Failed to run failure tests.\n"); return 1; }

    printf("\n1. Decrypting with wrong passphrase...\n");
    decrypted_data_t* decrypted_wrong = decrypt_data(encrypted, "wrong-passphrase");
    printf("Result: %s\n", decrypted_wrong == NULL ? "SUCCESS (Decryption failed as expected)" : "FAILURE");
    if (decrypted_wrong) { free(decrypted_wrong->data); free(decrypted_wrong); }

    printf("\n2. Decrypting with tampered ciphertext...\n");
    encrypted->data[encrypted->len - 1] ^= 0x01; // Flip a bit in the tag
    decrypted_data_t* decrypted_tampered = decrypt_data(encrypted, passphrase);
    printf("Result: %s\n", decrypted_tampered == NULL ? "SUCCESS (Decryption failed as expected)" : "FAILURE");
    if (decrypted_tampered) { free(decrypted_tampered->data); free(decrypted_tampered); }

    free(encrypted->data); free(encrypted);
    return 0;
}