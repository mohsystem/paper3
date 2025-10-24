#include <iostream>
#include <string>
#include <memory>
#include <stdexcept>

// Note: This code requires OpenSSL version 3.0 or newer.
// To compile: g++ your_file_name.cpp -o your_program_name -l crypto
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/bio.h>
#include <openssl/encoder.h>

// Custom deleters for std::unique_ptr to manage OpenSSL resources safely.
struct BIODeleter {
    void operator()(BIO* bio) const { BIO_free_all(bio); }
};
struct EVP_PKEY_CTXDeleter {
    void operator()(EVP_PKEY_CTX* ctx) const { EVP_PKEY_CTX_free(ctx); }
};
struct EVP_PKEYDeleter {
    void operator()(EVP_PKEY* pkey) const { EVP_PKEY_free(pkey); }
};
struct OSSL_ENCODER_CTXDeleter {
    void operator()(OSSL_ENCODER_CTX* ctx) const { OSSL_ENCODER_CTX_free(ctx); }
};

// Type aliases for smart pointers for cleaner code.
using BIO_ptr = std::unique_ptr<BIO, BIODeleter>;
using EVP_PKEY_CTX_ptr = std::unique_ptr<EVP_PKEY_CTX, EVP_PKEY_CTXDeleter>;
using EVP_PKEY_ptr = std::unique_ptr<EVP_PKEY, EVP_PKEYDeleter>;
using OSSL_ENCODER_CTX_ptr = std::unique_ptr<OSSL_ENCODER_CTX, OSSL_ENCODER_CTXDeleter>;

/**
 * Generates an RSA private key and returns it in the OpenSSH format.
 *
 * @param keySize The size of the key in bits (e.g., 2048, 4096).
 * @return A string containing the RSA private key in OpenSSH format.
 * @throws std::runtime_error on failure.
 */
std::string generateOpenSSHPrivateKey(int keySize) {
    // 1. Create a context for the key generation.
    EVP_PKEY_CTX_ptr ctx(EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL));
    if (!ctx) {
        throw std::runtime_error("Failed to create EVP_PKEY_CTX");
    }

    // 2. Initialize the context for key generation.
    if (EVP_PKEY_keygen_init(ctx.get()) <= 0) {
        throw std::runtime_error("Failed to initialize keygen context");
    }

    // 3. Set the RSA key size.
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx.get(), keySize) <= 0) {
        throw std::runtime_error("Failed to set RSA keygen bits");
    }

    // 4. Generate the key.
    EVP_PKEY* pkey_raw = NULL;
    if (EVP_PKEY_keygen(ctx.get(), &pkey_raw) <= 0) {
        throw std::runtime_error("Failed to generate key");
    }
    EVP_PKEY_ptr pkey(pkey_raw);

    // 5. Create a memory BIO to write the key to.
    BIO_ptr bio(BIO_new(BIO_s_mem()));
    if (!bio) {
        throw std::runtime_error("Failed to create memory BIO");
    }

    // 6. Create an encoder context to export the key in OpenSSH format.
    OSSL_ENCODER_CTX_ptr ectx(OSSL_ENCODER_CTX_new_for_pkey(
        pkey.get(),
        OSSL_KEYMGMT_SELECT_PRIVATE_KEY, // Select the private key components.
        "PEM",                           // Output format is PEM.
        "OpenSSH",                       // Specific PEM structure is "OpenSSH".
        NULL                             // No passphrase.
    ));
    if (!ectx) {
        throw std::runtime_error("Failed to create OSSL_ENCODER_CTX. OpenSSL 3.0+ is required.");
    }
    
    // 7. Perform the encoding and write to the BIO.
    if (!OSSL_ENCODER_to_bio(ectx.get(), bio.get())) {
        throw std::runtime_error("Failed to encode key to BIO");
    }
    
    // 8. Read the key from the BIO into a C++ string.
    char* data_ptr = NULL;
    long data_len = BIO_get_mem_data(bio.get(), &data_ptr);
    if (data_len <= 0 || data_ptr == NULL) {
        throw std::runtime_error("Failed to get data from BIO");
    }
    
    return std::string(data_ptr, data_len);
}

class Task78 {
public:
    static void main() {
        std::cout << "--- C++ RSA OpenSSH Key Generation ---" << std::endl;
        std::cout << "Note: Requires OpenSSL 3.0+ and linking with -lcrypto." << std::endl;
        
        for (int i = 1; i <= 5; ++i) {
            std::cout << "\n--- Test Case " << i << " (2048 bits) ---" << std::endl;
            try {
                std::string privateKey = generateOpenSSHPrivateKey(2048);
                std::cout << privateKey;
            } catch (const std::runtime_error& e) {
                std::cerr << "Error: " << e.what() << std::endl;
            }
        }
    }
};

int main() {
    Task78::main();
    return 0;
}