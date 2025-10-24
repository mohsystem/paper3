#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <cstring>
#include <vector>
#include <string>
#include <iostream>

// File format: [magic="ENC1"(4 bytes)][version=1(1 byte)][salt(16)][iv(12)][ciphertext(...)] [tag(16)]
static constexpr unsigned char MAGIC[4] = {'E','N','C','1'};
static constexpr unsigned char VERSION = 1;
static constexpr size_t SALT_LEN = 16;
static constexpr size_t IV_LEN = 12;
static constexpr size_t TAG_LEN = 16;
static constexpr int PBKDF2_ITERS = 210000;
static constexpr size_t KEY_LEN = 32;

static void secure_clean(void* ptr, size_t len) {
#if defined(OPENSSL_cleanse)
    OPENSSL_cleanse(ptr, len);
#else
    volatile unsigned char* p = static_cast<volatile unsigned char*>(ptr);
    while (len--) *p++ = 0;
#endif
}

static bool derive_key(const std::string& passphrase, const unsigned char* salt, unsigned char* out_key, size_t out_key_len) {
    if (passphrase.size() > 1024) return false; // basic input limit
    int ok = PKCS5_PBKDF2_HMAC(passphrase.c_str(),
                               static_cast<int>(passphrase.size()),
                               salt,
                               static_cast<int>(SALT_LEN),
                               PBKDF2_ITERS,
                               EVP_sha256(),
                               static_cast<int>(out_key_len),
                               out_key);
    return ok == 1;
}

static std::vector<unsigned char> encrypt_message(const std::string& passphrase, const std::string& plaintext) {
    std::vector<unsigned char> out;
    if (plaintext.size() > (size_t(1) << 30) || passphrase.empty()) {
        return out; // fail closed
    }

    unsigned char salt[SALT_LEN];
    unsigned char iv[IV_LEN];
    unsigned char key[KEY_LEN];
    std::memset(key, 0, sizeof(key));

    if (RAND_bytes(salt, SALT_LEN) != 1 || RAND_bytes(iv, IV_LEN) != 1) {
        secure_clean(key, sizeof(key));
        return out;
    }
    if (!derive_key(passphrase, salt, key, sizeof(key))) {
        secure_clean(key, sizeof(key));
        return out;
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        secure_clean(key, sizeof(key));
        return out;
    }

    int ok = EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr);
    if (ok != 1) { EVP_CIPHER_CTX_free(ctx); secure_clean(key, sizeof(key)); return out; }

    ok = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(IV_LEN), nullptr);
    if (ok != 1) { EVP_CIPHER_CTX_free(ctx); secure_clean(key, sizeof(key)); return out; }

    ok = EVP_EncryptInit_ex(ctx, nullptr, nullptr, key, iv);
    if (ok != 1) { EVP_CIPHER_CTX_free(ctx); secure_clean(key, sizeof(key)); return out; }

    std::vector<unsigned char> ciphertext(plaintext.size());
    int outlen = 0;
    ok = EVP_EncryptUpdate(ctx, ciphertext.data(), &outlen,
                           reinterpret_cast<const unsigned char*>(plaintext.data()),
                           static_cast<int>(plaintext.size()));
    if (ok != 1) { EVP_CIPHER_CTX_free(ctx); secure_clean(key, sizeof(key)); return {}; }
    int total_len = outlen;

    ok = EVP_EncryptFinal_ex(ctx, ciphertext.data() + total_len, &outlen);
    if (ok != 1) { EVP_CIPHER_CTX_free(ctx); secure_clean(key, sizeof(key)); return {}; }
    total_len += outlen;
    ciphertext.resize(static_cast<size_t>(total_len));

    unsigned char tag[TAG_LEN];
    ok = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, static_cast<int>(TAG_LEN), tag);
    if (ok != 1) { EVP_CIPHER_CTX_free(ctx); secure_clean(key, sizeof(key)); return {}; }

    // Build output blob
    out.reserve(4 + 1 + SALT_LEN + IV_LEN + ciphertext.size() + TAG_LEN);
    out.insert(out.end(), MAGIC, MAGIC + 4);
    out.push_back(VERSION);
    out.insert(out.end(), salt, salt + SALT_LEN);
    out.insert(out.end(), iv, iv + IV_LEN);
    out.insert(out.end(), ciphertext.begin(), ciphertext.end());
    out.insert(out.end(), tag, tag + TAG_LEN);

    EVP_CIPHER_CTX_free(ctx);
    secure_clean(key, sizeof(key));
    return out;
}

static std::string decrypt_message(const std::string& passphrase, const std::vector<unsigned char>& blob) {
    std::string plaintext;
    if (blob.size() < 4 + 1 + SALT_LEN + IV_LEN + TAG_LEN || passphrase.empty()) {
        return plaintext; // fail
    }
    if (!(blob[0]==MAGIC[0] && blob[1]==MAGIC[1] && blob[2]==MAGIC[2] && blob[3]==MAGIC[3])) {
        return plaintext;
    }
    if (blob[4] != VERSION) {
        return plaintext;
    }
    size_t offset = 5;
    const unsigned char* salt = blob.data() + offset; offset += SALT_LEN;
    const unsigned char* iv   = blob.data() + offset; offset += IV_LEN;

    if (blob.size() < offset + TAG_LEN) return plaintext;
    size_t ct_len = blob.size() - offset - TAG_LEN;
    const unsigned char* ciphertext = blob.data() + offset;
    const unsigned char* tag = blob.data() + offset + ct_len;

    unsigned char key[KEY_LEN];
    std::memset(key, 0, sizeof(key));
    if (!derive_key(passphrase, salt, key, sizeof(key))) {
        secure_clean(key, sizeof(key));
        return plaintext;
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) { secure_clean(key, sizeof(key)); return plaintext; }

    int ok = EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr);
    if (ok != 1) { EVP_CIPHER_CTX_free(ctx); secure_clean(key, sizeof(key)); return plaintext; }

    ok = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(IV_LEN), nullptr);
    if (ok != 1) { EVP_CIPHER_CTX_free(ctx); secure_clean(key, sizeof(key)); return plaintext; }

    ok = EVP_DecryptInit_ex(ctx, nullptr, nullptr, key, iv);
    if (ok != 1) { EVP_CIPHER_CTX_free(ctx); secure_clean(key, sizeof(key)); return plaintext; }

    std::string out;
    out.resize(ct_len);
    int outlen = 0;
    ok = EVP_DecryptUpdate(ctx, reinterpret_cast<unsigned char*>(&out[0]), &outlen, ciphertext, static_cast<int>(ct_len));
    if (ok != 1) { EVP_CIPHER_CTX_free(ctx); secure_clean(key, sizeof(key)); return std::string(); }
    int total_len = outlen;

    // Set expected tag
    ok = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, static_cast<int>(TAG_LEN), const_cast<unsigned char*>(tag));
    if (ok != 1) { EVP_CIPHER_CTX_free(ctx); secure_clean(key, sizeof(key)); return std::string(); }

    ok = EVP_DecryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(&out[0]) + total_len, &outlen);
    if (ok != 1) { // Authentication failed; do not reveal partial plaintext
        EVP_CIPHER_CTX_free(ctx);
        secure_clean(key, sizeof(key));
        return std::string();
    }
    total_len += outlen;
    out.resize(static_cast<size_t>(total_len));

    EVP_CIPHER_CTX_free(ctx);
    secure_clean(key, sizeof(key));
    return out;
}

static void print_test_result(int idx, bool success, size_t enc_size) {
    std::cout << "Test " << idx << ": " << (success ? "OK" : "FAIL") << " | enc_size=" << enc_size << "\n";
}

int main() {
    // 5 test cases
    struct Case { std::string pass; std::string msg; };
    std::vector<Case> cases;
    cases.push_back({"correct horse battery staple", "Hello, world!"});
    cases.push_back({"p@ssw0rd! with unicode ñ", "The quick brown fox jumps over the lazy dog 12345."});
    std::string longmsg(1000, 'A');
    cases.push_back({"another strong passphrase", longmsg});
    cases.push_back({"another pass", "Short"});
    cases.push_back({"🧪 emojis", "This message contains emojis 😀🚀 and symbols ©®"});

    for (size_t i = 0; i < cases.size(); ++i) {
        auto blob = encrypt_message(cases[i].pass, cases[i].msg);
        bool success = false;
        if (!blob.empty()) {
            auto dec = decrypt_message(cases[i].pass, blob);
            success = (!dec.empty() && dec == cases[i].msg);
        }
        print_test_result(static_cast<int>(i+1), success, blob.size());
    }
    return 0;
}