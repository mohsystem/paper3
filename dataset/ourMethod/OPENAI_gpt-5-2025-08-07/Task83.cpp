#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/hmac.h>
#include <openssl/kdf.h>
#include <openssl/sha.h>

#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>
#include <limits>

static const unsigned char MAGIC[] = {'E','C','B','C'};
static const unsigned char VERSION = 1;
static const size_t SALT_LEN = 16;
static const size_t IV_LEN = 16;
static const size_t ENC_KEY_LEN = 32; // AES-256
static const size_t MAC_KEY_LEN = 32; // HMAC-SHA256
static const size_t HMAC_LEN = 32;
static const size_t MAX_PLAINTEXT = 1u << 20; // 1 MiB

static std::string to_hex(const std::vector<unsigned char>& data) {
    std::ostringstream oss;
    for (unsigned char b : data) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)b;
    }
    return oss.str();
}

std::vector<unsigned char> encrypt_key_aes_cbc_etm(const std::vector<unsigned char>& key_to_encrypt,
                                                   const std::vector<unsigned char>& master_key) {
    std::vector<unsigned char> empty;

    if (key_to_encrypt.empty() || key_to_encrypt.size() > MAX_PLAINTEXT) {
        return empty;
    }
    if (master_key.size() < 16) {
        return empty;
    }

    unsigned char salt[SALT_LEN];
    unsigned char iv[IV_LEN];
    if (RAND_bytes(salt, SALT_LEN) != 1) return empty;
    if (RAND_bytes(iv, IV_LEN) != 1) return empty;

    unsigned char okm[ENC_KEY_LEN + MAC_KEY_LEN];
    size_t okm_len = sizeof(okm);
    EVP_PKEY_CTX* pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, nullptr);
    if (!pctx) return empty;
    int rc = 1;
    rc &= EVP_PKEY_derive_init(pctx) == 1;
    rc &= EVP_PKEY_CTX_set_hkdf_md(pctx, EVP_sha256()) == 1;
    rc &= EVP_PKEY_CTX_set1_hkdf_salt(pctx, salt, SALT_LEN) == 1;
    rc &= EVP_PKEY_CTX_set1_hkdf_key(pctx, master_key.data(), (int)master_key.size()) == 1;
    const unsigned char info[] = "AES-CBC-ETM";
    rc &= EVP_PKEY_CTX_add1_hkdf_info(pctx, info, sizeof(info) - 1) == 1;
    rc &= EVP_PKEY_derive(pctx, okm, &okm_len) == 1;
    EVP_PKEY_CTX_free(pctx);
    if (!rc || okm_len != sizeof(okm)) return empty;

    unsigned char enc_key[ENC_KEY_LEN];
    unsigned char mac_key[MAC_KEY_LEN];
    std::memcpy(enc_key, okm, ENC_KEY_LEN);
    std::memcpy(mac_key, okm + ENC_KEY_LEN, MAC_KEY_LEN);
    OPENSSL_cleanse(okm, sizeof(okm));

    std::vector<unsigned char> ciphertext;
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        OPENSSL_cleanse(enc_key, sizeof(enc_key));
        OPENSSL_cleanse(mac_key, sizeof(mac_key));
        return empty;
    }
    rc = 1;
    rc &= EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, enc_key, iv) == 1;
    int block = EVP_CIPHER_CTX_block_size(ctx);
    ciphertext.resize((int)key_to_encrypt.size() + block);
    int outlen1 = 0, outlen2 = 0;
    rc &= EVP_EncryptUpdate(ctx, ciphertext.data(), &outlen1, key_to_encrypt.data(), (int)key_to_encrypt.size()) == 1;
    rc &= EVP_EncryptFinal_ex(ctx, ciphertext.data() + outlen1, &outlen2) == 1;
    EVP_CIPHER_CTX_free(ctx);
    if (!rc) {
        OPENSSL_cleanse(enc_key, sizeof(enc_key));
        OPENSSL_cleanse(mac_key, sizeof(mac_key));
        return empty;
    }
    ciphertext.resize(outlen1 + outlen2);
    OPENSSL_cleanse(enc_key, sizeof(enc_key));

    std::vector<unsigned char> mac_input;
    mac_input.reserve(sizeof(MAGIC) + 1 + SALT_LEN + IV_LEN + ciphertext.size());
    mac_input.insert(mac_input.end(), MAGIC, MAGIC + sizeof(MAGIC));
    mac_input.push_back(VERSION);
    mac_input.insert(mac_input.end(), salt, salt + SALT_LEN);
    mac_input.insert(mac_input.end(), iv, iv + IV_LEN);
    mac_input.insert(mac_input.end(), ciphertext.begin(), ciphertext.end());

    unsigned char tag[HMAC_LEN];
    unsigned int tag_len = 0;
    unsigned char* hm = HMAC(EVP_sha256(), mac_key, (int)sizeof(mac_key),
                             mac_input.data(), (int)mac_input.size(),
                             tag, &tag_len);
    OPENSSL_cleanse(mac_key, sizeof(mac_key));
    if (!hm || tag_len != HMAC_LEN) return empty;

    std::vector<unsigned char> out;
    out.reserve(sizeof(MAGIC) + 1 + SALT_LEN + IV_LEN + ciphertext.size() + HMAC_LEN);
    out.insert(out.end(), MAGIC, MAGIC + sizeof(MAGIC));
    out.push_back(VERSION);
    out.insert(out.end(), salt, salt + SALT_LEN);
    out.insert(out.end(), iv, iv + IV_LEN);
    out.insert(out.end(), ciphertext.begin(), ciphertext.end());
    out.insert(out.end(), tag, tag + HMAC_LEN);

    return out;
}

static std::string sha256_hex(const std::vector<unsigned char>& data) {
    unsigned char digest[SHA256_DIGEST_LENGTH];
    SHA256_CTX sctx;
    SHA256_Init(&sctx);
    SHA256_Update(&sctx, data.data(), data.size());
    SHA256_Final(digest, &sctx);
    std::ostringstream oss;
    for (size_t i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)digest[i];
    }
    return oss.str();
}

int main() {
    for (int i = 1; i <= 5; ++i) {
        std::vector<unsigned char> master_key(32);
        if (RAND_bytes(master_key.data(), (int)master_key.size()) != 1) {
            std::cout << "RNG error\n";
            return 1;
        }
        size_t pt_len = 8u * i;
        std::vector<unsigned char> key_to_encrypt(pt_len);
        if (RAND_bytes(key_to_encrypt.data(), (int)key_to_encrypt.size()) != 1) {
            std::cout << "RNG error\n";
            return 1;
        }
        std::vector<unsigned char> enc = encrypt_key_aes_cbc_etm(key_to_encrypt, master_key);
        std::cout << "Test " << i << ": outLen=" << enc.size() << ", sha256=" << sha256_hex(enc) << "\n";
        OPENSSL_cleanse(master_key.data(), master_key.size());
        OPENSSL_cleanse(key_to_encrypt.data(), key_to_encrypt.size());
    }
    return 0;
}