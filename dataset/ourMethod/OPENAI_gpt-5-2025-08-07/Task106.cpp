#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/bn.h>
#include <openssl/err.h>

#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <cstring>

struct EVP_PKEY_Deleter {
    void operator()(EVP_PKEY* p) const noexcept { if (p) EVP_PKEY_free(p); }
};
struct EVP_PKEY_CTX_Deleter {
    void operator()(EVP_PKEY_CTX* p) const noexcept { if (p) EVP_PKEY_CTX_free(p); }
};
struct BIGNUM_Deleter {
    void operator()(BIGNUM* b) const noexcept { if (b) BN_free(b); }
};
struct RSA_Deleter {
    void operator()(RSA* r) const noexcept { if (r) RSA_free(r); }
};

static void fail_if(bool cond, const char* msg) {
    if (cond) {
        std::cerr << msg << "\n";
        std::exit(1);
    }
}

EVP_PKEY* generate_rsa_key(const int bits) {
    if (bits < 2048) {
        return nullptr;
    }
    std::unique_ptr<BIGNUM, BIGNUM_Deleter> e(BN_new());
    if (!e || !BN_set_word(e.get(), RSA_F4)) {
        return nullptr;
    }
    std::unique_ptr<RSA, RSA_Deleter> rsa(RSA_new());
    if (!rsa || RSA_generate_key_ex(rsa.get(), bits, e.get(), nullptr) != 1) {
        return nullptr;
    }
    EVP_PKEY* pkey = EVP_PKEY_new();
    if (!pkey) return nullptr;
    if (EVP_PKEY_assign_RSA(pkey, rsa.release()) != 1) {
        EVP_PKEY_free(pkey);
        return nullptr;
    }
    return pkey; // pkey owns RSA now
}

std::vector<unsigned char> rsa_encrypt(const std::vector<unsigned char>& plaintext, EVP_PKEY* pkey) {
    if (!pkey) throw std::runtime_error("Invalid key");
    std::unique_ptr<EVP_PKEY_CTX, EVP_PKEY_CTX_Deleter> ctx(EVP_PKEY_CTX_new(pkey, nullptr));
    if (!ctx) throw std::runtime_error("CTX alloc failed");
    if (EVP_PKEY_encrypt_init(ctx.get()) != 1) throw std::runtime_error("encrypt_init failed");
    if (EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_PKCS1_OAEP_PADDING) != 1) throw std::runtime_error("set padding failed");
    if (EVP_PKEY_CTX_set_rsa_oaep_md(ctx.get(), EVP_sha256()) != 1) throw std::runtime_error("set oaep md failed");
    if (EVP_PKEY_CTX_set_rsa_mgf1_md(ctx.get(), EVP_sha256()) != 1) throw std::runtime_error("set mgf1 md failed");
    size_t outlen = 0;
    if (EVP_PKEY_encrypt(ctx.get(), nullptr, &outlen, plaintext.data(), plaintext.size()) != 1) {
        throw std::runtime_error("encrypt size failed");
    }
    std::vector<unsigned char> out(outlen);
    if (EVP_PKEY_encrypt(ctx.get(), out.data(), &outlen, plaintext.data(), plaintext.size()) != 1) {
        throw std::runtime_error("encrypt failed");
    }
    out.resize(outlen);
    return out;
}

std::vector<unsigned char> rsa_decrypt(const std::vector<unsigned char>& ciphertext, EVP_PKEY* pkey) {
    if (!pkey) throw std::runtime_error("Invalid key");
    std::unique_ptr<EVP_PKEY_CTX, EVP_PKEY_CTX_Deleter> ctx(EVP_PKEY_CTX_new(pkey, nullptr));
    if (!ctx) throw std::runtime_error("CTX alloc failed");
    if (EVP_PKEY_decrypt_init(ctx.get()) != 1) throw std::runtime_error("decrypt_init failed");
    if (EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_PKCS1_OAEP_PADDING) != 1) throw std::runtime_error("set padding failed");
    if (EVP_PKEY_CTX_set_rsa_oaep_md(ctx.get(), EVP_sha256()) != 1) throw std::runtime_error("set oaep md failed");
    if (EVP_PKEY_CTX_set_rsa_mgf1_md(ctx.get(), EVP_sha256()) != 1) throw std::runtime_error("set mgf1 md failed");
    size_t outlen = 0;
    if (EVP_PKEY_decrypt(ctx.get(), nullptr, &outlen, ciphertext.data(), ciphertext.size()) != 1) {
        throw std::runtime_error("decrypt size failed");
    }
    std::vector<unsigned char> out(outlen);
    if (EVP_PKEY_decrypt(ctx.get(), out.data(), &outlen, ciphertext.data(), ciphertext.size()) != 1) {
        throw std::runtime_error("decrypt failed");
    }
    out.resize(outlen);
    return out;
}

int main() {
    const std::vector<std::string> messages = {
        "Hello, RSA OAEP!",
        "The quick brown fox jumps over the lazy dog.",
        "RSA with OAEP SHA-256 and MGF1.",
        "Data 1234567890!@#$%^&*()",
        "Unicode: こんにちは世界🌐"
    };

    EVP_PKEY* pkey_raw = generate_rsa_key(2048);
    fail_if(!pkey_raw, "Key generation failed");
    std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter> pkey(pkey_raw);

    size_t passed = 0;
    for (size_t i = 0; i < messages.size(); ++i) {
        const std::string& msg = messages[i];
        std::vector<unsigned char> pt(msg.begin(), msg.end());
        std::vector<unsigned char> ct;
        std::vector<unsigned char> dec;
        try {
            ct = rsa_encrypt(pt, pkey.get());
            dec = rsa_decrypt(ct, pkey.get());
        } catch (const std::exception&) {
            std::cout << "Test " << (i + 1) << ": FAIL\n";
            continue;
        }
        bool ok = (pt.size() == dec.size()) && (CRYPTO_memcmp(pt.data(), dec.data(), pt.size()) == 0);
        if (ok) passed++;
        std::cout << "Test " << (i + 1) << ": " << (ok ? "OK" : "FAIL") << " | Ciphertext length=" << ct.size() << "\n";
    }
    std::cout << "Passed " << passed << " of " << messages.size() << " tests.\n";
    return 0;
}