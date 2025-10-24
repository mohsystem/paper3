#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

namespace securestore {
    static const uint8_t MAGIC[4] = {'E','N','C','1'};
    static const uint8_t VERSION = 1;
    static const size_t SALT_LEN = 16;
    static const size_t IV_LEN = 12;
    static const size_t TAG_LEN = 16;
    static const int PBKDF2_ITERS = 210000;
    static const size_t KEY_LEN = 32;

    static bool derive_key(const std::string& pass, const uint8_t* salt, uint8_t* out_key, size_t out_len) {
        return PKCS5_PBKDF2_HMAC(pass.data(), static_cast<int>(pass.size()), salt, (int)SALT_LEN,
                                 PBKDF2_ITERS, EVP_sha256(), (int)out_len, out_key) == 1;
    }

    static std::vector<uint8_t> encryptToBytes(const std::string& passphrase, const std::vector<uint8_t>& plaintext) {
        std::vector<uint8_t> result;
        uint8_t salt[SALT_LEN];
        uint8_t iv[IV_LEN];
        if (RAND_bytes(salt, (int)SALT_LEN) != 1 || RAND_bytes(iv, (int)IV_LEN) != 1) {
            return result;
        }
        uint8_t key[KEY_LEN];
        if (!derive_key(passphrase, salt, key, KEY_LEN)) {
            OPENSSL_cleanse(key, KEY_LEN);
            return result;
        }

        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            OPENSSL_cleanse(key, KEY_LEN);
            return result;
        }

        std::vector<uint8_t> ciphertext(plaintext.size());
        int len = 0, outlen = 0;
        bool ok = true;

        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) ok = false;
        if (ok && EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, (int)IV_LEN, nullptr) != 1) ok = false;
        if (ok && EVP_EncryptInit_ex(ctx, nullptr, nullptr, key, iv) != 1) ok = false;
        if (ok && !plaintext.empty()) {
            if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext.data(), (int)plaintext.size()) != 1) ok = false;
            outlen = len;
        } else {
            outlen = 0;
        }
        if (ok && EVP_EncryptFinal_ex(ctx, ciphertext.data() + outlen, &len) != 1) ok = false;
        outlen += len;

        uint8_t tag[TAG_LEN];
        if (ok && EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, (int)TAG_LEN, tag) != 1) ok = false;

        EVP_CIPHER_CTX_free(ctx);

        if (!ok) {
            OPENSSL_cleanse(key, KEY_LEN);
            return std::vector<uint8_t>();
        }

        ciphertext.resize((size_t)outlen);

        // Assemble: MAGIC|VERSION|SALT|IV|CIPHERTEXT|TAG
        result.reserve(4 + 1 + SALT_LEN + IV_LEN + ciphertext.size() + TAG_LEN);
        result.insert(result.end(), MAGIC, MAGIC + 4);
        result.push_back(VERSION);
        result.insert(result.end(), salt, salt + SALT_LEN);
        result.insert(result.end(), iv, iv + IV_LEN);
        result.insert(result.end(), ciphertext.begin(), ciphertext.end());
        result.insert(result.end(), tag, tag + TAG_LEN);

        OPENSSL_cleanse(key, KEY_LEN);
        return result;
    }

    static std::vector<uint8_t> decryptFromBytes(const std::string& passphrase, const std::vector<uint8_t>& blob) {
        std::vector<uint8_t> plaintext;
        size_t minLen = 4 + 1 + SALT_LEN + IV_LEN + TAG_LEN;
        if (blob.size() < minLen) return plaintext;
        if (!std::equal(blob.begin(), blob.begin() + 4, MAGIC)) return plaintext;
        if (blob[4] != VERSION) return plaintext;

        const uint8_t* salt = blob.data() + 5;
        const uint8_t* iv = blob.data() + 5 + SALT_LEN;
        const uint8_t* rest = blob.data() + 5 + SALT_LEN + IV_LEN;
        size_t restLen = blob.size() - (5 + SALT_LEN + IV_LEN);
        if (restLen < TAG_LEN) return plaintext;

        size_t ctLen = restLen - TAG_LEN;
        const uint8_t* ciphertext = rest;
        const uint8_t* tag = rest + ctLen;

        uint8_t key[KEY_LEN];
        if (!derive_key(passphrase, salt, key, KEY_LEN)) {
            OPENSSL_cleanse(key, KEY_LEN);
            return plaintext;
        }

        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            OPENSSL_cleanse(key, KEY_LEN);
            return plaintext;
        }

        plaintext.assign(ctLen, 0);
        int len = 0, outlen = 0;
        bool ok = true;

        if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) ok = false;
        if (ok && EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, (int)IV_LEN, nullptr) != 1) ok = false;
        if (ok && EVP_DecryptInit_ex(ctx, nullptr, nullptr, key, iv) != 1) ok = false;
        if (ok && ctLen > 0) {
            if (EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext, (int)ctLen) != 1) ok = false;
            outlen = len;
        } else {
            outlen = 0;
        }
        if (ok && EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, (int)TAG_LEN, (void*)tag) != 1) ok = false;
        if (ok && EVP_DecryptFinal_ex(ctx, plaintext.data() + outlen, &len) != 1) ok = false;
        outlen += len;

        EVP_CIPHER_CTX_free(ctx);
        OPENSSL_cleanse(key, KEY_LEN);

        if (!ok) {
            plaintext.clear();
            return plaintext;
        }
        plaintext.resize((size_t)outlen);
        return plaintext;
    }

    static std::filesystem::path resolveSafePath(const std::filesystem::path& base, const std::string& userRel) {
        if (userRel.empty() || userRel.front() == '/' || userRel.front() == '\\' || userRel.find("..") != std::string::npos) {
            throw std::runtime_error("Invalid path");
        }
        std::filesystem::path p = (base / userRel);
        p = std::filesystem::weakly_canonical(p);
        auto baseCanon = std::filesystem::weakly_canonical(base);
        std::string pStr = p.generic_string();
        std::string baseStr = baseCanon.generic_string();
        if (pStr.rfind(baseStr, 0) != 0) {
            throw std::runtime_error("Escapes base");
        }
        // Disallow symlink for target if exists and for parents
        if (std::filesystem::exists(p) && std::filesystem::is_symlink(p)) {
            throw std::runtime_error("Symlink not allowed");
        }
        auto parent = p.parent_path();
        while (!parent.empty() && parent != parent.root_path()) {
            if (std::filesystem::exists(parent) && std::filesystem::is_symlink(parent)) {
                throw std::runtime_error("Symlink in path");
            }
            parent = parent.parent_path();
        }
        return p;
    }

    static bool saveEncrypted(const std::string& baseDir, const std::string& userRel, const std::string& pass, const std::vector<uint8_t>& data) {
        try {
            auto base = std::filesystem::absolute(baseDir);
            auto target = resolveSafePath(base, userRel);
            auto blob = encryptToBytes(pass, data);
            if (blob.empty()) return false;
            std::filesystem::create_directories(target.parent_path());
            auto tmp = target;
            tmp += ".tmp";
            {
                std::ofstream ofs(tmp, std::ios::binary | std::ios::trunc);
                if (!ofs) return false;
                ofs.write(reinterpret_cast<const char*>(blob.data()), (std::streamsize)blob.size());
                ofs.flush();
            }
            std::error_code ec;
            std::filesystem::rename(tmp, target, ec);
            if (ec) {
                std::filesystem::remove(target);
                std::filesystem::rename(tmp, target, ec);
                if (ec) return false;
            }
            return true;
        } catch (...) {
            return false;
        }
    }

    static std::vector<uint8_t> loadDecrypted(const std::string& baseDir, const std::string& userRel, const std::string& pass) {
        std::vector<uint8_t> out;
        try {
            auto base = std::filesystem::absolute(baseDir);
            auto target = resolveSafePath(base, userRel);
            std::ifstream ifs(target, std::ios::binary);
            if (!ifs) return out;
            std::vector<uint8_t> blob((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
            return decryptFromBytes(pass, blob);
        } catch (...) {
            return out;
        }
    }

    static std::string randomPassphrase(size_t nbytes) {
        std::vector<uint8_t> buf(nbytes);
        RAND_bytes(buf.data(), (int)buf.size());
        static const char* hex = "0123456789abcdef";
        std::string s;
        s.reserve(buf.size()*2);
        for (auto b : buf) {
            s.push_back(hex[(b>>4) & 0xF]);
            s.push_back(hex[b & 0xF]);
        }
        return s;
    }
}

int main() {
    using namespace securestore;
    std::string base = "secure_store_cpp";
    std::vector<std::string> files = {"rec1.bin","rec2.bin","rec3.bin","rec4.bin","rec5.bin"};
    std::vector<std::string> pass = {
        randomPassphrase(16),
        randomPassphrase(16),
        randomPassphrase(16),
        randomPassphrase(16),
        randomPassphrase(16)
    };
    std::vector<std::string> records = {
        "User: Alice; CC: 4111-1111-1111-1111; Exp: 12/29",
        "User: Bob; SSN: 123-45-6789; DOB: 1990-01-01",
        "User: Carol; Passport: X12345678; Country: US",
        "User: Dave; Bank: 0123456789; Routing: 021000021",
        "User: Eve; Email: eve@example.com; Phone: +1-555-0100"
    };
    for (size_t i = 0; i < 5; ++i) {
        std::vector<uint8_t> data(records[i].begin(), records[i].end());
        bool ok = saveEncrypted(base, files[i], pass[i], data);
        if (!ok) {
            std::cout << "Test " << (i+1) << " save failed\n";
            continue;
        }
        auto dec = loadDecrypted(base, files[i], pass[i]);
        std::string rec(dec.begin(), dec.end());
        std::cout << "Test " << (i+1) << " " << (rec == records[i] ? "OK" : "mismatch") << "\n";
    }
    return 0;
}