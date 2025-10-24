#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

// This code requires OpenSSL 3.0+
// To compile: g++ your_file.cpp -o your_app -lssl -lcrypto
#include <openssl/evp.h>
#include <openssl/encoder.h>
#include <openssl/rsa.h>
#include <openssl/bio.h>

/**
 * Generates an RSA private key and exports it using the OpenSSH format.
 *
 * @param keySize The size of the key in bits (e.g., 2048, 4096).
 * @return The RSA private key as a string in OpenSSH PEM format.
 */
std::string generateOpenSSHPrivateKey(int keySize) {
    EVP_PKEY *pkey = nullptr;
    // Use the modern key generation API
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    if (!ctx) {
        throw std::runtime_error("Failed to create EVP_PKEY_CTX");
    }

    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize keygen context");
    }

    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, keySize) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("Failed to set RSA key bits");
    }

    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("Failed to generate key");
    }
    
    EVP_PKEY_CTX_free(ctx); // Context is no longer needed after keygen

    // Create a memory BIO to write the key to
    BIO *bio = BIO_new(BIO_s_mem());
    if (!bio) {
        EVP_PKEY_free(pkey);
        throw std::runtime_error("Failed to create BIO");
    }
    
    // Use the modern OpenSSL 3.0 encoder API
    OSSL_ENCODER_CTX *ectx = OSSL_ENCODER_CTX_new_for_pkey(
        pkey,
        EVP_PKEY_PRIVATE_KEY, // Selection: export the private key
        "PEM",                // Output format: PEM
        "openssh",            // Output structure: OpenSSH private key format
        nullptr               // No properties
    );
    
    if (!ectx) {
        BIO_free(bio);
        EVP_PKEY_free(pkey);
        throw std::runtime_error("Failed to create OSSL_ENCODER_CTX");
    }

    if (OSSL_ENCODER_to_bio(ectx, bio) <= 0) {
        OSSL_ENCODER_CTX_free(ectx);
        BIO_free(bio);
        EVP_PKEY_free(pkey);
        throw std::runtime_error("Failed to encode key to BIO");
    }
    
    // Extract the key string from the BIO
    BUF_MEM *bptr;
    BIO_get_mem_ptr(bio, &bptr);
    std::string key(bptr->data, bptr->length);
    
    // Cleanup
    OSSL_ENCODER_CTX_free(ectx);
    BIO_free_all(bio);
    EVP_PKEY_free(pkey);

    return key;
}

int main() {
    // In C++, the name of the main class is not applicable
    // as it is not an object-oriented requirement for the main entry point.
    // We run the tests directly in main().

    int testKeySizes[] = {1024, 1024, 2048, 2048, 4096};
    for (int i = 0; i < 5; ++i) {
        std::cout << "--- Test Case " << (i + 1) << ": Generating " << testKeySizes[i] << "-bit key ---" << std::endl;
        try {
            if (testKeySizes[i] < 2048) {
                std::cout << "(Note: " << testKeySizes[i] << "-bit RSA is considered weak for production use.)" << std::endl;
            }
            std::string key = generateOpenSSHPrivateKey(testKeySizes[i]);
            std::cout << key << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    return 0;
}