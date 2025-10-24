
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

typedef struct {
    EVP_PKEY* keypair;
} Task106;

char* base64_encode(const unsigned char* buffer, size_t length) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, buffer, length);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);

    char* result = (char*)malloc(bufferPtr->length + 1);
    memcpy(result, bufferPtr->data, bufferPtr->length);
    result[bufferPtr->length] = '\\0';

    BIO_free_all(bio);
    return result;
}

unsigned char* base64_decode(const char* encoded_string, size_t* output_length) {
    BIO *bio, *b64;
    size_t decodeLen = strlen(encoded_string);
    unsigned char* buffer = (unsigned char*)malloc(decodeLen);

    bio = BIO_new_mem_buf(encoded_string, -1);
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    *output_length = BIO_read(bio, buffer, decodeLen);
    BIO_free_all(bio);

    return buffer;
}

Task106* task106_create() {
    Task106* rsa = (Task106*)malloc(sizeof(Task106));
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    EVP_PKEY_keygen_init(ctx);
    EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048);
    EVP_PKEY_keygen(ctx, &rsa->keypair);
    EVP_PKEY_CTX_free(ctx);
    return rsa;
}

void task106_destroy(Task106* rsa) {
    if (rsa) {
        if (rsa->keypair) {
            EVP_PKEY_free(rsa->keypair);
        }
        free(rsa);
    }
}

char* task106_encrypt(Task106* rsa, const char* plainText) {
    if (!plainText || strlen(plainText) == 0) {
        return NULL;
    }

    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(rsa->keypair, NULL);
    EVP_PKEY_encrypt_init(ctx);
    EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING);

    size_t outlen;
    EVP_PKEY_encrypt(ctx, NULL, &outlen, 
                    (const unsigned char*)plainText, strlen(plainText));

    unsigned char* encrypted = (unsigned char*)malloc(outlen);
    EVP_PKEY_encrypt(ctx, encrypted, &outlen, 
                    (const unsigned char*)plainText, strlen(plainText));

    char* result = base64_encode(encrypted, outlen);
    free(encrypted);
    EVP_PKEY_CTX_free(ctx);

    return result;
}

char* task106_decrypt(Task106* rsa, const char* encryptedText) {
    if (!encryptedText || strlen(encryptedText) == 0) {
        return NULL;
    }

    size_t decoded_length;
    unsigned char* decoded = base64_decode(encryptedText, &decoded_length);

    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(rsa->keypair, NULL);
    EVP_PKEY_decrypt_init(ctx);
    EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING);

    size_t outlen;
    EVP_PKEY_decrypt(ctx, NULL, &outlen, decoded, decoded_length);

    unsigned char* decrypted = (unsigned char*)malloc(outlen + 1);
    EVP_PKEY_decrypt(ctx, decrypted, &outlen, decoded, decoded_length);
    decrypted[outlen] = '\\0';

    free(decoded);
    EVP_PKEY_CTX_free(ctx);

    return (char*)decrypted;
}

int main() {
    // Test Case 1: Simple text encryption
    Task106* rsa1 = task106_create();
    const char* text1 = "Hello World";
    char* encrypted1 = task106_encrypt(rsa1, text1);
    char* decrypted1 = task106_decrypt(rsa1, encrypted1);
    printf("Test 1:\\n");
    printf("Original: %s\\n", text1);
    printf("Decrypted: %s\\n", decrypted1);
    printf("Match: %s\\n\\n", strcmp(text1, decrypted1) == 0 ? "true" : "false");
    free(encrypted1);
    free(decrypted1);
    task106_destroy(rsa1);

    // Test Case 2: Numbers and special characters
    Task106* rsa2 = task106_create();
    const char* text2 = "12345!@#$%";
    char* encrypted2 = task106_encrypt(rsa2, text2);
    char* decrypted2 = task106_decrypt(rsa2, encrypted2);
    printf("Test 2:\\n");
    printf("Original: %s\\n", text2);
    printf("Decrypted: %s\\n", decrypted2);
    printf("Match: %s\\n\\n", strcmp(text2, decrypted2) == 0 ? "true" : "false");
    free(encrypted2);
    free(decrypted2);
    task106_destroy(rsa2);

    // Test Case 3: Longer text
    Task106* rsa3 = task106_create();
    const char* text3 = "RSA Algorithm Test with longer message";
    char* encrypted3 = task106_encrypt(rsa3, text3);
    char* decrypted3 = task106_decrypt(rsa3, encrypted3);
    printf("Test 3:\\n");
    printf("Original: %s\\n", text3);
    printf("Decrypted: %s\\n", decrypted3);
    printf("Match: %s\\n\\n", strcmp(text3, decrypted3) == 0 ? "true" : "false");
    free(encrypted3);
    free(decrypted3);
    task106_destroy(rsa3);

    // Test Case 4: Basic message
    Task106* rsa4 = task106_create();
    const char* text4 = "Test Message 123";
    char* encrypted4 = task106_encrypt(rsa4, text4);
    char* decrypted4 = task106_decrypt(rsa4, encrypted4);
    printf("Test 4:\\n");
    printf("Original: %s\\n", text4);
    printf("Decrypted: %s\\n", decrypted4);
    printf("Match: %s\\n\\n", strcmp(text4, decrypted4) == 0 ? "true" : "false");
    free(encrypted4);
    free(decrypted4);
    task106_destroy(rsa4);

    // Test Case 5: Single character
    Task106* rsa5 = task106_create();
    const char* text5 = "X";
    char* encrypted5 = task106_encrypt(rsa5, text5);
    char* decrypted5 = task106_decrypt(rsa5, encrypted5);
    printf("Test 5:\\n");
    printf("Original: %s\\n", text5);
    printf("Decrypted: %s\\n", decrypted5);
    printf("Match: %s\\n", strcmp(text5, decrypted5) == 0 ? "true" : "false");
    free(encrypted5);
    free(decrypted5);
    task106_destroy(rsa5);

    return 0;
}
