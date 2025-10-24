
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/err.h>

char* generateRSAPrivateKeyOpenSSH(int keySize) {
    EVP_PKEY* pkey = EVP_PKEY_new();
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    
    if (!ctx || EVP_PKEY_keygen_init(ctx) <= 0) {
        if (ctx) EVP_PKEY_CTX_free(ctx);
        return strdup("Error initializing key generation");
    }
    
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, keySize) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return strdup("Error setting key size");
    }
    
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return strdup("Error generating key");
    }
    
    BIO* bio = BIO_new(BIO_s_mem());
    if (!bio) {
        EVP_PKEY_free(pkey);
        EVP_PKEY_CTX_free(ctx);
        return strdup("Error creating BIO");
    }
    
    if (!PEM_write_bio_PrivateKey(bio, pkey, NULL, NULL, 0, NULL, NULL)) {
        BIO_free(bio);
        EVP_PKEY_free(pkey);
        EVP_PKEY_CTX_free(ctx);
        return strdup("Error writing private key");
    }
    
    char* data;
    long len = BIO_get_mem_data(bio, &data);
    char* result = (char*)malloc(len + 1);
    memcpy(result, data, len);
    result[len] = '\\0';
    
    BIO_free(bio);
    EVP_PKEY_free(pkey);
    EVP_PKEY_CTX_free(ctx);
    
    return result;
}

int main() {
    printf("Test Case 1: Generate 2048-bit RSA key\\n");
    char* key1 = generateRSAPrivateKeyOpenSSH(2048);
    printf("%.200s...\\n\\n", key1);
    free(key1);
    
    printf("Test Case 2: Generate 1024-bit RSA key\\n");
    char* key2 = generateRSAPrivateKeyOpenSSH(1024);
    printf("%.200s...\\n\\n", key2);
    free(key2);
    
    printf("Test Case 3: Generate 2048-bit RSA key (second instance)\\n");
    char* key3 = generateRSAPrivateKeyOpenSSH(2048);
    printf("%.200s...\\n\\n", key3);
    free(key3);
    
    printf("Test Case 4: Generate 3072-bit RSA key\\n");
    char* key4 = generateRSAPrivateKeyOpenSSH(3072);
    printf("%.200s...\\n\\n", key4);
    free(key4);
    
    printf("Test Case 5: Generate 4096-bit RSA key\\n");
    char* key5 = generateRSAPrivateKeyOpenSSH(4096);
    printf("%.200s...\\n\\n", key5);
    free(key5);
    
    return 0;
}
