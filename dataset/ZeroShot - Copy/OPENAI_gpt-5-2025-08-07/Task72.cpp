#include <openssl/evp.h>
#include <openssl/err.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <string>

static const unsigned char STATIC_IV[16] = {
    0x00,0x01,0x02,0x03,
    0x04,0x05,0x06,0x07,
    0x08,0x09,0x0A,0x0B,
    0x0C,0x0D,0x0E,0x0F
};
static const size_t BLOCK_SIZE = 16;

static std::vector<unsigned char> pkcs7_pad(const std::vector<unsigned char>& data, size_t blockSize) {
    if (blockSize == 0) throw std::runtime_error("Invalid block size");
    size_t padLen = blockSize - (data.size() % blockSize);
    if (padLen == 0) padLen = blockSize;
    std::vector<unsigned char> out(data);
    out.resize(data.size() + padLen, static_cast<unsigned char>(padLen));
    return out;
}

static std::vector<unsigned char> encrypt(const std::vector<unsigned char>& key, const std::vector<unsigned char>& data) {
    if (!(key.size() == 16 || key.size() == 24 || key.size() == 32)) {
        throw std::runtime_error("AES key length must be 16, 24, or 32 bytes");
    }
    const EVP_CIPHER* cipher = nullptr;
    if (key.size() == 16) cipher = EVP_aes_128_cbc();
    else if (key.size() == 24) cipher = EVP_aes_192_cbc();
    else cipher = EVP_aes_256_cbc();

    std::vector<unsigned char> padded = pkcs7_pad(data, BLOCK_SIZE);

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) throw std::runtime_error("EVP_CIPHER_CTX_new failed");

    std::vector<unsigned char> out(padded.size() + BLOCK_SIZE);
    int outlen1 = 0, outlen2 = 0;

    if (EVP_EncryptInit_ex(ctx, cipher, nullptr, key.data(), STATIC_IV) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptInit_ex failed");
    }
    if (EVP_CIPHER_CTX_set_padding(ctx, 0) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Disable padding failed");
    }
    if (EVP_EncryptUpdate(ctx, out.data(), &outlen1, padded.data(), static_cast<int>(padded.size())) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptUpdate failed");
    }
    if (EVP_EncryptFinal_ex(ctx, out.data() + outlen1, &outlen2) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptFinal_ex failed");
    }
    EVP_CIPHER_CTX_free(ctx);
    out.resize(static_cast<size_t>(outlen1 + outlen2));
    return out;
}

static std::string b64encode(const std::vector<unsigned char>& data) {
    static const char* tbl = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    size_t i = 0;
    while (i + 2 < data.size()) {
        unsigned int n = (data[i] << 16) | (data[i+1] << 8) | data[i+2];
        out.push_back(tbl[(n >> 18) & 63]);
        out.push_back(tbl[(n >> 12) & 63]);
        out.push_back(tbl[(n >> 6) & 63]);
        out.push_back(tbl[n & 63]);
        i += 3;
    }
    if (i < data.size()) {
        unsigned int n = data[i] << 16;
        out.push_back(tbl[(n >> 18) & 63]);
        if (i + 1 < data.size()) {
            n |= data[i+1] << 8;
            out.push_back(tbl[(n >> 12) & 63]);
            out.push_back(tbl[(n >> 6) & 63]);
            out.push_back('=');
        } else {
            out.push_back(tbl[(n >> 12) & 63]);
            out.push_back('=');
            out.push_back('=');
        }
    }
    return out;
}

int main() {
    try {
        std::vector<unsigned char> key = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'}; // 16-byte key
        std::vector<std::string> tests = {
            "",
            "Hello, World!",
            "The quick brown fox jumps over the lazy dog",
            "1234567890abcdef",
            "A longer message that will span multiple blocks to test padding and encryption correctness."
        };
        for (size_t i = 0; i < tests.size(); ++i) {
            std::vector<unsigned char> msg(tests[i].begin(), tests[i].end());
            auto ct = encrypt(key, msg);
            std::cout << "Test " << (i+1) << ": " << b64encode(ct) << "\n";
        }
    } catch (const std::exception& ex) {
        std::cerr << "Encryption error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}