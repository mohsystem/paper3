#include <openssl/evp.h>
#include <openssl/crypto.h>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

// Compute SHA-256 fingerprint of a certificate (DER or PEM input).
std::string fingerprintHex(const std::vector<unsigned char>& certInput, const std::string& algorithm) {
    if (certInput.empty()) {
        throw std::runtime_error("Empty certificate input");
    }
    std::string alg = algorithm;
    std::transform(alg.begin(), alg.end(), alg.begin(), ::toupper);
    if (alg != "SHA-256") {
        throw std::runtime_error("Only SHA-256 is supported");
    }
    std::vector<unsigned char> der = decodePemIfNeeded(certInput);
    std::vector<unsigned char> digest = sha256(der);
    return toHex(digest);
}

// Returns true if the certificate's fingerprint matches the known hash in constant time.
bool matchesKnownHash(const std::vector<unsigned char>& certInput, const std::string& knownHash, const std::string& algorithm) {
    if (knownHash.empty() || certInput.empty()) return false;
    std::string alg = algorithm;
    std::transform(alg.begin(), alg.end(), alg.begin(), ::toupper);
    if (alg != "SHA-256") return false;
    std::vector<unsigned char> der = decodePemIfNeeded(certInput);
    std::vector<unsigned char> actual = sha256(der);
    std::vector<unsigned char> expected;
    if (!parseHexFlexible(knownHash, expected)) return false;
    if (expected.size() != actual.size()) return false;
    return CRYPTO_memcmp(actual.data(), expected.data(), actual.size()) == 0;
}

static std::vector<unsigned char> sha256(const std::vector<unsigned char>& data) {
    std::vector<unsigned char> out(EVP_MAX_MD_SIZE);
    unsigned int out_len = 0;
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) throw std::runtime_error("EVP_MD_CTX_new failed");
    if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1) {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("EVP_DigestInit_ex failed");
    }
    if (EVP_DigestUpdate(ctx, data.data(), data.size()) != 1) {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("EVP_DigestUpdate failed");
    }
    if (EVP_DigestFinal_ex(ctx, out.data(), &out_len) != 1) {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("EVP_DigestFinal_ex failed");
    }
    EVP_MD_CTX_free(ctx);
    out.resize(out_len);
    return out;
}

static std::string toHex(const std::vector<unsigned char>& data) {
    static const char* hex = "0123456789abcdef";
    std::string s;
    s.reserve(data.size() * 2);
    for (unsigned char b : data) {
        s.push_back(hex[(b >> 4) & 0xF]);
        s.push_back(hex[b & 0xF]);
    }
    return s;
}

static bool isHexChar(char c) {
    return (c >= '0' && c <= '9') ||
           (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F');
}

// Parse a hex string allowing separators (:, space, -). Reject other chars.
static bool parseHexFlexible(const std::string& in, std::vector<unsigned char>& out) {
    std::string hexOnly;
    hexOnly.reserve(in.size());
    for (char c : in) {
        if (isHexChar(c)) {
            hexOnly.push_back(c);
        } else if (c == ':' || c == ' ' || c == '-' || c == '\t' || c == '\n' || c == '\r') {
            continue;
        } else {
            return false;
        }
    }
    if (hexOnly.empty() || (hexOnly.size() % 2) != 0) return false;
    out.clear();
    out.reserve(hexOnly.size() / 2);
    for (size_t i = 0; i < hexOnly.size(); i += 2) {
        int hi = std::isdigit(hexOnly[i]) ? hexOnly[i] - '0' : (std::tolower(hexOnly[i]) - 'a' + 10);
        int lo = std::isdigit(hexOnly[i + 1]) ? hexOnly[i + 1] - '0' : (std::tolower(hexOnly[i + 1]) - 'a' + 10);
        if (hi < 0 || hi > 15 || lo < 0 || lo > 15) return false;
        out.push_back(static_cast<unsigned char>((hi << 4) | lo));
    }
    return true;
}

static std::string toColonUpper(const std::string& hexLower) {
    std::string u;
    u.reserve(hexLower.size() + hexLower.size() / 2);
    for (size_t i = 0; i < hexLower.size(); ++i) {
        if (i > 0 && (i % 2) == 0) u.push_back(':');
        char c = hexLower[i];
        if (c >= 'a' && c <= 'z') c = static_cast<char>(c - 'a' + 'A');
        u.push_back(c);
    }
    return u;
}

// Base64 encode using OpenSSL (for tests)
static std::string base64Encode(const std::vector<unsigned char>& data) {
    // EVP_EncodeBlock outputs with no newlines
    int out_len = 4 * ((static_cast<int>(data.size()) + 2) / 3);
    std::string out(out_len, '\0');
    int written = EVP_EncodeBlock(reinterpret_cast<unsigned char*>(&out[0]),
                                  data.data(), static_cast<int>(data.size()));
    if (written < 0) return std::string();
    out.resize(static_cast<size_t>(written));
    return out;
}

// Base64 decode using OpenSSL EVP_DecodeBlock. Input must have no whitespace.
static bool base64DecodeNoNl(const std::string& in, std::vector<unsigned char>& out) {
    if (in.empty()) {
        out.clear();
        return true;
    }
    int inlen = static_cast<int>(in.size());
    int out_len = (inlen / 4) * 3;
    std::vector<unsigned char> buf(static_cast<size_t>(out_len));
    int ret = EVP_DecodeBlock(buf.data(),
                              reinterpret_cast<const unsigned char*>(in.data()),
                              inlen);
    if (ret < 0) return false;
    size_t pad = 0;
    if (!in.empty() && in.back() == '=') {
        pad++;
        if (in.size() >= 2 && in[in.size() - 2] == '=') pad++;
    }
    buf.resize(static_cast<size_t>(ret - static_cast<int>(pad)));
    out.swap(buf);
    return true;
}

static std::vector<unsigned char> decodePemIfNeeded(const std::vector<unsigned char>& input) {
    std::string s(reinterpret_cast<const char*>(input.data()), input.size());
    const std::string begin = "-----BEGIN CERTIFICATE-----";
    const std::string end = "-----END CERTIFICATE-----";
    if (s.find(begin) != std::string::npos && s.find(end) != std::string::npos) {
        size_t start = s.find(begin);
        start += begin.size();
        size_t stop = s.find(end, start);
        if (stop == std::string::npos || stop <= start) {
            throw std::runtime_error("Invalid PEM format");
        }
        std::string inner = s.substr(start, stop - start);
        // Remove whitespace
        std::string b64;
        b64.reserve(inner.size());
        for (char c : inner) {
            if (c != '\r' && c != '\n' && c != ' ' && c != '\t') b64.push_back(c);
        }
        std::vector<unsigned char> dec;
        if (!base64DecodeNoNl(b64, dec)) {
            throw std::runtime_error("Invalid Base64 in PEM");
        }
        return dec;
    }
    return input;
}

int main() {
    try {
        std::vector<unsigned char> cert1 = {'D','E','R','-','C','E','R','T','-','O','N','E','-','1','2','3'};
        std::vector<unsigned char> cert2 = {'D','E','R','-','C','E','R','T','-','T','W','O','-','4','5','6'};
        std::vector<unsigned char> cert3 = {'D','I','F','F','E','R','E','N','T','-','C','E','R','T','-','X','Y','Z'};

        std::string fp1 = fingerprintHex(cert1, "SHA-256");
        std::string fp2 = fingerprintHex(cert2, "SHA-256");
        std::string fp3 = fingerprintHex(cert3, "SHA-256");

        // Build PEM for cert2
        std::string b64 = base64Encode(cert2);
        std::string pem2 = "-----BEGIN CERTIFICATE-----\n" + b64 + "\n-----END CERTIFICATE-----\n";
        std::vector<unsigned char> pem2bytes(pem2.begin(), pem2.end());

        bool t1 = matchesKnownHash(cert1, fp1, "SHA-256");
        bool t2 = matchesKnownHash(cert1, toColonUpper(fp1), "SHA-256");
        bool t3 = matchesKnownHash(cert1, fp2, "SHA-256");
        bool t4 = matchesKnownHash(pem2bytes, fp2, "SHA-256");
        bool t5 = matchesKnownHash(cert3, "ZZ:11:GG", "SHA-256");

        std::cout << "Test1: " << (t1 ? "true" : "false") << "\n";
        std::cout << "Test2: " << (t2 ? "true" : "false") << "\n";
        std::cout << "Test3: " << (t3 ? "true" : "false") << "\n";
        std::cout << "Test4: " << (t4 ? "true" : "false") << "\n";
        std::cout << "Test5: " << (t5 ? "true" : "false") << "\n";
    } catch (const std::exception& ex) {
        std::cout << "Error\n";
        return 1;
    }
    return 0;
}