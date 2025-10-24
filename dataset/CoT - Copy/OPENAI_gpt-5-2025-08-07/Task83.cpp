// Chain-of-Through secure implementation for AES-CBC encryption.
// 1) Problem understanding: Encrypt provided key material using AES-CBC, return Base64(IV||ciphertext).
// 2) Security requirements: Validate AES key size, random IV, standard padding (PKCS7).
// 3) Secure coding generation: Use OpenSSL EVP, check errors, avoid static IV.
// 4) Code review: Proper memory handling, no deprecated/weak primitives.
// 5) Secure code output: Provide 5 tests.

#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <iomanip>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

static const EVP_CIPHER* select_cipher(size_t key_len) {
    switch (key_len) {
        case 16: return EVP_aes_128_cbc();
        case 24: return EVP_aes_192_cbc();
        case 32: return EVP_aes_256_cbc();
        default: return nullptr;
    }
}

static std::string base64_encode(const std::vector<unsigned char>& data) {
    std::string out;
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* mem = BIO_new(BIO_s_mem());
    if (!b64 || !mem) {
        if (b64) BIO_free(b64);
        if (mem) BIO_free(mem);
        throw std::runtime_error("BIO allocation failed");
    }
    b64 = BIO_push(b64, mem);
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    if (BIO_write(b64, data.data(), static_cast<int>(data.size())) <= 0) {
        BIO_free_all(b64);
        throw std::runtime_error("BIO_write failed");
    }
    if (BIO_flush(b64) != 1) {
        BIO_free_all(b64);
        throw std::runtime_error("BIO_flush failed");
    }
    BUF_MEM* mem_ptr = nullptr;
    BIO_get_mem_ptr(b64, &mem_ptr);
    if (!mem_ptr || !mem_ptr->data || mem_ptr->length == 0) {
        BIO_free_all(b64);
        throw std::runtime_error("BIO_get_mem_ptr failed");
    }
    out.assign(mem_ptr->data, mem_ptr->length);
    BIO_free_all(b64);
    return out;
}

std::string encrypt_key(const std::vector<unsigned char>& aes_key, const std::vector<unsigned char>& key_to_encrypt) {
    if (aes_key.empty() || key_to_encrypt.empty()) {
        throw std::invalid_argument("Inputs must not be empty");
    }
    const EVP_CIPHER* cipher_type = select_cipher(aes_key.size());
    if (!cipher_type) throw std::invalid_argument("AES key must be 16, 24, or 32 bytes");

    std::vector<unsigned char> iv(16);
    if (RAND_bytes(iv.data(), static_cast<int>(iv.size())) != 1) {
        throw std::runtime_error("RAND_bytes failed");
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) throw std::runtime_error("EVP_CIPHER_CTX_new failed");

    std::vector<unsigned char> ciphertext(key_to_encrypt.size() + EVP_CIPHER_block_size(cipher_type));
    int len = 0, total_len = 0;

    if (EVP_EncryptInit_ex(ctx, cipher_type, nullptr, aes_key.data(), iv.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptInit_ex failed");
    }

    if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, key_to_encrypt.data(), static_cast<int>(key_to_encrypt.size())) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptUpdate failed");
    }
    total_len = len;

    if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + total_len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptFinal_ex failed");
    }
    total_len += len;
    ciphertext.resize(total_len);
    EVP_CIPHER_CTX_free(ctx);

    std::vector<unsigned char> out;
    out.reserve(iv.size() + ciphertext.size());
    out.insert(out.end(), iv.begin(), iv.end());
    out.insert(out.end(), ciphertext.begin(), ciphertext.end());

    return base64_encode(out);
}

static std::vector<unsigned char> hexToBytes(const std::string& hex) {
    if (hex.size() % 2 != 0) throw std::invalid_argument("Invalid hex length");
    std::vector<unsigned char> out(hex.size() / 2);
    for (size_t i = 0; i < out.size(); ++i) {
        unsigned int val = 0;
        std::stringstream ss;
        ss << std::hex << hex.substr(2 * i, 2);
        ss >> val;
        if (ss.fail()) throw std::invalid_argument("Invalid hex");
        out[i] = static_cast<unsigned char>(val);
    }
    return out;
}

int main() {
    try {
        std::vector<unsigned char> k128 = hexToBytes("00112233445566778899aabbccddeeff");
        std::vector<unsigned char> k192 = hexToBytes("00112233445566778899aabbccddeeff0011223344556677");
        std::vector<unsigned char> k256 = hexToBytes("000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f");

        std::cout << "Test1: " << encrypt_key(k128, std::vector<unsigned char>{'s','e','c','r','e','t','-','k','e','y','-','1'}) << std::endl;
        std::cout << "Test2: " << encrypt_key(k192, std::vector<unsigned char>{'a','n','o','t','h','e','r','-','s','e','c','r','e','t','-','k','e','y','-','2'}) << std::endl;
        std::cout << "Test3: " << encrypt_key(k256, std::vector<unsigned char>{'K','3'}) << std::endl;
        std::cout << "Test4: " << encrypt_key(k128, std::vector<unsigned char>{0x00,0x01,0x02,0x03,0xFF}) << std::endl;
        std::string longData = "A much longer piece of data to encrypt as key material.";
        std::cout << "Test5: " << encrypt_key(k256, std::vector<unsigned char>(longData.begin(), longData.end())) << std::endl;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}