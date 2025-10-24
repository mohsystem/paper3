#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <openssl/evp.h>

static std::vector<unsigned char> hexToBytes(const std::string& s) {
    if (s.size() % 2 != 0) throw std::runtime_error("Invalid hex");
    std::vector<unsigned char> out(s.size() / 2);
    for (size_t i = 0; i < out.size(); ++i) {
        auto hexVal = [](char c)->int {
            if ('0' <= c && c <= '9') return c - '0';
            c = std::tolower(static_cast<unsigned char>(c));
            if ('a' <= c && c <= 'f') return c - 'a' + 10;
            return -1;
        };
        int hi = hexVal(s[2*i]);
        int lo = hexVal(s[2*i+1]);
        if (hi < 0 || lo < 0) throw std::runtime_error("Invalid hex");
        out[i] = static_cast<unsigned char>((hi << 4) | lo);
    }
    return out;
}

static std::string bytesToHex(const std::vector<unsigned char>& data) {
    static const char* hex = "0123456789abcdef";
    std::string out;
    out.resize(data.size() * 2);
    for (size_t i = 0; i < data.size(); ++i) {
        unsigned char v = data[i];
        out[2*i] = hex[v >> 4];
        out[2*i+1] = hex[v & 0x0F];
    }
    return out;
}

std::vector<unsigned char> encryptKeyAesCbc(const std::vector<unsigned char>& keyToEncrypt,
                                            const std::vector<unsigned char>& cipherKey,
                                            const std::vector<unsigned char>& iv) {
    if (!(cipherKey.size() == 16 || cipherKey.size() == 24 || cipherKey.size() == 32)) {
        throw std::runtime_error("cipherKey must be 16, 24, or 32 bytes");
    }
    if (iv.size() != 16) {
        throw std::runtime_error("iv must be 16 bytes");
    }

    const EVP_CIPHER* cipher = nullptr;
    if (cipherKey.size() == 16) cipher = EVP_aes_128_cbc();
    else if (cipherKey.size() == 24) cipher = EVP_aes_192_cbc();
    else if (cipherKey.size() == 32) cipher = EVP_aes_256_cbc();

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) throw std::runtime_error("EVP_CIPHER_CTX_new failed");

    std::vector<unsigned char> out(keyToEncrypt.size() + EVP_CIPHER_block_size(cipher));
    int outlen1 = 0, outlen2 = 0;

    if (EVP_EncryptInit_ex(ctx, cipher, nullptr, cipherKey.data(), iv.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptInit_ex failed");
    }

    if (EVP_EncryptUpdate(ctx, out.data(), &outlen1, keyToEncrypt.data(), static_cast<int>(keyToEncrypt.size())) != 1) {
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

int main() {
    try {
        // Test case 1
        auto key1 = hexToBytes("00112233445566778899aabbccddeeff");
        auto ck1 = hexToBytes("000102030405060708090a0b0c0d0e0f");
        auto iv1 = hexToBytes("101112131415161718191a1b1c1d1e1f");
        auto ct1 = encryptKeyAesCbc(key1, ck1, iv1);
        std::cout << bytesToHex(ct1) << std::endl;

        // Test case 2
        auto key2 = hexToBytes("000102030405060708090a0b0c0d0e0f1011121314151617");
        auto ck2 = hexToBytes("2b7e151628aed2a6abf7158809cf4f3c");
        auto iv2 = hexToBytes("000102030405060708090a0b0c0d0e0f");
        auto ct2 = encryptKeyAesCbc(key2, ck2, iv2);
        std::cout << bytesToHex(ct2) << std::endl;

        // Test case 3
        auto key3 = hexToBytes("000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f");
        auto ck3 = hexToBytes("603deb1015ca71be2b73aef0857d77811f352c073b6108d72d9810a30914dff4");
        auto iv3 = hexToBytes("0f0e0d0c0b0a09080706050403020100");
        auto ct3 = encryptKeyAesCbc(key3, ck3, iv3);
        std::cout << bytesToHex(ct3) << std::endl;

        // Test case 4
        auto key4 = hexToBytes("00112233445566778899aabbccddeeff00112233");
        auto ck4 = hexToBytes("8e73b0f7da0e6452c810f32b809079e562f8ead2522c6b7b");
        auto iv4 = hexToBytes("1f1e1d1c1b1a19181716151413121110");
        auto ct4 = encryptKeyAesCbc(key4, ck4, iv4);
        std::cout << bytesToHex(ct4) << std::endl;

        // Test case 5
        auto key5 = hexToBytes("000102030405060708090a0b0c0d0e");
        auto ck5 = hexToBytes("000102030405060708090a0b0c0d0e0f");
        auto iv5 = hexToBytes("aabbccddeeff00112233445566778899");
        auto ct5 = encryptKeyAesCbc(key5, ck5, iv5);
        std::cout << bytesToHex(ct5) << std::endl;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}