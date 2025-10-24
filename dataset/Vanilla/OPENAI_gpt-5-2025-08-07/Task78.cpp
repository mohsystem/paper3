#include <openssl/rsa.h>
#include <openssl/bn.h>
#include <openssl/evp.h>
#include <string>
#include <vector>
#include <cstdint>
#include <iostream>

static void append_u32(std::vector<uint8_t>& buf, uint32_t v) {
    buf.push_back((v >> 24) & 0xFF);
    buf.push_back((v >> 16) & 0xFF);
    buf.push_back((v >> 8) & 0xFF);
    buf.push_back(v & 0xFF);
}

static void append_bytes(std::vector<uint8_t>& buf, const uint8_t* data, size_t len) {
    buf.insert(buf.end(), data, data + len);
}

static void append_string(std::vector<uint8_t>& buf, const std::string& s) {
    append_u32(buf, (uint32_t)s.size());
    append_bytes(buf, reinterpret_cast<const uint8_t*>(s.data()), s.size());
}

static void append_string_bytes(std::vector<uint8_t>& buf, const std::vector<uint8_t>& b) {
    append_u32(buf, (uint32_t)b.size());
    append_bytes(buf, b.data(), b.size());
}

static void append_mpint(std::vector<uint8_t>& buf, const BIGNUM* bn) {
    if (BN_is_zero(bn)) {
        append_u32(buf, 0);
        return;
    }
    int nbytes = BN_num_bytes(bn);
    std::vector<uint8_t> tmp(nbytes);
    BN_bn2bin(bn, tmp.data());
    // ensure positive
    if ((tmp[0] & 0x80) != 0) {
        std::vector<uint8_t> with0;
        with0.reserve(tmp.size() + 1);
        with0.push_back(0x00);
        with0.insert(with0.end(), tmp.begin(), tmp.end());
        append_string_bytes(buf, with0);
    } else {
        append_string_bytes(buf, tmp);
    }
}

static std::string b64_wrap(const std::string& in, size_t width = 70) {
    std::string out;
    for (size_t i = 0; i < in.size(); i += width) {
        size_t n = std::min(width, in.size() - i);
        out.append(in, i, n);
        if (i + n < in.size()) out.push_back('\n');
    }
    return out;
}

std::string generateOpenSSHRSAKey(int bits, const std::string& comment) {
    std::string result;

    RSA* rsa = RSA_new();
    BIGNUM* e = BN_new();
    BN_set_word(e, RSA_F4);
    if (RSA_generate_key_ex(rsa, bits, e, nullptr) != 1) {
        BN_free(e);
        RSA_free(rsa);
        return std::string();
    }

    const BIGNUM *n, *e_bn, *d, *p, *q, *dmp1, *dmq1, *iqmp;
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    RSA_get0_key(rsa, &n, &e_bn, &d);
    RSA_get0_factors(rsa, &p, &q);
    RSA_get0_crt_params(rsa, &dmp1, &dmq1, &iqmp);
#else
    n = rsa->n; e_bn = rsa->e; d = rsa->d; p = rsa->p; q = rsa->q; dmp1 = rsa->dmp1; dmq1 = rsa->dmq1; iqmp = rsa->iqmp;
#endif

    // Build public key blob
    std::vector<uint8_t> pub;
    append_string(pub, "ssh-rsa");
    append_mpint(pub, e_bn);
    append_mpint(pub, n);

    // Build private key blob
    std::vector<uint8_t> priv;
    uint32_t check = (uint32_t)rand() ^ ((uint32_t)rand() << 16);
    append_u32(priv, check);
    append_u32(priv, check);
    append_string(priv, "ssh-rsa");
    append_mpint(priv, n);
    append_mpint(priv, e_bn);
    append_mpint(priv, d);
    append_mpint(priv, iqmp);
    append_mpint(priv, p);
    append_mpint(priv, q);
    append_string(priv, comment);
    // padding to 8
    size_t block = 8;
    size_t pad = (block - (priv.size() % block)) % block;
    for (size_t i = 1; i <= pad; ++i) priv.push_back((uint8_t)i);

    // Build full structure
    std::vector<uint8_t> all;
    const char magic[] = "openssh-key-v1";
    append_bytes(all, (const uint8_t*)magic, sizeof(magic) - 1);
    all.push_back(0x00);
    append_string(all, "none");
    append_string(all, "none");
    append_string_bytes(all, std::vector<uint8_t>()); // empty kdfoptions
    append_u32(all, 1);
    append_string_bytes(all, pub);
    append_string_bytes(all, priv);

    // Base64 encode
    int inlen = (int)all.size();
    int outlen = 4 * ((inlen + 2) / 3);
    std::string b64;
    b64.resize(outlen);
    int actual = EVP_EncodeBlock(reinterpret_cast<unsigned char*>(&b64[0]), all.data(), inlen);
    b64.resize(actual);

    std::string wrapped = b64_wrap(b64, 70);
    result.reserve(64 + wrapped.size());
    result += "-----BEGIN OPENSSH PRIVATE KEY-----\n";
    result += wrapped;
    result += "\n-----END OPENSSH PRIVATE KEY-----\n";

    BN_free(e);
    RSA_free(rsa);
    return result;
}

int main() {
    std::cout << generateOpenSSHRSAKey(2048, "cpp-test-1") << std::endl;
    std::cout << generateOpenSSHRSAKey(1024, "cpp-test-2") << std::endl;
    std::cout << generateOpenSSHRSAKey(3072, "cpp-test-3") << std::endl;
    std::cout << generateOpenSSHRSAKey(4096, "cpp-test-4") << std::endl;
    std::cout << generateOpenSSHRSAKey(1536, "cpp-test-5") << std::endl;
    return 0;
}