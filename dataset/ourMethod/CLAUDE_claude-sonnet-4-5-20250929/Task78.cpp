
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <iostream>
#include <memory>
#include <string>
#include <cstring>

// RAII wrapper for BIO to ensure proper cleanup
struct BIODeleter {
    void operator()(BIO* bio) const {
        if (bio) BIO_free_all(bio);
    }
};

// RAII wrapper for EVP_PKEY to ensure proper cleanup
struct EVPKeyDeleter {
    void operator()(EVP_PKEY* key) const {
        if (key) EVP_PKEY_free(key);
    }
};

// RAII wrapper for EVP_PKEY_CTX to ensure proper cleanup
struct EVPKeyCtxDeleter {
    void operator()(EVP_PKEY_CTX* ctx) const {
        if (ctx) EVP_PKEY_CTX_free(ctx);
    }
};

// Generate RSA key pair and export in OpenSSH format
// Returns empty string on error
std::string generateRSAKeyOpenSSH(int bits = 2048) {
    // Validate key size - must be at least 2048 bits for security
    if (bits < 2048 || bits > 16384) {
        std::cerr << "Invalid key size. Must be between 2048 and 16384 bits." << std::endl;
        return "";
    }

    // Initialize OpenSSL error strings for better error reporting
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();

    // Create key generation context using RSA algorithm
    std::unique_ptr<EVP_PKEY_CTX, EVPKeyCtxDeleter> ctx(
        EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr)
    );
    if (!ctx) {
        std::cerr << "Failed to create key generation context" << std::endl;
        return "";
    }

    // Initialize key generation
    if (EVP_PKEY_keygen_init(ctx.get()) <= 0) {
        std::cerr << "Failed to initialize key generation" << std::endl;
        return "";
    }

    // Set RSA key size
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx.get(), bits) <= 0) {
        std::cerr << "Failed to set key size" << std::endl;
        return "";
    }

    // Generate the key pair using cryptographically secure RNG from OpenSSL
    EVP_PKEY* pkey_raw = nullptr;
    if (EVP_PKEY_keygen(ctx.get(), &pkey_raw) <= 0) {
        std::cerr << "Failed to generate key pair" << std::endl;
        return "";
    }
    std::unique_ptr<EVP_PKEY, EVPKeyDeleter> pkey(pkey_raw);

    // Create memory BIO for writing the key
    std::unique_ptr<BIO, BIODeleter> bio(BIO_new(BIO_s_mem()));
    if (!bio) {
        std::cerr << "Failed to create memory BIO" << std::endl;
        return "";
    }

    // Write private key in OpenSSH format
    // PEM_write_bio_PrivateKey with nullptr cipher writes unencrypted key
    // In production, use passphrase protection with EVP_aes_256_cbc()
    if (PEM_write_bio_PrivateKey(bio.get(), pkey.get(), nullptr, nullptr, 0, nullptr, nullptr) != 1) {
        std::cerr << "Failed to write private key in PEM format" << std::endl;
        return "";
    }

    // Get the key data from BIO
    BUF_MEM* mem = nullptr;
    BIO_get_mem_ptr(bio.get(), &mem);
    if (!mem || !mem->data || mem->length == 0) {
        std::cerr << "Failed to retrieve key data from BIO" << std::endl;
        return "";
    }

    // Validate length is reasonable (max 100KB for a private key)
    if (mem->length > 102400) {
        std::cerr << "Key data exceeds maximum expected size" << std::endl;
        return "";
    }

    // Copy to string - bounds are validated by BIO
    std::string result(mem->data, mem->length);

    // Note: Private key material is in result string
    // In production, ensure this is securely handled and cleared after use
    return result;
}

int main() {
    std::cout << "=== RSA Private Key Generator (OpenSSH Format) ===" << std::endl;
    std::cout << std::endl;

    // Test case 1: Generate 2048-bit RSA key (minimum secure size)
    std::cout << "Test 1: Generating 2048-bit RSA private key..." << std::endl;
    std::string key1 = generateRSAKeyOpenSSH(2048);
    if (!key1.empty()) {
        std::cout << "Success! Key length: " << key1.length() << " bytes" << std::endl;
        std::cout << "Key preview (first 100 chars):" << std::endl;
        std::cout << key1.substr(0, 100) << "..." << std::endl;
    } else {
        std::cout << "Failed to generate key" << std::endl;
    }
    std::cout << std::endl;

    // Test case 2: Generate 3072-bit RSA key
    std::cout << "Test 2: Generating 3072-bit RSA private key..." << std::endl;
    std::string key2 = generateRSAKeyOpenSSH(3072);
    if (!key2.empty()) {
        std::cout << "Success! Key length: " << key2.length() << " bytes" << std::endl;
    } else {
        std::cout << "Failed to generate key" << std::endl;
    }
    std::cout << std::endl;

    // Test case 3: Generate 4096-bit RSA key (high security)
    std::cout << "Test 3: Generating 4096-bit RSA private key..." << std::endl;
    std::string key3 = generateRSAKeyOpenSSH(4096);
    if (!key3.empty()) {
        std::cout << "Success! Key length: " << key3.length() << " bytes" << std::endl;
    } else {
        std::cout << "Failed to generate key" << std::endl;
    }
    std::cout << std::endl;

    // Test case 4: Invalid key size (too small - should fail)
    std::cout << "Test 4: Attempting to generate 1024-bit RSA key (should fail)..." << std::endl;
    std::string key4 = generateRSAKeyOpenSSH(1024);
    if (key4.empty()) {
        std::cout << "Correctly rejected insecure key size" << std::endl;
    } else {
        std::cout << "Warning: Insecure key size was accepted" << std::endl;
    }
    std::cout << std::endl;

    // Test case 5: Invalid key size (too large - should fail)
    std::cout << "Test 5: Attempting to generate 20480-bit RSA key (should fail)..." << std::endl;
    std::string key5 = generateRSAKeyOpenSSH(20480);
    if (key5.empty()) {
        std::cout << "Correctly rejected excessive key size" << std::endl;
    } else {
        std::cout << "Warning: Excessive key size was accepted" << std::endl;
    }

    // Securely clear key material from memory before exit
    // Using explicit_bzero or memset_s would be better but not portable
    volatile char* ptr1 = const_cast<char*>(key1.data());
    for (size_t i = 0; i < key1.length(); ++i) {
        ptr1[i] = 0;
    }
    volatile char* ptr2 = const_cast<char*>(key2.data());
    for (size_t i = 0; i < key2.length(); ++i) {
        ptr2[i] = 0;
    }
    volatile char* ptr3 = const_cast<char*>(key3.data());
    for (size_t i = 0; i < key3.length(); ++i) {
        ptr3[i] = 0;
    }

    return 0;
}
