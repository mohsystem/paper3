/*
NOTE: This C++ code requires the OpenSSL library (version 3.0 or newer recommended).
You must have OpenSSL installed on your system.
To compile:
g++ -std=c++17 your_file_name.cpp -o your_program_name -lssl -lcrypto
*/
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/bio.h>
#include <openssl/encoder.h>
#include <openssl/err.h>

/**
 * @brief Generates an RSA private key and exports it in the OpenSSH format.
 *
 * @param key_size The size of the key in bits (e.g., 2048, 4096).
 * @return The RSA private key in OpenSSH PEM format.
 * @throws std::runtime_error if key generation or encoding fails.
 */
std::string generateAndExportRsaKey(int key_size) {
    if (key_size < 2048) {
        std::cerr << "Warning: Key size " << key_size << " is smaller than the recommended 2048 bits." << std::endl;
    }

    EVP_PKEY* pkey = nullptr;
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    if (!ctx) {
        throw std::runtime_error("Failed to create EVP_PKEY_CTX.");
    }

    if (EVP_PKEY_keygen_init(ctx) <= 0 ||
        EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, key_size) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize RSA keygen parameters.");
    }
    
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("Failed to generate RSA key pair.");
    }
    EVP_PKEY_CTX_free(ctx);

    BIO* bio = BIO_new(BIO_s_mem());
    if (!bio) {
        EVP_PKEY_free(pkey);
        throw std::runtime_error("Failed to create memory BIO.");
    }

    // OSSL_ENCODER is the modern way (OpenSSL 3.0+) to export keys
    OSSL_ENCODER_CTX* enc_ctx = OSSL_ENCODER_CTX_new_for_pkey(
        pkey, EVP_PKEY_KEYPAIR, "PEM", "OSSHPRIV", nullptr);

    if (!enc_ctx) {
        BIO_free(bio);
        EVP_PKEY_free(pkey);
        throw std::runtime_error("Failed to create OSSL_ENCODER_CTX. Is OpenSSL 3.0+ installed?");
    }
    
    if (OSSL_ENCODER_to_bio(enc_ctx, bio) <= 0) {
        OSSL_ENCODER_CTX_free(enc_ctx);
        BIO_free(bio);
        EVP_PKEY_free(pkey);
        throw std::runtime_error("Failed to encode key to BIO.");
    }

    char* buffer = nullptr;
    long len = BIO_get_mem_data(bio, &buffer);
    std::string result(buffer, len);

    OSSL_ENCODER_CTX_free(enc_ctx);
    BIO_free(bio);
    EVP_PKEY_free(pkey);

    return result;
}

int main() {
    // 5 Test Cases
    std::vector<int> key_sizes = {2048, 2048, 3072, 4096, 4096};
    for (int i = 0; i < key_sizes.size(); ++i) {
        std::cout << "--- Test Case " << (i + 1) << " (Key Size: " << key_sizes[i] << ") ---" << std::endl;
        try {
            std::string openssh_key = generateAndExportRsaKey(key_sizes[i]);
            std::cout << openssh_key << std::endl;
        } catch (const std::runtime_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            ERR_print_errors_fp(stderr);
        }
    }
    return 0;
}