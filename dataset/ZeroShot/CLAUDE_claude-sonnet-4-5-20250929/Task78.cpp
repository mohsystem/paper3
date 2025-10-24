
#include <iostream>
#include <string>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/err.h>

std::string generateRSAPrivateKeyOpenSSH(int keySize) {
    // Generate RSA key pair
    EVP_PKEY* pkey = EVP_PKEY_new();
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    
    if (!ctx || EVP_PKEY_keygen_init(ctx) <= 0) {
        if (ctx) EVP_PKEY_CTX_free(ctx);
        return "Error initializing key generation";
    }
    
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, keySize) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return "Error setting key size";
    }
    
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return "Error generating key";
    }
    
    // Create BIO for output
    BIO* bio = BIO_new(BIO_s_mem());
    if (!bio) {
        EVP_PKEY_free(pkey);
        EVP_PKEY_CTX_free(ctx);
        return "Error creating BIO";
    }
    
    // Write private key in PEM format (OpenSSH format requires additional processing)
    if (!PEM_write_bio_PrivateKey(bio, pkey, NULL, NULL, 0, NULL, NULL)) {
        BIO_free(bio);
        EVP_PKEY_free(pkey);
        EVP_PKEY_CTX_free(ctx);
        return "Error writing private key";
    }
    
    // Read from BIO
    char* data;
    long len = BIO_get_mem_data(bio, &data);
    std::string result(data, len);
    
    // Cleanup
    BIO_free(bio);
    EVP_PKEY_free(pkey);
    EVP_PKEY_CTX_free(ctx);
    
    return result;
}

int main() {
    std::cout << "Test Case 1: Generate 2048-bit RSA key" << std::endl;
    std::string key1 = generateRSAPrivateKeyOpenSSH(2048);
    std::cout << key1.substr(0, std::min(200, (int)key1.length())) << "...\\n" << std::endl;
    
    std::cout << "Test Case 2: Generate 1024-bit RSA key" << std::endl;
    std::string key2 = generateRSAPrivateKeyOpenSSH(1024);
    std::cout << key2.substr(0, std::min(200, (int)key2.length())) << "...\\n" << std::endl;
    
    std::cout << "Test Case 3: Generate 2048-bit RSA key (second instance)" << std::endl;
    std::string key3 = generateRSAPrivateKeyOpenSSH(2048);
    std::cout << key3.substr(0, std::min(200, (int)key3.length())) << "...\\n" << std::endl;
    
    std::cout << "Test Case 4: Generate 3072-bit RSA key" << std::endl;
    std::string key4 = generateRSAPrivateKeyOpenSSH(3072);
    std::cout << key4.substr(0, std::min(200, (int)key4.length())) << "...\\n" << std::endl;
    
    std::cout << "Test Case 5: Generate 4096-bit RSA key" << std::endl;
    std::string key5 = generateRSAPrivateKeyOpenSSH(4096);
    std::cout << key5.substr(0, std::min(200, (int)key5.length())) << "...\\n" << std::endl;
    
    return 0;
}
