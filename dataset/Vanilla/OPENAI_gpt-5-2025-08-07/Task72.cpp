// WARNING: Using a static IV is insecure and only for demonstration/testing.
#include <openssl/evp.h>
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <iomanip>

static const unsigned char STATIC_IV[16] = "0123456789ABCDEF";

std::vector<unsigned char> pad_pkcs7(const std::vector<unsigned char>& data, size_t block_size = 16) {
    size_t padding = block_size - (data.size() % block_size);
    if (padding == 0) padding = block_size;
    std::vector<unsigned char> out = data;
    out.insert(out.end(), padding, static_cast<unsigned char>(padding));
    return out;
}

const EVP_CIPHER* cipher_for_key_size(size_t key_len) {
    switch (key_len) {
        case 16: return EVP_aes_128_cbc();
        case 24: return EVP_aes_192_cbc();
        case 32: return EVP_aes_256_cbc();
        default: return nullptr;
    }
}

std::vector<unsigned char> encrypt(const std::vector<unsigned char>& key, const std::vector<unsigned char>& data) {
    const EVP_CIPHER* cipher = cipher_for_key_size(key.size());
    if (!cipher) throw std::runtime_error("Invalid AES key length. Use 16, 24, or 32 bytes.");

    std::vector<unsigned char> padded = pad_pkcs7(data, 16);

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) throw std::runtime_error("EVP_CIPHER_CTX_new failed");

    std::vector<unsigned char> out(padded.size() + 16);
    int outlen1 = 0, outlen2 = 0;

    if (EVP_EncryptInit_ex(ctx, cipher, nullptr, key.data(), STATIC_IV) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptInit_ex failed");
    }

    // Disable internal padding since we already did PKCS#7 padding
    EVP_CIPHER_CTX_set_padding(ctx, 0);

    if (EVP_EncryptUpdate(ctx, out.data(), &outlen1, padded.data(), static_cast<int>(padded.size())) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptUpdate failed");
    }

    if (EVP_EncryptFinal_ex(ctx, out.data() + outlen1, &outlen2) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptFinal_ex failed");
    }

    out.resize(outlen1 + outlen2);
    EVP_CIPHER_CTX_free(ctx);
    return out;
}

std::string to_hex(const std::vector<unsigned char>& data) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (unsigned char b : data) {
        oss << std::setw(2) << static_cast<int>(b);
    }
    return oss.str();
}

int main() {
    std::vector<unsigned char> key{'T','h','i','s','I','s','A','1','6','B','y','t','e','K','e','y'}; // 16 bytes

    std::vector<std::string> tests = {
        "",
        "Hello, AES-CBC!",
        "The quick brown fox jumps over the lazy dog",
        "1234567890ABCDEF1234",
        "Data with \x00 nulls \x00 inside"
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        std::vector<unsigned char> pt(tests[i].begin(), tests[i].end());
        std::vector<unsigned char> ct = encrypt(key, pt);
        std::cout << "Test " << (i + 1) << " Hex:    " << to_hex(ct) << "\n";
    }
    return 0;
}