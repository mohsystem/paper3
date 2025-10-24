#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <memory>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/rsa.h>
#include <openssl/err.h>

namespace crypto {

static const unsigned char MAGIC[4] = {'E', 'N', 'C', '1'};
static const unsigned char VERSION = 1;
static const size_t SALT_LEN = 16;
static const size_t IV_LEN = 12;
static const size_t TAG_LEN = 16;
static const int PBKDF2_ITERS = 210000;

struct EVP_CIPHER_CTX_Deleter {
    void operator()(EVP_CIPHER_CTX* p) const { if (p) EVP_CIPHER_CTX_free(p); }
};
struct EVP_PKEY_Deleter {
    void operator()(EVP_PKEY* p) const { if (p) EVP_PKEY_free(p); }
};
struct EVP_PKEY_CTX_Deleter {
    void operator()(EVP_PKEY_CTX* p) const { if (p) EVP_PKEY_CTX_free(p); }
};

using EVP_CIPHER_CTX_ptr = std::unique_ptr<EVP_CIPHER_CTX, EVP_CIPHER_CTX_Deleter>;
using EVP_PKEY_ptr = std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter>;
using EVP_PKEY_CTX_ptr = std::unique_ptr<EVP_PKEY_CTX, EVP_PKEY_CTX_Deleter>;

static std::vector<unsigned char> secureRandom(size_t len) {
    std::vector<unsigned char> out(len);
    if (RAND_bytes(out.data(), static_cast<int>(len)) != 1) {
        out.clear();
    }
    return out;
}

static std::vector<unsigned char> deriveKey(const std::string& passphrase, const std::vector<unsigned char>& salt, int iterations) {
    std::vector<unsigned char> key(32);
    if (PKCS5_PBKDF2_HMAC(passphrase.data(), static_cast<int>(passphrase.size()), salt.data(), static_cast<int>(salt.size()), iterations, EVP_sha256(), static_cast<int>(key.size()), key.data()) != 1) {
        key.clear();
    }
    return key;
}

static bool ctEqual(const std::vector<unsigned char>& a, const std::vector<unsigned char>& b) {
    if (a.size() != b.size()) return false;
    unsigned char r = 0;
    for (size_t i = 0; i < a.size(); ++i) r |= (a[i] ^ b[i]);
    return r == 0;
}

static std::vector<unsigned char> encrypt(const std::vector<unsigned char>& plaintext, const std::string& passphrase) {
    auto salt = secureRandom(SALT_LEN);
    auto iv = secureRandom(IV_LEN);
    if (salt.empty() || iv.empty()) return {};

    auto key = deriveKey(passphrase, salt, PBKDF2_ITERS);
    if (key.size() != 32) return {};

    EVP_CIPHER_CTX_ptr ctx(EVP_CIPHER_CTX_new());
    if (!ctx) return {};

    std::vector<unsigned char> ciphertext(plaintext.size() + 16); // GCM may add up to block size
    int len = 0, total = 0;
    if (EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) return {};
    if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(iv.size()), nullptr) != 1) return {};
    if (EVP_EncryptInit_ex(ctx.get(), nullptr, nullptr, key.data(), iv.data()) != 1) return {};
    if (!plaintext.empty()) {
        if (EVP_EncryptUpdate(ctx.get(), ciphertext.data(), &len, plaintext.data(), static_cast<int>(plaintext.size())) != 1) return {};
        total = len;
    } else {
        // No plaintext, still need to call EncryptUpdate with 0
        if (EVP_EncryptUpdate(ctx.get(), nullptr, &len, nullptr, 0) != 1) return {};
    }
    if (EVP_EncryptFinal_ex(ctx.get(), ciphertext.data() + total, &len) != 1) return {};
    total += len;
    ciphertext.resize(total);
    std::vector<unsigned char> tag(TAG_LEN);
    if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_GET_TAG, static_cast<int>(TAG_LEN), tag.data()) != 1) return {};

    std::vector<unsigned char> out;
    out.reserve(4 + 1 + SALT_LEN + IV_LEN + ciphertext.size() + TAG_LEN);
    out.insert(out.end(), MAGIC, MAGIC + 4);
    out.push_back(VERSION);
    out.insert(out.end(), salt.begin(), salt.end());
    out.insert(out.end(), iv.begin(), iv.end());
    out.insert(out.end(), ciphertext.begin(), ciphertext.end());
    out.insert(out.end(), tag.begin(), tag.end());

    // Clean key material
    OPENSSL_cleanse(key.data(), key.size());
    return out;
}

static std::vector<unsigned char> decrypt(const std::vector<unsigned char>& data, const std::string& passphrase) {
    if (data.size() < 4 + 1 + SALT_LEN + IV_LEN + TAG_LEN) return {};
    if (CRYPTO_memcmp(data.data(), MAGIC, 4) != 0) return {};
    if (data[4] != VERSION) return {};

    size_t offset = 5;
    std::vector<unsigned char> salt(data.begin() + offset, data.begin() + offset + SALT_LEN);
    offset += SALT_LEN;
    std::vector<unsigned char> iv(data.begin() + offset, data.begin() + offset + IV_LEN);
    offset += IV_LEN;

    if (data.size() < offset + TAG_LEN) return {};
    size_t ct_len = data.size() - offset - TAG_LEN;
    std::vector<unsigned char> ct(data.begin() + offset, data.begin() + offset + ct_len);
    std::vector<unsigned char> tag(data.begin() + offset + ct_len, data.end());

    auto key = deriveKey(passphrase, salt, PBKDF2_ITERS);
    if (key.size() != 32) return {};

    EVP_CIPHER_CTX_ptr ctx(EVP_CIPHER_CTX_new());
    if (!ctx) return {};
    std::vector<unsigned char> plaintext(ct_len ? ct_len : 1); // ensure non-zero length buffer
    int len = 0, total = 0;

    if (EVP_DecryptInit_ex(ctx.get(), EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) { OPENSSL_cleanse(key.data(), key.size()); return {}; }
    if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(iv.size()), nullptr) != 1) { OPENSSL_cleanse(key.data(), key.size()); return {}; }
    if (EVP_DecryptInit_ex(ctx.get(), nullptr, nullptr, key.data(), iv.data()) != 1) { OPENSSL_cleanse(key.data(), key.size()); return {}; }

    if (ct_len > 0) {
        if (EVP_DecryptUpdate(ctx.get(), plaintext.data(), &len, ct.data(), static_cast<int>(ct.size())) != 1) {
            OPENSSL_cleanse(key.data(), key.size());
            return {};
        }
        total = len;
    }

    if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_SET_TAG, static_cast<int>(tag.size()), const_cast<unsigned char*>(tag.data())) != 1) {
        OPENSSL_cleanse(key.data(), key.size());
        return {};
    }
    int ret_final = EVP_DecryptFinal_ex(ctx.get(), plaintext.data() + total, &len);
    if (ret_final != 1) {
        OPENSSL_cleanse(key.data(), key.size());
        return {};
    }
    total += len;
    plaintext.resize(total);

    OPENSSL_cleanse(key.data(), key.size());
    return plaintext;
}

struct KeyPair {
    EVP_PKEY_ptr priv;
    EVP_PKEY_ptr pub;
};

static KeyPair generateRSA(int bits) {
    KeyPair kp;
    EVP_PKEY_CTX_ptr kctx(EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr));
    if (!kctx) return kp;
    if (EVP_PKEY_keygen_init(kctx.get()) != 1) return kp;
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(kctx.get(), bits) != 1) return kp;
    EVP_PKEY* pkey = nullptr;
    if (EVP_PKEY_keygen(kctx.get(), &pkey) != 1) return kp;
    kp.priv.reset(pkey);

    RSA* rsa = EVP_PKEY_get1_RSA(kp.priv.get());
    if (!rsa) { kp.priv.reset(); return kp; }
    RSA* rsa_pub = RSAPublicKey_dup(rsa);
    RSA_free(rsa);
    if (!rsa_pub) { kp.priv.reset(); return kp; }
    EVP_PKEY* pubkey = EVP_PKEY_new();
    if (!pubkey) { RSA_free(rsa_pub); kp.priv.reset(); return kp; }
    if (EVP_PKEY_assign_RSA(pubkey, rsa_pub) != 1) { RSA_free(rsa_pub); EVP_PKEY_free(pubkey); kp.priv.reset(); return kp; }
    kp.pub.reset(pubkey);
    return kp;
}

static std::vector<unsigned char> rsaEncrypt(const std::vector<unsigned char>& plaintext, EVP_PKEY* pub) {
    std::vector<unsigned char> out;
    if (!pub) return out;
    EVP_PKEY_CTX_ptr ctx(EVP_PKEY_CTX_new(pub, nullptr));
    if (!ctx) return out;
    if (EVP_PKEY_encrypt_init(ctx.get()) != 1) return out;
    if (EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_PKCS1_OAEP_PADDING) != 1) return out;
    if (EVP_PKEY_CTX_set_rsa_oaep_md(ctx.get(), EVP_sha256()) != 1) return out;
    if (EVP_PKEY_CTX_set_rsa_mgf1_md(ctx.get(), EVP_sha256()) != 1) return out;

    size_t outlen = 0;
    if (EVP_PKEY_encrypt(ctx.get(), nullptr, &outlen, plaintext.data(), plaintext.size()) != 1) return out;
    out.resize(outlen);
    if (EVP_PKEY_encrypt(ctx.get(), out.data(), &outlen, plaintext.data(), plaintext.size()) != 1) { out.clear(); return out; }
    out.resize(outlen);
    return out;
}

static std::vector<unsigned char> rsaDecrypt(const std::vector<unsigned char>& ciphertext, EVP_PKEY* priv) {
    std::vector<unsigned char> out;
    if (!priv) return out;
    EVP_PKEY_CTX_ptr ctx(EVP_PKEY_CTX_new(priv, nullptr));
    if (!ctx) return out;
    if (EVP_PKEY_decrypt_init(ctx.get()) != 1) return out;
    if (EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_PKCS1_OAEP_PADDING) != 1) return out;
    if (EVP_PKEY_CTX_set_rsa_oaep_md(ctx.get(), EVP_sha256()) != 1) return out;
    if (EVP_PKEY_CTX_set_rsa_mgf1_md(ctx.get(), EVP_sha256()) != 1) return out;

    size_t outlen = 0;
    if (EVP_PKEY_decrypt(ctx.get(), nullptr, &outlen, ciphertext.data(), ciphertext.size()) != 1) return out;
    out.resize(outlen);
    if (EVP_PKEY_decrypt(ctx.get(), out.data(), &outlen, ciphertext.data(), ciphertext.size()) != 1) { out.clear(); return out; }
    out.resize(outlen);
    return out;
}

} // namespace crypto

int main() {
    bool all = true;

    // Test 1: AES-GCM roundtrip
    std::vector<unsigned char> msg1{'S','e','c','r','e','t',' ','m','e','s','s','a','g','e',' ','1'};
    std::string pw1 = "correct horse battery staple";
    std::vector<unsigned char> enc1 = crypto::encrypt(msg1, pw1);
    std::vector<unsigned char> dec1 = crypto::decrypt(enc1, pw1);
    bool t1 = !dec1.empty() && crypto::ctEqual(msg1, dec1);
    std::cout << "Test1 AES roundtrip: " << std::boolalpha << t1 << "\n";
    all &= t1;

    // Test 2: AES wrong passphrase
    std::vector<unsigned char> dec2 = crypto::decrypt(enc1, std::string("wrong pass"));
    bool t2 = dec2.empty();
    std::cout << "Test2 AES wrong passphrase: " << std::boolalpha << t2 << "\n";
    all &= t2;

    // Test 3: AES tamper detection
    std::vector<unsigned char> tampered = enc1;
    size_t payloadStart = 4 + 1 + crypto::SALT_LEN + crypto::IV_LEN;
    if (tampered.size() > payloadStart) {
        tampered[payloadStart] ^= 0x01;
    }
    std::vector<unsigned char> dec3 = crypto::decrypt(tampered, pw1);
    bool t3 = dec3.empty();
    std::cout << "Test3 AES tamper detection: " << std::boolalpha << t3 << "\n";
    all &= t3;

    // Test 4: RSA OAEP roundtrip
    crypto::KeyPair kp = crypto::generateRSA(2048);
    std::vector<unsigned char> msg2{'R','S','A',' ','s','e','c','r','e','t'};
    std::vector<unsigned char> renc = crypto::rsaEncrypt(msg2, kp.pub.get());
    std::vector<unsigned char> rdec = crypto::rsaDecrypt(renc, kp.priv.get());
    bool t4 = !rdec.empty() && crypto::ctEqual(msg2, rdec);
    std::cout << "Test4 RSA OAEP roundtrip: " << std::boolalpha << t4 << "\n";
    all &= t4;

    // Test 5: RSA wrong key fails
    crypto::KeyPair kp2 = crypto::generateRSA(2048);
    std::vector<unsigned char> rdecWrong = crypto::rsaDecrypt(renc, kp2.priv.get());
    bool t5 = rdecWrong.empty();
    std::cout << "Test5 RSA wrong key fails: " << std::boolalpha << t5 << "\n";
    all &= t5;

    std::cout << "All tests passed: " << std::boolalpha << all << "\n";
    return all ? 0 : 1;
}