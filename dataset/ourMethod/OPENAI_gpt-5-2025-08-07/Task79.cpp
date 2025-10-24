#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <iomanip>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/crypto.h>

static std::string b64encode(const std::vector<unsigned char>& data) {
    static const char* tbl =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    out.reserve(((data.size() + 2) / 3) * 4);
    size_t i = 0;
    while (i + 3 <= data.size()) {
        unsigned int n = (data[i] << 16) | (data[i + 1] << 8) | data[i + 2];
        out.push_back(tbl[(n >> 18) & 63]);
        out.push_back(tbl[(n >> 12) & 63]);
        out.push_back(tbl[(n >> 6) & 63]);
        out.push_back(tbl[n & 63]);
        i += 3;
    }
    if (i < data.size()) {
        unsigned int n = data[i] << 16;
        if (i + 1 < data.size()) n |= (data[i + 1] << 8);
        out.push_back(tbl[(n >> 18) & 63]);
        out.push_back(tbl[(n >> 12) & 63]);
        if (i + 1 < data.size()) {
            out.push_back(tbl[(n >> 6) & 63]);
            out.push_back('=');
        } else {
            out.push_back('=');
            out.push_back('=');
        }
    }
    return out;
}

// Encrypts plaintext using passphrase-derived AES-256-GCM key and returns blob:
// [magic="ENC1"][version=1][salt(16)][iv(12)][ciphertext][tag(16)]
static std::vector<unsigned char> encrypt(const std::string& passphrase, const std::vector<unsigned char>& plaintext) {
    if (passphrase.empty()) {
        throw std::invalid_argument("Invalid passphrase");
    }
    if (plaintext.size() > 50'000'000) {
        throw std::invalid_argument("Plaintext too large");
    }

    unsigned char salt[16];
    unsigned char iv[12];
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        throw std::runtime_error("RNG failure");
    }
    if (RAND_bytes(iv, sizeof(iv)) != 1) {
        throw std::runtime_error("RNG failure");
    }

    unsigned char key[32];
    const int iterations = 210000;
    if (PKCS5_PBKDF2_HMAC(passphrase.c_str(),
                          static_cast<int>(passphrase.size()),
                          salt, sizeof(salt),
                          iterations, EVP_sha256(),
                          sizeof(key), key) != 1) {
        throw std::runtime_error("KDF failure");
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        OPENSSL_cleanse(key, sizeof(key));
        throw std::runtime_error("CTX alloc failure");
    }

    std::vector<unsigned char> ciphertext(plaintext.size() + 16);
    int outlen = 0, tmplen = 0;

    try {
        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1)
            throw std::runtime_error("EncryptInit 1 failed");

        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, sizeof(iv), nullptr) != 1)
            throw std::runtime_error("Set IV len failed");

        if (EVP_EncryptInit_ex(ctx, nullptr, nullptr, key, iv) != 1)
            throw std::runtime_error("EncryptInit 2 failed");

        if (EVP_EncryptUpdate(ctx, ciphertext.data(), &outlen, plaintext.data(), static_cast<int>(plaintext.size())) != 1)
            throw std::runtime_error("EncryptUpdate failed");

        if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + outlen, &tmplen) != 1)
            throw std::runtime_error("EncryptFinal failed");

        outlen += tmplen;
        ciphertext.resize(outlen);

        unsigned char tag[16];
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, sizeof(tag), tag) != 1)
            throw std::runtime_error("Get tag failed");

        std::vector<unsigned char> out;
        out.reserve(4 + 1 + sizeof(salt) + sizeof(iv) + ciphertext.size() + sizeof(tag));
        out.push_back('E'); out.push_back('N'); out.push_back('C'); out.push_back('1');
        out.push_back(1);
        out.insert(out.end(), salt, salt + sizeof(salt));
        out.insert(out.end(), iv, iv + sizeof(iv));
        out.insert(out.end(), ciphertext.begin(), ciphertext.end());
        out.insert(out.end(), tag, tag + sizeof(tag));

        OPENSSL_cleanse(key, sizeof(key));
        OPENSSL_cleanse(tag, sizeof(tag));
        EVP_CIPHER_CTX_free(ctx);
        return out;
    } catch (...) {
        OPENSSL_cleanse(key, sizeof(key));
        EVP_CIPHER_CTX_free(ctx);
        throw;
    }
}

int main() {
    try {
        std::vector<std::pair<std::string, std::string>> tests = {
            {"correct horse battery staple", "Hello, world!"},
            {"Tr0ub4dor&3", "Security-focused encryption using AES-256-GCM."},
            {"another strong passphrase", "短いメッセージ"},
            {"Yet-Another-Secret", "1234567890"},
            {"SufficientlyLongAndComplexPassphrase!", "The quick brown fox jumps over the lazy dog."}
        };

        for (const auto& t : tests) {
            std::vector<unsigned char> pt(t.second.begin(), t.second.end());
            auto blob = encrypt(t.first, pt);
            std::cout << b64encode(blob) << "\n";
        }
    } catch (...) {
        std::cout << "Encryption failed\n";
    }
    return 0;
}