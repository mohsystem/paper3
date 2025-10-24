
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/bn.h>

std::string generateRSAPrivateKeyOpenSSH(int keySize) {
    RSA* rsa = nullptr;
    BIGNUM* bne = nullptr;
    BIO* bio = nullptr;
    
    try {
        bne = BN_new();
        if (!bne || BN_set_word(bne, RSA_F4) != 1) {
            throw std::runtime_error("Failed to set exponent");
        }
        
        rsa = RSA_new();
        if (!rsa || RSA_generate_key_ex(rsa, keySize, bne, nullptr) != 1) {
            throw std::runtime_error("Failed to generate RSA key");
        }
        
        bio = BIO_new(BIO_s_mem());
        if (!bio) {
            throw std::runtime_error("Failed to create BIO");
        }
        
        EVP_PKEY* pkey = EVP_PKEY_new();
        EVP_PKEY_assign_RSA(pkey, rsa);
        
        PEM_write_bio_PrivateKey(bio, pkey, nullptr, nullptr, 0, nullptr, nullptr);
        
        char* data = nullptr;
        long len = BIO_get_mem_data(bio, &data);
        std::string result(data, len);
        
        EVP_PKEY_free(pkey);
        BIO_free(bio);
        BN_free(bne);
        
        return result;
    } catch (const std::exception& e) {
        if (rsa) RSA_free(rsa);
        if (bio) BIO_free(bio);
        if (bne) BN_free(bne);
        return std::string("Error generating key: ") + e.what();
    }
}

int main() {
    std::cout << "Test Case 1: 512-bit key" << std::endl;
    std::string key1 = generateRSAPrivateKeyOpenSSH(512);
    std::cout << key1.substr(0, std::min(200, (int)key1.length())) << "...\\n" << std::endl;
    
    std::cout << "Test Case 2: 1024-bit key" << std::endl;
    std::string key2 = generateRSAPrivateKeyOpenSSH(1024);
    std::cout << key2.substr(0, std::min(200, (int)key2.length())) << "...\\n" << std::endl;
    
    std::cout << "Test Case 3: 2048-bit key" << std::endl;
    std::string key3 = generateRSAPrivateKeyOpenSSH(2048);
    std::cout << key3.substr(0, std::min(200, (int)key3.length())) << "...\\n" << std::endl;
    
    std::cout << "Test Case 4: Another 1024-bit key" << std::endl;
    std::string key4 = generateRSAPrivateKeyOpenSSH(1024);
    std::cout << key4.substr(0, std::min(200, (int)key4.length())) << "...\\n" << std::endl;
    
    std::cout << "Test Case 5: 512-bit key" << std::endl;
    std::string key5 = generateRSAPrivateKeyOpenSSH(512);
    std::cout << key5.substr(0, std::min(200, (int)key5.length())) << "...\\n" << std::endl;
    
    return 0;
}
