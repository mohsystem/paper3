
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

#define AES_KEY_SIZE 32
#define GCM_IV_SIZE 12
#define GCM_TAG_SIZE 16

static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char* base64_encode(const unsigned char* buffer, size_t length) {
    size_t output_length = 4 * ((length + 2) / 3);
    char* encoded = (char*)malloc(output_length + 1);
    if (!encoded) return NULL;

    size_t i, j;
    for (i = 0, j = 0; i < length;) {
        uint32_t octet_a = i < length ? buffer[i++] : 0;
        uint32_t octet_b = i < length ? buffer[i++] : 0;
        uint32_t octet_c = i < length ? buffer[i++] : 0;
        uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;

        encoded[j++] = base64_chars[(triple >> 18) & 0x3F];
        encoded[j++] = base64_chars[(triple >> 12) & 0x3F];
        encoded[j++] = base64_chars[(triple >> 6) & 0x3F];
        encoded[j++] = base64_chars[triple & 0x3F];
    }

    size_t padding = (3 - length % 3) % 3;
    for (i = 0; i < padding; i++)
        encoded[output_length - 1 - i] = '=';

    encoded[output_length] = '\\0';
    return encoded;
}

unsigned char* base64_decode(const char* encoded, size_t* output_length) {
    size_t input_length = strlen(encoded);
    if (input_length % 4 != 0) return NULL;

    *output_length = input_length / 4 * 3;
    if (encoded[input_length - 1] == '=') (*output_length)--;
    if (encoded[input_length - 2] == '=') (*output_length)--;

    unsigned char* decoded = (unsigned char*)malloc(*output_length);
    if (!decoded) return NULL;

    size_t i, j;
    for (i = 0, j = 0; i < input_length;) {
        uint32_t sextet_a = encoded[i] == '=' ? 0 : strchr(base64_chars, encoded[i]) - base64_chars; i++;
        uint32_t sextet_b = encoded[i] == '=' ? 0 : strchr(base64_chars, encoded[i]) - base64_chars; i++;
        uint32_t sextet_c = encoded[i] == '=' ? 0 : strchr(base64_chars, encoded[i]) - base64_chars; i++;
        uint32_t sextet_d = encoded[i] == '=' ? 0 : strchr(base64_chars, encoded[i]) - base64_chars; i++;

        uint32_t triple = (sextet_a << 18) + (sextet_b << 12) + (sextet_c << 6) + sextet_d;

        if (j < *output_length) decoded[j++] = (triple >> 16) & 0xFF;
        if (j < *output_length) decoded[j++] = (triple >> 8) & 0xFF;
        if (j < *output_length) decoded[j++] = triple & 0xFF;
    }

    return decoded;
}

char* generate_key() {
    unsigned char key[AES_KEY_SIZE];
    if (RAND_bytes(key, AES_KEY_SIZE) != 1) {
        return NULL;
    }
    return base64_encode(key, AES_KEY_SIZE);
}

char* encrypt_string(const char* plaintext, const char* base64_key) {
    if (!plaintext || !base64_key) return NULL;

    size_t key_len;
    unsigned char* key = base64_decode(base64_key, &key_len);
    if (!key || key_len != AES_KEY_SIZE) {
        free(key);
        return NULL;
    }

    unsigned char iv[GCM_IV_SIZE];
    if (RAND_bytes(iv, GCM_IV_SIZE) != 1) {
        free(key);
        return NULL;
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        free(key);
        return NULL;
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(key);
        return NULL;
    }

    int plaintext_len = strlen(plaintext);
    unsigned char* ciphertext = (unsigned char*)malloc(plaintext_len + EVP_CIPHER_block_size(EVP_aes_256_gcm()));
    int len, ciphertext_len;

    if (EVP_EncryptUpdate(ctx, ciphertext, &len, (unsigned char*)plaintext, plaintext_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(key);
        free(ciphertext);
        return NULL;
    }
    ciphertext_len = len;

    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(key);
        free(ciphertext);
        return NULL;
    }
    ciphertext_len += len;

    unsigned char tag[GCM_TAG_SIZE];
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, GCM_TAG_SIZE, tag) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(key);
        free(ciphertext);
        return NULL;
    }

    EVP_CIPHER_CTX_free(ctx);
    free(key);

    size_t result_len = GCM_IV_SIZE + GCM_TAG_SIZE + ciphertext_len;
    unsigned char* result = (unsigned char*)malloc(result_len);
    memcpy(result, iv, GCM_IV_SIZE);
    memcpy(result + GCM_IV_SIZE, tag, GCM_TAG_SIZE);
    memcpy(result + GCM_IV_SIZE + GCM_TAG_SIZE, ciphertext, ciphertext_len);
    free(ciphertext);

    char* encoded = base64_encode(result, result_len);
    free(result);

    return encoded;
}

char* decrypt_string(const char* encrypted_text, const char* base64_key) {
    if (!encrypted_text || !base64_key) return NULL;

    size_t key_len;
    unsigned char* key = base64_decode(base64_key, &key_len);
    if (!key || key_len != AES_KEY_SIZE) {
        free(key);
        return NULL;
    }

    size_t encrypted_len;
    unsigned char* encrypted = base64_decode(encrypted_text, &encrypted_len);
    if (!encrypted || encrypted_len < GCM_IV_SIZE + GCM_TAG_SIZE) {
        free(key);
        free(encrypted);
        return NULL;
    }

    unsigned char* iv = encrypted;
    unsigned char* tag = encrypted + GCM_IV_SIZE;
    unsigned char* ciphertext = encrypted + GCM_IV_SIZE + GCM_TAG_SIZE;
    int ciphertext_len = encrypted_len - GCM_IV_SIZE - GCM_TAG_SIZE;

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        free(key);
        free(encrypted);
        return NULL;
    }

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(key);
        free(encrypted);
        return NULL;
    }

    unsigned char* plaintext = (unsigned char*)malloc(ciphertext_len + EVP_CIPHER_block_size(EVP_aes_256_gcm()) + 1);
    int len, plaintext_len;

    if (EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(key);
        free(encrypted);
        free(plaintext);
        return NULL;
    }
    plaintext_len = len;

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, GCM_TAG_SIZE, tag) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(key);
        free(encrypted);
        free(plaintext);
        return NULL;
    }

    if (EVP_DecryptFinal_ex(ctx, plaintext + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(key);
        free(encrypted);
        free(plaintext);
        return NULL;
    }
    plaintext_len += len;
    plaintext[plaintext_len] = '\\0';

    EVP_CIPHER_CTX_free(ctx);
    free(key);
    free(encrypted);

    return (char*)plaintext;
}

int main() {
    char* key = generate_key();
    if (!key) {
        fprintf(stderr, "Failed to generate key\\n");
        return 1;
    }
    printf("Generated Key: %s\\n\\n", key);

    // Test Case 1
    const char* plaintext1 = "Hello, World!";
    char* encrypted1 = encrypt_string(plaintext1, key);
    char* decrypted1 = decrypt_string(encrypted1, key);
    printf("Test Case 1:\\n");
    printf("Original: %s\\n", plaintext1);
    printf("Encrypted: %s\\n", encrypted1);
    printf("Decrypted: %s\\n", decrypted1);
    printf("Match: %s\\n\\n", strcmp(plaintext1, decrypted1) == 0 ? "true" : "false");
    free(encrypted1);
    free(decrypted1);

    // Test Case 2
    const char* plaintext2 = "Secure Encryption 2024!";
    char* encrypted2 = encrypt_string(plaintext2, key);
    char* decrypted2 = decrypt_string(encrypted2, key);
    printf("Test Case 2:\\n");
    printf("Original: %s\\n", plaintext2);
    printf("Encrypted: %s\\n", encrypted2);
    printf("Decrypted: %s\\n", decrypted2);
    printf("Match: %s\\n\\n", strcmp(plaintext2, decrypted2) == 0 ? "true" : "false");
    free(encrypted2);
    free(decrypted2);

    // Test Case 3
    const char* plaintext3 = "Special characters: @#$%^&*()";
    char* encrypted3 = encrypt_string(plaintext3, key);
    char* decrypted3 = decrypt_string(encrypted3, key);
    printf("Test Case 3:\\n");
    printf("Original: %s\\n", plaintext3);
    printf("Encrypted: %s\\n", encrypted3);
    printf("Decrypted: %s\\n", decrypted3);
    printf("Match: %s\\n\\n", strcmp(plaintext3, decrypted3) == 0 ? "true" : "false");
    free(encrypted3);
    free(decrypted3);

    // Test Case 4
    const char* plaintext4 = "1234567890";
    char* encrypted4 = encrypt_string(plaintext4, key);
    char* decrypted4 = decrypt_string(encrypted4, key);
    printf("Test Case 4:\\n");
    printf("Original: %s\\n", plaintext4);
    printf("Encrypted: %s\\n", encrypted4);
    printf("Decrypted: %s\\n", decrypted4);
    printf("Match: %s\\n\\n", strcmp(plaintext4, decrypted4) == 0 ? "true" : "false");
    free(encrypted4);
    free(decrypted4);

    // Test Case 5
    const char* plaintext5 = "This is a longer text to test encryption with multiple words and sentences.";
    char* encrypted5 = encrypt_string(plaintext5, key);
    char* decrypted5 = decrypt_string(encrypted5, key);
    printf("Test Case 5:\\n");
    printf("Original: %s\\n", plaintext5);
    printf("Encrypted: %s\\n", encrypted5);
    printf("Decrypted: %s\\n", decrypted5);
    printf("Match: %s\\n", strcmp(plaintext5, decrypted5) == 0 ? "true" : "false");
    free(encrypted5);
    free(decrypted5);

    free(key);
    return 0;
}
