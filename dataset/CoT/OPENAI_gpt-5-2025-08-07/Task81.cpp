#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <cstring>

#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/crypto.h>

class Task81 {
public:
    static std::vector<unsigned char> hexToBytes(const std::string& hex) {
        std::string cleaned;
        cleaned.reserve(hex.size());
        for (char c : hex) {
            if (std::isxdigit(static_cast<unsigned char>(c))) {
                cleaned.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
            }
        }
        if (cleaned.empty() || (cleaned.size() % 2 != 0)) {
            throw std::invalid_argument("Invalid hex fingerprint format.");
        }
        std::vector<unsigned char> out(cleaned.size() / 2);
        for (size_t i = 0; i < out.size(); ++i) {
            int hi = std::isdigit(cleaned[2*i]) ? cleaned[2*i] - '0' : cleaned[2*i] - 'A' + 10;
            int lo = std::isdigit(cleaned[2*i+1]) ? cleaned[2*i+1] - '0' : cleaned[2*i+1] - 'A' + 10;
            if (hi < 0 || hi > 15 || lo < 0 || lo > 15) throw std::invalid_argument("Invalid hex digit.");
            out[i] = static_cast<unsigned char>((hi << 4) | lo);
        }
        return out;
    }

    static std::string bytesToHex(const std::vector<unsigned char>& data) {
        static const char* hexd = "0123456789ABCDEF";
        std::string out;
        out.reserve(data.size() * 2);
        for (unsigned char b : data) {
            out.push_back(hexd[(b >> 4) & 0xF]);
            out.push_back(hexd[b & 0xF]);
        }
        return out;
    }

    static const EVP_MD* normalizeAlg(const std::string& alg) {
        std::string up;
        up.reserve(alg.size());
        for (char c : alg) up.push_back(std::toupper(static_cast<unsigned char>(c)));
        if (up == "SHA-256" || up == "SHA256") return EVP_sha256();
        if (up == "SHA-384" || up == "SHA384") return EVP_sha384();
        if (up == "SHA-512" || up == "SHA512") return EVP_sha512();
        throw std::invalid_argument("Unsupported algorithm. Allowed: SHA-256, SHA-384, SHA-512");
    }

    static std::vector<unsigned char> computeDigest(const std::vector<unsigned char>& der, const std::string& alg) {
        const EVP_MD* md = normalizeAlg(alg);
        EVP_MD_CTX* ctx = EVP_MD_CTX_new();
        if (!ctx) throw std::runtime_error("EVP_MD_CTX_new failed");
        std::vector<unsigned char> out(EVP_MD_size(md));
        unsigned int outlen = 0;

        if (EVP_DigestInit_ex(ctx, md, nullptr) != 1 ||
            EVP_DigestUpdate(ctx, der.data(), der.size()) != 1 ||
            EVP_DigestFinal_ex(ctx, out.data(), &outlen) != 1) {
            EVP_MD_CTX_free(ctx);
            throw std::runtime_error("EVP digest operation failed");
        }
        EVP_MD_CTX_free(ctx);
        out.resize(outlen);
        return out;
    }

    static bool constantTimeEquals(const std::vector<unsigned char>& a, const std::vector<unsigned char>& b) {
        if (a.size() != b.size()) return false;
        // Use CRYPTO_memcmp for constant-time compare (OpenSSL)
        return CRYPTO_memcmp(a.data(), b.data(), a.size()) == 0;
    }

    static std::vector<unsigned char> pemToDerIfPem(const std::string& s) {
        const std::string begin = "-----BEGIN CERTIFICATE-----";
        const std::string end = "-----END CERTIFICATE-----";
        auto posBegin = s.find(begin);
        auto posEnd = s.find(end);
        if (posBegin == std::string::npos || posEnd == std::string::npos || posEnd < posBegin) {
            return {};
        }
        size_t contentStart = posBegin + begin.size();
        std::string b64 = s.substr(contentStart, posEnd - contentStart);

        // Remove non-base64 chars
        std::string cleaned;
        cleaned.reserve(b64.size());
        for (char c : b64) {
            if ((c >= 'A' && c <= 'Z') ||
                (c >= 'a' && c <= 'z') ||
                (c >= '0' && c <= '9') ||
                c == '+' || c == '/' || c == '=') {
                cleaned.push_back(c);
            }
        }
        if (cleaned.empty()) throw std::invalid_argument("Invalid PEM base64 content.");
        // Decode base64 using OpenSSL
        std::vector<unsigned char> out(((cleaned.size() + 3) / 4) * 3);
        // EVP_DecodeBlock doesn't ignore whitespace; we already cleaned.
        int decodedLen = EVP_DecodeBlock(out.data(),
                                         reinterpret_cast<const unsigned char*>(cleaned.data()),
                                         static_cast<int>(cleaned.size()));
        if (decodedLen < 0) {
            throw std::invalid_argument("Base64 decode failed.");
        }
        // Adjust for padding '='
        int pad = 0;
        if (!cleaned.empty() && cleaned.back() == '=') {
            pad++;
            if (cleaned.size() > 1 && cleaned[cleaned.size()-2] == '=') pad++;
        }
        decodedLen -= pad;
        if (decodedLen < 0) decodedLen = 0;
        out.resize(static_cast<size_t>(decodedLen));
        return out;
    }

    static std::string computeCertDigestHex(const std::vector<unsigned char>& der, const std::string& alg) {
        if (der.empty()) throw std::invalid_argument("Certificate data cannot be empty.");
        return bytesToHex(computeDigest(der, alg));
    }

    static bool certificateMatchesHash(const std::vector<unsigned char>& certBytes, const std::string& expectedHex, const std::string& alg) {
        if (certBytes.empty()) throw std::invalid_argument("Certificate data cannot be empty.");
        auto expected = hexToBytes(expectedHex);
        auto actual = computeDigest(certBytes, alg);
        return constantTimeEquals(actual, expected);
    }

    static bool certificateMatchesHashFromPem(const std::string& pem, const std::string& expectedHex, const std::string& alg) {
        auto der = pemToDerIfPem(pem);
        if (der.empty()) throw std::invalid_argument("Invalid PEM format or certificate block not found.");
        return certificateMatchesHash(der, expectedHex, alg);
    }

    static std::string colonizeHex(const std::string& hex) {
        std::string cleaned;
        for (char c : hex) if (std::isxdigit(static_cast<unsigned char>(c))) cleaned.push_back(std::toupper(static_cast<unsigned char>(c)));
        std::string out;
        for (size_t i = 0; i < cleaned.size(); i += 2) {
            if (!out.empty()) out.push_back(':');
            out.push_back(cleaned[i]);
            if (i + 1 < cleaned.size()) out.push_back(cleaned[i+1]);
        }
        return out;
    }

    static std::string base64Encode(const std::vector<unsigned char>& data, size_t lineWidth = 64) {
        // Use OpenSSL EVP_EncodeBlock (no line breaks), then insert line breaks manually
        std::string out;
        std::vector<unsigned char> enc(((data.size() + 2) / 3) * 4 + 4);
        int n = EVP_EncodeBlock(enc.data(), data.data(), static_cast<int>(data.size()));
        if (n < 0) throw std::runtime_error("Base64 encode failed");
        std::string b64(reinterpret_cast<char*>(enc.data()), static_cast<size_t>(n));
        if (lineWidth == 0) return b64;
        out.reserve(b64.size() + b64.size() / lineWidth + 8);
        for (size_t i = 0; i < b64.size(); i += lineWidth) {
            out.append(b64.substr(i, std::min(lineWidth, b64.size() - i)));
            out.push_back('\n');
        }
        if (!out.empty() && out.back() == '\n') out.pop_back();
        return out;
    }
};

int main() {
    try {
        // Test 1: DER match, SHA-256
        std::vector<unsigned char> cert1 = {'D','E','R','C','E','R','T','-','O','N','E'};
        std::string expected1 = Task81::computeCertDigestHex(cert1, "SHA-256");
        std::cout << "Test1 (DER match, SHA-256): " << (Task81::certificateMatchesHash(cert1, expected1, "SHA-256") ? "1" : "0") << "\n";

        // Test 2: DER mismatch
        std::vector<unsigned char> cert2 = {'A','N','O','T','H','E','R','-','C','E','R','T'};
        std::string wrong(64, '0'); // 256-bit all-zero hex
        std::cout << "Test2 (DER mismatch): " << (Task81::certificateMatchesHash(cert2, wrong, "SHA-256") ? "1" : "0") << "\n";

        // Test 3: PEM match, SHA-256
        std::vector<unsigned char> der3 = {1,2,3,4,5};
        std::string pemBody = Task81::base64Encode(der3, 64);
        std::string pem = "-----BEGIN CERTIFICATE-----\n" + pemBody + "\n-----END CERTIFICATE-----\n";
        std::string expected3 = Task81::computeCertDigestHex(der3, "SHA-256");
        std::cout << "Test3 (PEM match, SHA-256): " << (Task81::certificateMatchesHashFromPem(pem, expected3, "SHA-256") ? "1" : "0") << "\n";

        // Test 4: Invalid algorithm handling
        try {
            (void)Task81::certificateMatchesHash(cert1, expected1, "MD5");
            std::cout << "Test4 (Invalid alg): 0\n";
        } catch (const std::invalid_argument&) {
            std::cout << "Test4 (Invalid alg): 1\n";
        }

        // Test 5: Colon-separated fingerprint accepted
        std::string colonExpected1 = Task81::colonizeHex(expected1);
        std::cout << "Test5 (Colon-separated fingerprint): " << (Task81::certificateMatchesHash(cert1, colonExpected1, "SHA-256") ? "1" : "0") << "\n";
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}