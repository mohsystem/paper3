
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/aes.h>
#include <openssl/sha.h>
#include <openssl/evp.h>

typedef struct {
    unsigned char* data;
    size_t length;
} ByteArray;

void generate_key(const char* secret_key, unsigned char* key) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)secret_key, strlen(secret_key), hash);
    memcpy(key, hash, 16);
}

char* base64_encode(const unsigned char* buffer, size_t length) {
    BIO *bio, *b64;
    BUF_MEM *buffer_ptr;
    
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, buffer, length);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &buffer_ptr);
    
    char* result = (char*)malloc(buffer_ptr->length + 1);
    memcpy(result, buffer_ptr->data, buffer_ptr->length);
    result[buffer_ptr->length] = '\\0';
    
    BIO_free_all(bio);
    return result;
}

ByteArray base64_decode(const char* encoded) {
    BIO *bio, *b64;
    size_t decode_len = strlen(encoded);
    
    unsigned char* buffer = (unsigned char*)malloc(decode_len);
    
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new_mem_buf(encoded, decode_len);
    bio = BIO_push(b64, bio);
    
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    
    int length = BIO_read(bio, buffer, decode_len);
    BIO_free_all(bio);
    
    ByteArray result;
    result.data = buffer;
    result.length = length;
    return result;
}

char* encrypt(const char* plain_text, const char* secret_key) {
    unsigned char key[16];
    generate_key(secret_key, key);
    
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return NULL;
    
    if (EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, key, NULL) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }
    
    size_t plain_len = strlen(plain_text);
    unsigned char* ciphertext = (unsigned char*)malloc(plain_len + AES_BLOCK_SIZE);
    int len = 0, ciphertext_len = 0;
    
    if (EVP_EncryptUpdate(ctx, ciphertext, &len, (unsigned char*)plain_text, plain_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(ciphertext);
        return NULL;
    }
    ciphertext_len = len;
    
    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(ciphertext);
        return NULL;
    }
    ciphertext_len += len;
    
    EVP_CIPHER_CTX_free(ctx);
    
    char* encoded = base64_encode(ciphertext, ciphertext_len);
    free(ciphertext);
    
    return encoded;
}

char* decrypt(const char* encrypted_text, const char* secret_key) {
    unsigned char key[16];
    generate_key(secret_key, key);
    
    ByteArray ciphertext = base64_decode(encrypted_text);
    
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        free(ciphertext.data);
        return NULL;
    }
    
    if (EVP_DecryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, key, NULL) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(ciphertext.data);
        return NULL;
    }
    
    unsigned char* plaintext = (unsigned char*)malloc(ciphertext.length);
    int len = 0, plaintext_len = 0;
    
    if (EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext.data, ciphertext.length) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(ciphertext.data);
        free(plaintext);
        return NULL;
    }
    plaintext_len = len;
    
    if (EVP_DecryptFinal_ex(ctx, plaintext + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(ciphertext.data);
        free(plaintext);
        return NULL;
    }
    plaintext_len += len;
    
    EVP_CIPHER_CTX_free(ctx);
    free(ciphertext.data);
    
    char* result = (char*)malloc(plaintext_len + 1);
    memcpy(result, plaintext, plaintext_len);
    result[plaintext_len] = '\\0';
    free(plaintext);
    
    return result;
}

int main() {
    // Test case 1
    char* encrypted1 = encrypt("Hello World", "mySecretKey123");
    char* decrypted1 = decrypt(encrypted1, "mySecretKey123");
    printf("Test 1:\\n");
    printf("Original: Hello World\\n");
    printf("Encrypted: %s\\n", encrypted1);
    printf("Decrypted: %s\\n\\n", decrypted1);
    free(encrypted1);
    free(decrypted1);
    
    // Test case 2
    char* encrypted2 = encrypt("Secure Programming 2024", "strongPassword!@#");
    char* decrypted2 = decrypt(encrypted2, "strongPassword!@#");
    printf("Test 2:\\n");
    printf("Original: Secure Programming 2024\\n");
    printf("Encrypted: %s\\n", encrypted2);
    printf("Decrypted: %s\\n\\n", decrypted2);
    free(encrypted2);
    free(decrypted2);
    
    // Test case 3
    char* encrypted3 = encrypt("12345 Special!@#$%", "encryptionKey789");
    char* decrypted3 = decrypt(encrypted3, "encryptionKey789");
    printf("Test 3:\\n");
    printf("Original: 12345 Special!@#$%%\\n");
    printf("Encrypted: %s\\n", encrypted3);
    printf("Decrypted: %s\\n\\n", decrypted3);
    free(encrypted3);
    free(decrypted3);
    
    // Test case 4
    char* encrypted4 = encrypt("The quick brown fox jumps over the lazy dog", "testKey2024");
    char* decrypted4 = decrypt(encrypted4, "testKey2024");
    printf("Test 4:\\n");
    printf("Original: The quick brown fox jumps over the lazy dog\\n");
    printf("Encrypted: %s\\n", encrypted4);
    printf("Decrypted: %s\\n\\n", decrypted4);
    free(encrypted4);
    free(decrypted4);
    
    // Test case 5
    char* encrypted5 = encrypt("AES Encryption Test", "anotherSecretKey");
    char* decrypted5 = decrypt(encrypted5, "anotherSecretKey");
    printf("Test 5:\\n");
    printf("Original: AES Encryption Test\\n");
    printf("Encrypted: %s\\n", encrypted5);
    printf("Decrypted: %s\\n", decrypted5);
    free(encrypted5);
    free(decrypted5);
    
    return 0;
}
