// Chain-of-Through process:
// 1) Problem: File encryption/decryption via AES-256-GCM; functions with params; 5 tests.
// 2) Security: Use OpenSSL EVP AES-256-GCM, random 96-bit IV, tag verification.
// 3) Secure coding: Check errors, avoid static IVs, write IV||ciphertext||tag.
// 4) Review: Handle edge cases, verify sizes, clean up contexts.
// 5) Output: Final vetted implementation.

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <cstring>
#include <cstdio>
#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>

static std::vector<unsigned char> generateKey(int bits) {
    if (bits != 128 && bits != 192 && bits != 256) {
        throw std::invalid_argument("AES key size must be 128, 192, or 256 bits");
    }
    std::vector<unsigned char> key(bits / 8);
    if (RAND_bytes(key.data(), (int)key.size()) != 1) {
        throw std::runtime_error("RAND_bytes failed");
    }
    return key;
}

static std::vector<unsigned char> readAll(const std::string& path) {
    std::vector<unsigned char> data;
    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) throw std::runtime_error("Failed to open file for reading");
    std::fseek(f, 0, SEEK_END);
    long sz = std::ftell(f);
    if (sz < 0) { std::fclose(f); throw std::runtime_error("ftell failed"); }
    std::fseek(f, 0, SEEK_SET);
    data.resize((size_t)sz);
    if (sz > 0 && std::fread(data.data(), 1, (size_t)sz, f) != (size_t)sz) {
        std::fclose(f);
        throw std::runtime_error("Failed to read file");
    }
    std::fclose(f);
    return data;
}

static void writeAll(const std::string& path, const std::vector<unsigned char>& data) {
    FILE* f = std::fopen(path.c_str(), "wb");
    if (!f) throw std::runtime_error("Failed to open file for writing");
    if (!data.empty() && std::fwrite(data.data(), 1, data.size(), f) != data.size()) {
        std::fclose(f);
        throw std::runtime_error("Failed to write file");
    }
    std::fclose(f);
}

static void encryptFile(const std::vector<unsigned char>& key, const std::string& inPath, const std::string& outPath) {
    if (!(key.size() == 16 || key.size() == 24 || key.size() == 32)) {
        throw std::invalid_argument("Invalid AES key length");
    }
    std::vector<unsigned char> pt = readAll(inPath);

    std::vector<unsigned char> iv(12);
    if (RAND_bytes(iv.data(), (int)iv.size()) != 1) throw std::runtime_error("RAND_bytes failed");

    const EVP_CIPHER* cipher = nullptr;
    if (key.size() == 16) cipher = EVP_aes_128_gcm();
    else if (key.size() == 24) cipher = EVP_aes_192_gcm();
    else cipher = EVP_aes_256_gcm();

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) throw std::runtime_error("EVP_CIPHER_CTX_new failed");

    std::vector<unsigned char> ct(pt.size());
    std::vector<unsigned char> tag(16);
    int len = 0, ct_len = 0;

    try {
        if (EVP_EncryptInit_ex(ctx, cipher, nullptr, nullptr, nullptr) != 1) throw std::runtime_error("EncryptInit failed");
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, (int)iv.size(), nullptr) != 1) throw std::runtime_error("SET_IVLEN failed");
        if (EVP_EncryptInit_ex(ctx, nullptr, nullptr, key.data(), iv.data()) != 1) throw std::runtime_error("EncryptInit key/iv failed");

        if (!pt.empty()) {
            if (EVP_EncryptUpdate(ctx, ct.data(), &len, pt.data(), (int)pt.size()) != 1) throw std::runtime_error("EncryptUpdate failed");
            ct_len = len;
        }

        if (EVP_EncryptFinal_ex(ctx, ct.data() + ct_len, &len) != 1) throw std::runtime_error("EncryptFinal failed");
        ct_len += len;

        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, (int)tag.size(), tag.data()) != 1) throw std::runtime_error("GET_TAG failed");

        std::vector<unsigned char> out;
        out.reserve(iv.size() + ct_len + tag.size());
        out.insert(out.end(), iv.begin(), iv.end());
        out.insert(out.end(), ct.begin(), ct.begin() + ct_len);
        out.insert(out.end(), tag.begin(), tag.end());
        writeAll(outPath, out);
    } catch (...) {
        EVP_CIPHER_CTX_free(ctx);
        throw;
    }
    EVP_CIPHER_CTX_free(ctx);
}

static void decryptFile(const std::vector<unsigned char>& key, const std::string& inPath, const std::string& outPath) {
    if (!(key.size() == 16 || key.size() == 24 || key.size() == 32)) {
        throw std::invalid_argument("Invalid AES key length");
    }
    std::vector<unsigned char> blob = readAll(inPath);
    if (blob.size() < 12 + 16) throw std::runtime_error("Invalid input: too short");
    std::vector<unsigned char> iv(blob.begin(), blob.begin() + 12);
    std::vector<unsigned char> tag(blob.end() - 16, blob.end());
    std::vector<unsigned char> ct(blob.begin() + 12, blob.end() - 16);

    const EVP_CIPHER* cipher = nullptr;
    if (key.size() == 16) cipher = EVP_aes_128_gcm();
    else if (key.size() == 24) cipher = EVP_aes_192_gcm();
    else cipher = EVP_aes_256_gcm();

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) throw std::runtime_error("EVP_CIPHER_CTX_new failed");

    std::vector<unsigned char> pt(ct.size());
    int len = 0, pt_len = 0;

    try {
        if (EVP_DecryptInit_ex(ctx, cipher, nullptr, nullptr, nullptr) != 1) throw std::runtime_error("DecryptInit failed");
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, (int)iv.size(), nullptr) != 1) throw std::runtime_error("SET_IVLEN failed");
        if (EVP_DecryptInit_ex(ctx, nullptr, nullptr, key.data(), iv.data()) != 1) throw std::runtime_error("DecryptInit key/iv failed");

        if (!ct.empty()) {
            if (EVP_DecryptUpdate(ctx, pt.data(), &len, ct.data(), (int)ct.size()) != 1) throw std::runtime_error("DecryptUpdate failed");
            pt_len = len;
        }

        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, (int)tag.size(), tag.data()) != 1) throw std::runtime_error("SET_TAG failed");

        int ret = EVP_DecryptFinal_ex(ctx, pt.data() + pt_len, &len);
        if (ret != 1) throw std::runtime_error("Authentication failed");
        pt_len += len;

        pt.resize((size_t)pt_len);
        writeAll(outPath, pt);
    } catch (...) {
        EVP_CIPHER_CTX_free(ctx);
        throw;
    }
    EVP_CIPHER_CTX_free(ctx);
}

static bool filesEqual(const std::string& p1, const std::string& p2) {
    std::vector<unsigned char> a = readAll(p1);
    std::vector<unsigned char> b = readAll(p2);
    if (a.size() != b.size()) return false;
    unsigned char diff = 0;
    for (size_t i = 0; i < a.size(); ++i) diff |= (a[i] ^ b[i]);
    return diff == 0;
}

static void writeBytes(const std::string& path, const std::vector<unsigned char>& data) {
    writeAll(path, data);
}

int main() {
    try {
        std::vector<unsigned char> key = generateKey(256);
        std::vector<std::vector<unsigned char>> tests(5);
        tests[0] = {};
        tests[1] = std::vector<unsigned char>({'H','e','l','l','o',',',' ','A','E','S','-','G','C','M','!'});
        std::string u8 = "Unicode: \xE3\x81\x93\xE3\x82\x93\xE3\x81\xAB\xE3\x81\xA1\xE3\x81\xAF\xE4\xB8\x96\xE7\x95\x8C\xF0\x9F\x8C\x8D";
        tests[2] = std::vector<unsigned char>(u8.begin(), u8.end());
        tests[3] = std::vector<unsigned char>(1024, 0);
        tests[4] = std::vector<unsigned char>(4096);
        if (RAND_bytes(tests[4].data(), (int)tests[4].size()) != 1) throw std::runtime_error("RAND_bytes failed");

        std::string tmp =
#ifdef _WIN32
            (std::getenv("TEMP") ? std::getenv("TEMP") : ".")
#else
            (std::getenv("TMPDIR") ? std::getenv("TMPDIR") : "/tmp")
#endif
            ;

        bool allOk = true;
        for (int i = 0; i < 5; ++i) {
            std::string in = tmp + "/cpp_in_" + std::to_string(i) + ".bin";
            std::string enc = tmp + "/cpp_enc_" + std::to_string(i) + ".bin";
            std::string out = tmp + "/cpp_out_" + std::to_string(i) + ".bin";
            writeBytes(in, tests[i]);
            try {
                encryptFile(key, in, enc);
                decryptFile(key, enc, out);
                bool eq = filesEqual(in, out);
                allOk &= eq;
            } catch (...) {
                allOk = false;
            }
            std::remove(in.c_str());
            std::remove(enc.c_str());
            std::remove(out.c_str());
        }
        std::cout << (allOk ? "C++ tests passed" : "C++ tests failed") << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}