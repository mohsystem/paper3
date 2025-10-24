// Requires OpenSSL development libraries
#include <iostream>
#include <vector>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <openssl/evp.h>
#include <openssl/aes.h>

// Static IV - for demonstration only; avoid static IVs in real systems.
static const unsigned char STATIC_IV[16] = {
    0x00, 0x11, 0x22, 0x33,
    0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xAA, 0xBB,
    0xCC, 0xDD, 0xEE, 0xFF
};

static std::vector<unsigned char> pad_pkcs7(const std::vector<unsigned char>& data, size_t blockSize = 16) {
    if (blockSize == 0) throw std::invalid_argument("Invalid block size");
    size_t padLen = blockSize - (data.size() % blockSize);
    if (padLen == 0) padLen = blockSize;
    std::vector<unsigned char> out(data);
    out.insert(out.end(), padLen, static_cast<unsigned char>(padLen));
    return out;
}

static const EVP_CIPHER* select_cipher(size_t key_len) {
    switch (key_len) {
        case 16: return EVP_aes_128_cbc();
        case 24: return EVP_aes_192_cbc();
        case 32: return EVP_aes_256_cbc();
        default: return nullptr;
    }
}

std::vector<unsigned char> encrypt_aes_cbc(const std::vector<unsigned char>& key,
                                           const std::vector<unsigned char>& plaintext) {
    const EVP_CIPHER* cipher = select_cipher(key.size());
    if (!cipher) {
        throw std::invalid_argument("Key length must be 16, 24, or 32 bytes");
    }

    std::vector<unsigned char> padded = pad_pkcs7(plaintext, 16);
    std::vector<unsigned char> out(padded.size() + AES_BLOCK_SIZE);
    int out_len1 = 0, out_len2 = 0;

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) throw std::runtime_error("Failed to create cipher context");

    if (EVP_EncryptInit_ex(ctx, cipher, nullptr, key.data(), STATIC_IV) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EncryptInit failed");
    }

    // Disable internal padding; we do manual PKCS#7 padding
    if (EVP_CIPHER_CTX_set_padding(ctx, 0) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Disable padding failed");
    }

    if (EVP_EncryptUpdate(ctx, out.data(), &out_len1, padded.data(), static_cast<int>(padded.size())) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EncryptUpdate failed");
    }

    if (EVP_EncryptFinal_ex(ctx, out.data() + out_len1, &out_len2) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EncryptFinal failed");
    }

    EVP_CIPHER_CTX_free(ctx);
    out.resize(out_len1 + out_len2);
    return out;
}

static std::string to_hex(const std::vector<unsigned char>& data) {
    std::ostringstream oss;
    for (unsigned char b : data) {
        oss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(b);
    }
    return oss.str();
}

int main() {
    try {
        // Test case 1
        std::vector<unsigned char> key1(16);
        for (size_t i = 0; i < key1.size(); ++i) key1[i] = static_cast<unsigned char>(i);
        std::vector<unsigned char> pt1;
        std::cout << "TC1: " << to_hex(encrypt_aes_cbc(key1, pt1)) << "\n";

        // Test case 2
        std::vector<unsigned char> key2(16, 0x01);
        std::vector<unsigned char> pt2 = {'A'};
        std::cout << "TC2: " << to_hex(encrypt_aes_cbc(key2, pt2)) << "\n";

        // Test case 3
        std::vector<unsigned char> key3 = {'S','i','x','t','e','e','n',' ','b','y','t','e',' ','k','e','y'};
        std::vector<unsigned char> pt3 = {'H','e','l','l','o',' ','W','o','r','l','d'};
        std::cout << "TC3: " << to_hex(encrypt_aes_cbc(key3, pt3)) << "\n";

        // Test case 4 (AES-192)
        std::vector<unsigned char> key4(24);
        for (size_t i = 0; i < key4.size(); ++i) key4[i] = static_cast<unsigned char>(i);
        std::string s4 = "The quick brown fox jumps over the lazy dog";
        std::vector<unsigned char> pt4(s4.begin(), s4.end());
        std::cout << "TC4: " << to_hex(encrypt_aes_cbc(key4, pt4)) << "\n";

        // Test case 5 (AES-256)
        std::vector<unsigned char> key5(32);
        for (size_t i = 0; i < key5.size(); ++i) key5[i] = static_cast<unsigned char>(i);
        std::string s5 = "Data with a length not multiple of block size.";
        std::vector<unsigned char> pt5(s5.begin(), s5.end());
        std::cout << "TC5: " << to_hex(encrypt_aes_cbc(key5, pt5)) << "\n";
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}