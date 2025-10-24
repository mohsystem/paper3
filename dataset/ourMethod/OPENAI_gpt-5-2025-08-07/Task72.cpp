#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>

namespace secureenc {

static constexpr size_t SALT_LEN = 16;     // 16-byte salt
static constexpr size_t IV_LEN   = 12;     // 12-byte IV for GCM
static constexpr size_t TAG_LEN  = 16;     // 16-byte tag (128-bit)
static constexpr size_t KEY_LEN  = 32;     // AES-256
static constexpr int    PBKDF2_ITERS = 210000;

static const uint8_t MAGIC[4] = {'E','N','C','1'};
static const uint8_t VERSION = 1;

// Derive a key from passphrase and salt using PBKDF2-HMAC-SHA256
static bool derive_key(const std::string& passphrase, const uint8_t* salt, size_t salt_len, uint8_t* out_key, size_t out_key_len) {
    if (!out_key || out_key_len != KEY_LEN || !salt || salt_len != SALT_LEN) return false;
    int ok = PKCS5_PBKDF2_HMAC(
        passphrase.c_str(),
        static_cast<int>(passphrase.size()),
        salt,
        static_cast<int>(salt_len),
        PBKDF2_ITERS,
        EVP_sha256(),
        static_cast<int>(out_key_len),
        out_key
    );
    return ok == 1;
}

// AES-256-GCM encryption with PBKDF2 key derivation. Output format:
// [magic="ENC1"(4)][version=1(1)][salt(16)][iv(12)][ciphertext(...same len as plaintext...)][tag(16)]
std::vector<uint8_t> encrypt_data(const std::string& passphrase, const std::vector<uint8_t>& plaintext) {
    std::vector<uint8_t> result;
    uint8_t salt[SALT_LEN];
    uint8_t iv[IV_LEN];
    uint8_t key[KEY_LEN];

    if (RAND_bytes(salt, SALT_LEN) != 1) {
        return result;
    }
    if (RAND_bytes(iv, IV_LEN) != 1) {
        return result;
    }
    if (!derive_key(passphrase, salt, SALT_LEN, key, KEY_LEN)) {
        OPENSSL_cleanse(key, KEY_LEN);
        return result;
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        OPENSSL_cleanse(key, KEY_LEN);
        return result;
    }

    int ok = 1;
    ok &= EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) == 1;
    ok &= EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV_LEN, nullptr) == 1;
    ok &= EVP_EncryptInit_ex(ctx, nullptr, nullptr, key, iv) == 1;

    std::vector<uint8_t> ciphertext(plaintext.size());
    int out_len = 0;
    int total_len = 0;
    if (ok && !plaintext.empty()) {
        ok &= EVP_EncryptUpdate(ctx, ciphertext.data(), &out_len, plaintext.data(), static_cast<int>(plaintext.size())) == 1;
        total_len += out_len;
    }

    // Finalize (GCM does not output additional bytes here)
    if (ok) {
        ok &= EVP_EncryptFinal_ex(ctx, nullptr, &out_len) == 1;
    }

    uint8_t tag[TAG_LEN];
    if (ok) {
        ok &= EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_LEN, tag) == 1;
    }

    if (ok) {
        // Build result buffer
        size_t total_size = sizeof(MAGIC) + 1 + SALT_LEN + IV_LEN + ciphertext.size() + TAG_LEN;
        result.resize(total_size);
        size_t off = 0;
        std::memcpy(result.data() + off, MAGIC, sizeof(MAGIC)); off += sizeof(MAGIC);
        result[off++] = VERSION;
        std::memcpy(result.data() + off, salt, SALT_LEN); off += SALT_LEN;
        std::memcpy(result.data() + off, iv, IV_LEN); off += IV_LEN;
        if (!ciphertext.empty()) {
            std::memcpy(result.data() + off, ciphertext.data(), ciphertext.size()); off += ciphertext.size();
        }
        std::memcpy(result.data() + off, tag, TAG_LEN); off += TAG_LEN;
    }

    EVP_CIPHER_CTX_free(ctx);
    OPENSSL_cleanse(key, KEY_LEN);
    return result;
}

// Compute SHA-256 digest of data and return hex string (for non-sensitive test output)
static std::string sha256_hex(const std::vector<uint8_t>& data) {
    uint8_t md[SHA256_DIGEST_LENGTH];
    SHA256_CTX c;
    SHA256_Init(&c);
    if (!data.empty()) {
        SHA256_Update(&c, data.data(), data.size());
    }
    SHA256_Final(md, &c);
    std::ostringstream oss;
    for (size_t i = 0; i < sizeof(md); ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)md[i];
    }
    return oss.str();
}

} // namespace secureenc

int main() {
    using namespace secureenc;

    const char* envPass = std::getenv("TASK72_PASSPHRASE");
    std::string passphrase = envPass ? std::string(envPass) : std::string("passphrase-for-tests-only");

    std::vector<std::vector<uint8_t>> tests;
    tests.emplace_back(std::vector<uint8_t>{}); // empty
    {
        const std::string s = "Hello, world!";
        tests.emplace_back(s.begin(), s.end());
    }
    {
        const std::string s = "The quick brown fox jumps over the lazy dog";
        tests.emplace_back(s.begin(), s.end());
    }
    {
        const std::string s = "Data encryption test vector #4 with different length...";
        tests.emplace_back(s.begin(), s.end());
    }
    {
        const std::string s = "Unicode: \xF0\x9F\x94\x92 \xE2\x9C\x94";
        tests.emplace_back(s.begin(), s.end());
    }

    for (size_t i = 0; i < tests.size(); ++i) {
        auto enc = encrypt_data(passphrase, tests[i]);
        std::cout << "Test " << (i+1) << ": output_len=" << enc.size()
                  << ", sha256=" << sha256_hex(enc) << "\n";
    }
    return 0;
}