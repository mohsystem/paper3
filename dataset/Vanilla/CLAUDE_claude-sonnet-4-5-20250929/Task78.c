
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/bn.h>

char* generateRSAPrivateKeyOpenSSH(int keySize) {
    RSA* rsa = NULL;
    BIGNUM* bne = NULL;
    BIO* bio = NULL;
    EVP_PKEY* pkey = NULL;
    char* result = NULL;
    
    bne = BN_new();
    if (!bne || BN_set_word(bne, RSA_F4) != 1) {
        if (bne) BN_free(bne);
        return strdup("Error: Failed to set exponent");
    }
    
    rsa = RSA_new();
    if (!rsa || RSA_generate_key_ex(rsa, keySize, bne, NULL) != 1) {
        if (rsa) RSA_free(rsa);
        BN_free(bne);
        return strdup("Error: Failed to generate RSA key");
    }
    
    bio = BIO_new(BIO_s_mem());
    if (!bio) {
        RSA_free(rsa);
        BN_free(bne);
        return strdup("Error: Failed to create BIO");
    }
    
    pkey = EVP_PKEY_new();
    EVP_PKEY_assign_RSA(pkey, rsa);
    
    PEM_write_bio_PrivateKey(bio, pkey, NULL, NULL, 0, NULL, NULL);
    
    char* data = NULL;
    long len = BIO_get_mem_data(bio, &data);
    result = (char*)malloc(len + 1);
    memcpy(result, data, len);
    result[len] = '\\0';
    
    EVP_PKEY_free(pkey);
    BIO_free(bio);
    BN_free(bne);
    
    return result;
}

int main() {
    printf("Test Case 1: 512-bit key\\n");
    char* key1 = generateRSAPrivateKeyOpenSSH(512);
    printf("%.200s...\\n\\n", key1);
    free(key1);
    
    printf("Test Case 2: 1024-bit key\\n");
    char* key2 = generateRSAPrivateKeyOpenSSH(1024);
    printf("%.200s...\\n\\n", key2);
    free(key2);
    
    printf("Test Case 3: 2048-bit key\\n");
    char* key3 = generateRSAPrivateKeyOpenSSH(2048);
    printf("%.200s...\\n\\n", key3);
    free(key3);
    
    printf("Test Case 4: Another 1024-bit key\\n");
    char* key4 = generateRSAPrivateKeyOpenSSH(1024);
    printf("%.200s...\\n\\n", key4);
    free(key4);
    
    printf("Test Case 5: 512-bit key\\n");
    char* key5 = generateRSAPrivateKeyOpenSSH(512);
    printf("%.200s...\\n\\n", key5);
    free(key5);
    
    return 0;
}
