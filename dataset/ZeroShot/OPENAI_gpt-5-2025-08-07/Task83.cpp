#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/hmac.h>

static bool isValidAesKey(const std::vector<unsigned char>& k) {
    return k.size() == 16 || k.size() == 24 || k.size() == 32;
}

static const EVP_CIPHER* cipherForKeySize(size_t ks) {
    if (ks == 16) return EVP_aes_128_cbc();
    if (ks == 24) return EVP_aes_192_cbc();
    if (ks == 32) return EVP_aes_256_cbc();
    return nullptr;
}

static std::vector<unsigned char> hexToBytes(const std::string& s) {
    if (s.size() % 2 != 0) throw std::invalid_argument("Hex string must have even length");
    std::vector<unsigned char> out(s.size() / 2);
    for (size_t i = 0; i < out.size(); i++) {
        unsigned int byte;
        std::stringstream ss;
        ss << std::hex << s.substr(2*i, 2);
        ss >> byte;
        if (ss.fail()) throw std::invalid_argument("Invalid hex");
        out[i] = static_cast<unsigned char>(byte);
    }
    return out;
}

static std::string toHex(const std::vector<unsigned char>& data) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (unsigned char c : data) {
        oss << std::setw(2) << static_cast<int>(c);
    }
    return oss.str();
}

std::vector<unsigned char> encrypt_key(
    const std::vector<unsigned char>& encKey,
    const std::vector<unsigned char>& macKey,
    const std::vector<unsigned char>& keyToEncrypt
) {
    if (!isValidAesKey(encKey)) {
        throw std::invalid_argument("Encryption key must be 16, 24, or 32 bytes.");
    }
    if (macKey.empty()) {
        throw std::invalid_argument("MAC key must not be empty.");
    }

    std::vector<unsigned char> iv(16);
    if (RAND_bytes(iv.data(), static_cast<int>(iv.size())) != 1) {
        throw std::runtime_error("RAND_bytes failed for IV.");
    }

    const EVP_CIPHER* cipher = cipherForKeySize(encKey.size());
    if (!cipher) {
        throw std::runtime_error("Unsupported AES key size.");
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) throw std::runtime_error("EVP_CIPHER_CTX_new failed.");

    std::vector<unsigned char> ciphertext(keyToEncrypt.size() + EVP_CIPHER_block_size(cipher));
    int len = 0, total = 0;

    if (EVP_EncryptInit_ex(ctx, cipher, nullptr, encKey.data(), iv.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptInit_ex failed.");
    }

    if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, keyToEncrypt.data(), static_cast<int>(keyToEncrypt.size())) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptUpdate failed.");
    }
    total = len;

    if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + total, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptFinal_ex failed.");
    }
    total += len;
    ciphertext.resize(total);
    EVP_CIPHER_CTX_free(ctx);

    unsigned int tag_len = 0;
    unsigned char tag[EVP_MAX_MD_SIZE];
    if (!HMAC(EVP_sha256(), macKey.data(), static_cast<int>(macKey.size()),
              iv.data(), static_cast<int>(iv.size()))) {
        throw std::runtime_error("HMAC failed (stage 1).");
    }
    // Compute HMAC over iv||ciphertext
    HMAC_CTX* hctx = HMAC_CTX_new();
    if (!hctx) throw std::runtime_error("HMAC_CTX_new failed.");
    if (HMAC_Init_ex(hctx, macKey.data(), static_cast<int>(macKey.size()), EVP_sha256(), nullptr) != 1 ||
        HMAC_Update(hctx, iv.data(), iv.size()) != 1 ||
        HMAC_Update(hctx, ciphertext.data(), ciphertext.size()) != 1 ||
        HMAC_Final(hctx, tag, &tag_len) != 1) {
        HMAC_CTX_free(hctx);
        throw std::runtime_error("HMAC computation failed.");
    }
    HMAC_CTX_free(hctx);

    std::vector<unsigned char> out;
    out.reserve(iv.size() + ciphertext.size() + tag_len);
    out.insert(out.end(), iv.begin(), iv.end());
    out.insert(out.end(), ciphertext.begin(), ciphertext.end());
    out.insert(out.end(), tag, tag + tag_len);

    return out;
}

int main() {
    OpenSSL_add_all_algorithms();
    std::string encKeyHex = "603deb1015ca71be2b73aef0857d7781f352c073b6108d72d9810a30914dff4";
    std::string macKeyHex = "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f";
    std::vector<unsigned char> encKey = hexToBytes(encKeyHex);
    std::vector<unsigned char> macKey = hexToBytes(macKeyHex);

    std::vector<std::string> tests = {
        "00112233445566778899aabbccddeeff",
        "000102030405060708090a0b0c0d0e0f1011121314151617",
        "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f",
        "deadbeef",
        "aabbccddeeff"
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        std::vector<unsigned char> pt = hexToBytes(tests[i]);
        std::vector<unsigned char> out = encrypt_key(encKey, macKey, pt);
        std::cout << "Test " << (i + 1) << " (iv|ciphertext|tag hex): " << toHex(out) << std::endl;
    }
    return 0;
}