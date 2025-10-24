#include <iostream>
#include <string>
#include <vector>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/crypto.h>

static std::string b64encode(const unsigned char* data, size_t len) {
    size_t out_len = 4 * ((len + 2) / 3);
    std::string out(out_len, '\0');
    int written = EVP_EncodeBlock(reinterpret_cast<unsigned char*>(&out[0]), data, static_cast<int>(len));
    if (written < 0) return std::string();
    // Trim if OpenSSL wrote fewer bytes than expected (shouldn't happen)
    if (static_cast<size_t>(written) != out_len) out.resize(static_cast<size_t>(written));
    return out;
}

std::string encrypt(const std::string& message, const std::string& secret) {
    const int saltLen = 16;
    const int ivLen = 12;
    const int keyLen = 32;
    const int iterations = 200000;
    unsigned char salt[saltLen];
    unsigned char iv[ivLen];
    unsigned char key[keyLen];

    if (RAND_bytes(salt, saltLen) != 1) return std::string();
    if (RAND_bytes(iv, ivLen) != 1) return std::string();

    if (PKCS5_PBKDF2_HMAC(secret.c_str(), static_cast<int>(secret.size()),
                          salt, saltLen, iterations, EVP_sha256(),
                          keyLen, key) != 1) {
        OPENSSL_cleanse(key, keyLen);
        return std::string();
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        OPENSSL_cleanse(key, keyLen);
        return std::string();
    }

    std::string output;
    int outlen = 0, tmplen = 0;
    std::vector<unsigned char> ciphertext(message.size() + 16); // GCM tag up to 16 appended later

    bool ok = true;
    do {
        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) { ok = false; break; }
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, ivLen, nullptr) != 1) { ok = false; break; }
        if (EVP_EncryptInit_ex(ctx, nullptr, nullptr, key, iv) != 1) { ok = false; break; }

        if (EVP_EncryptUpdate(ctx, ciphertext.data(), &outlen,
                              reinterpret_cast<const unsigned char*>(message.data()),
                              static_cast<int>(message.size())) != 1) { ok = false; break; }

        if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + outlen, &tmplen) != 1) { ok = false; break; }
        int ct_len = outlen + tmplen;

        unsigned char tag[16];
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag) != 1) { ok = false; break; }

        // Append tag to ciphertext
        ciphertext.resize(ct_len);
        ciphertext.insert(ciphertext.end(), tag, tag + 16);

        std::string sSalt = b64encode(salt, saltLen);
        std::string sIv = b64encode(iv, ivLen);
        std::string sCt = b64encode(ciphertext.data(), ciphertext.size());
        output = sSalt + ":" + sIv + ":" + sCt;
    } while (false);

    EVP_CIPHER_CTX_free(ctx);
    OPENSSL_cleanse(key, keyLen);

    if (!ok) return std::string();
    return output;
}

int main() {
    std::cout << encrypt("Hello, World!", "S3cr3t!Key") << std::endl;
    std::cout << encrypt("Attack at dawn", "another$trongKey123") << std::endl;
    std::cout << encrypt("1234567890", "password") << std::endl;
    std::cout << encrypt("Unicode ✓ © ☕", "𝒦𝑒𝓎🔒") << std::endl;
    std::cout << encrypt("", "empty-message-key") << std::endl;
    return 0;
}