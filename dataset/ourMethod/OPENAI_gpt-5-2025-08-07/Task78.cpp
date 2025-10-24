#include <openssl/rsa.h>
#include <openssl/bn.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <cstring>
#include <vector>
#include <string>
#include <iostream>
#include <stdexcept>

static void write_uint32(std::vector<unsigned char>& buf, uint32_t v) {
    buf.push_back((unsigned char)((v >> 24) & 0xFF));
    buf.push_back((unsigned char)((v >> 16) & 0xFF));
    buf.push_back((unsigned char)((v >> 8) & 0xFF));
    buf.push_back((unsigned char)(v & 0xFF));
}

static void write_string(std::vector<unsigned char>& buf, const std::vector<unsigned char>& data) {
    write_uint32(buf, (uint32_t)data.size());
    buf.insert(buf.end(), data.begin(), data.end());
}

static void write_string(std::vector<unsigned char>& buf, const std::string& s) {
    write_uint32(buf, (uint32_t)s.size());
    buf.insert(buf.end(), s.begin(), s.end());
}

static void write_mpint(std::vector<unsigned char>& buf, const BIGNUM* bn) {
    if (BN_is_zero(bn)) {
        write_uint32(buf, 0);
        return;
    }
    int num_bytes = BN_num_bytes(bn);
    std::vector<unsigned char> tmp((size_t)num_bytes);
    BN_bn2bin(bn, tmp.data());

    bool need_lead_zero = (tmp.size() > 0 && (tmp[0] & 0x80) != 0);
    if (need_lead_zero) {
        write_uint32(buf, (uint32_t)(tmp.size() + 1));
        buf.push_back(0x00);
        buf.insert(buf.end(), tmp.begin(), tmp.end());
    } else {
        write_uint32(buf, (uint32_t)tmp.size());
        buf.insert(buf.end(), tmp.begin(), tmp.end());
    }
}

static std::vector<unsigned char> build_public_blob(const BIGNUM* e, const BIGNUM* n) {
    std::vector<unsigned char> b;
    write_string(b, std::string("ssh-rsa"));
    write_mpint(b, e);
    write_mpint(b, n);
    return b;
}

static std::vector<unsigned char> build_private_block(const BIGNUM* n, const BIGNUM* e, const BIGNUM* d, const BIGNUM* iqmp, const BIGNUM* p, const BIGNUM* q, const std::string& comment) {
    std::vector<unsigned char> b;
    unsigned int checkint;
    RAND_bytes((unsigned char*)&checkint, sizeof(checkint));
    write_uint32(b, checkint);
    write_uint32(b, checkint);
    write_string(b, std::string("ssh-rsa"));
    write_mpint(b, n);
    write_mpint(b, e);
    write_mpint(b, d);
    write_mpint(b, iqmp);
    write_mpint(b, p);
    write_mpint(b, q);
    write_string(b, comment);

    size_t pad_len = 8 - (b.size() % 8);
    if (pad_len == 0) pad_len = 8;
    for (size_t i = 1; i <= pad_len; ++i) b.push_back((unsigned char)i);
    return b;
}

static std::string b64_wrap(const std::vector<unsigned char>& data) {
    int outlen = 4 * ((int)data.size() + 2) / 3;
    std::string out;
    out.resize((size_t)outlen);
    int written = EVP_EncodeBlock(reinterpret_cast<unsigned char*>(&out[0]),
                                  data.data(), (int)data.size());
    out.resize((size_t)written);
    // Wrap at 70 chars
    std::string wrapped;
    for (size_t i = 0; i < out.size(); i += 70) {
        size_t len = (i + 70 <= out.size()) ? 70 : (out.size() - i);
        wrapped.append(out, i, len);
        wrapped.push_back('\n');
    }
    return wrapped;
}

std::string generateOpenSSHRsaPrivateKey(int bits, const std::string& comment) {
    if (bits < 2048 || bits > 8192) {
        throw std::invalid_argument("bits must be between 2048 and 8192");
    }
    if (comment.size() > 1024) {
        throw std::invalid_argument("comment too long");
    }

    std::string safeComment = comment;

    BIGNUM* e_bn = BN_new();
    if (!e_bn) throw std::runtime_error("BN_new failed");
    if (!BN_set_word(e_bn, RSA_F4)) {
        BN_free(e_bn);
        throw std::runtime_error("BN_set_word failed");
    }

    RSA* rsa = RSA_new();
    if (!rsa) {
        BN_free(e_bn);
        throw std::runtime_error("RSA_new failed");
    }

    if (RSA_generate_key_ex(rsa, bits, e_bn, nullptr) != 1) {
        RSA_free(rsa);
        BN_free(e_bn);
        throw std::runtime_error("RSA_generate_key_ex failed");
    }
    BN_free(e_bn);

    const BIGNUM* n = nullptr; const BIGNUM* e = nullptr; const BIGNUM* d = nullptr;
    const BIGNUM* p = nullptr; const BIGNUM* q = nullptr;
    const BIGNUM* dmp1 = nullptr; const BIGNUM* dmq1 = nullptr; const BIGNUM* iqmp = nullptr;
    RSA_get0_key(rsa, &n, &e, &d);
    RSA_get0_factors(rsa, &p, &q);
    RSA_get0_crt_params(rsa, &dmp1, &dmq1, &iqmp);
    if (!n || !e || !d || !p || !q || !iqmp) {
        RSA_free(rsa);
        throw std::runtime_error("RSA parameters missing");
    }

    std::vector<unsigned char> pubBlob = build_public_blob(e, n);
    std::vector<unsigned char> privBlock = build_private_block(n, e, d, iqmp, p, q, safeComment);

    std::vector<unsigned char> finalBytes;
    const char magic[] = "openssh-key-v1";
    finalBytes.insert(finalBytes.end(), magic, magic + sizeof(magic) - 1);
    finalBytes.push_back(0x00);
    write_string(finalBytes, std::string("none"));
    write_string(finalBytes, std::string("none"));
    write_string(finalBytes, std::vector<unsigned char>());
    write_uint32(finalBytes, 1);
    write_string(finalBytes, pubBlob);
    write_string(finalBytes, privBlock);

    RSA_free(rsa);

    std::string b64 = b64_wrap(finalBytes);
    std::string header = "-----BEGIN OPENSSH PRIVATE KEY-----\n";
    std::string footer = "-----END OPENSSH PRIVATE KEY-----\n";
    return header + b64 + footer;
}

int main() {
    try {
        std::string comments[5] = {"test1", "user@example", "device-key", "backup", "prod-key"};
        int sizes[5] = {2048, 2048, 3072, 4096, 2048};
        for (int i = 0; i < 5; ++i) {
            std::string key = generateOpenSSHRsaPrivateKey(sizes[i], comments[i]);
            bool ok = key.rfind("-----BEGIN OPENSSH PRIVATE KEY-----", 0) == 0;
            std::cout << "Case " << (i + 1) << ": generated=" << (ok ? "true" : "false")
                      << ", bits=" << sizes[i] << ", length=" << key.size() << "\n";
        }
    } catch (const std::exception& ex) {
        std::cout << "Error: " << ex.what() << "\n";
    }
    return 0;
}