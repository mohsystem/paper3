#include <cstdint>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>

static const uint32_t DELTA_CPP = 0x9E3779B9u;

static bool parseHexKey(const std::string& keyHex, uint32_t k[4]) {
    auto hexToBytes = [](const std::string& s, std::vector<uint8_t>& out)->bool {
        if (s.size() % 2 != 0) return false;
        out.clear();
        out.reserve(s.size() / 2);
        auto hexval = [](char c)->int {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
            return -1;
        };
        for (size_t i = 0; i < s.size(); i += 2) {
            int hi = hexval(s[i]);
            int lo = hexval(s[i + 1]);
            if (hi < 0 || lo < 0) return false;
            out.push_back(static_cast<uint8_t>((hi << 4) | lo));
        }
        return true;
    };
    if (keyHex.size() != 32) return false;
    std::vector<uint8_t> kb;
    if (!hexToBytes(keyHex, kb) || kb.size() != 16) return false;
    for (int i = 0; i < 4; ++i) {
        size_t j = i * 4;
        k[i] = (uint32_t)kb[j] |
               ((uint32_t)kb[j + 1] << 8) |
               ((uint32_t)kb[j + 2] << 16) |
               ((uint32_t)kb[j + 3] << 24);
    }
    return true;
}

static std::vector<uint32_t> toUint32Array(const std::vector<uint8_t>& data, bool includeLength) {
    size_t n = (data.size() + 3) >> 2;
    size_t size = includeLength ? (n + 1) : (n == 0 ? 1 : n);
    std::vector<uint32_t> v(size, 0);
    for (size_t i = 0; i < data.size(); ++i) {
        v[i >> 2] |= ((uint32_t)data[i] & 0xFFu) << ((i & 3) << 3);
    }
    if (includeLength) {
        v[size - 1] = (uint32_t)data.size();
    }
    return v;
}

static std::vector<uint8_t> toByteArray(const std::vector<uint32_t>& v, bool includeLength) {
    size_t n = v.size() << 2;
    size_t m = n;
    if (includeLength) {
        if (v.empty()) return {};
        uint32_t len = v.back();
        if (len > n) return {};
        m = len;
    }
    std::vector<uint8_t> out(m);
    for (size_t i = 0; i < m; ++i) {
        out[i] = (uint8_t)((v[i >> 2] >> ((i & 3) << 3)) & 0xFFu);
    }
    return out;
}

static void xxteaEncrypt(std::vector<uint32_t>& v, const uint32_t k[4]) {
    size_t n = v.size();
    if (n < 2) return;
    uint32_t rounds = 6 + 52 / (uint32_t)n;
    uint32_t sum = 0;
    uint32_t z = v[n - 1], y;
    while (rounds--) {
        sum += DELTA_CPP;
        uint32_t e = (sum >> 2) & 3u;
        for (size_t p = 0; p < n - 1; ++p) {
            y = v[p + 1];
            uint32_t mx = (((z >> 5) ^ (y << 2)) + ((y >> 3) ^ (z << 4))) ^ ((sum ^ y) + (k[(p & 3u) ^ e] ^ z));
            v[p] += mx;
            z = v[p];
        }
        y = v[0];
        uint32_t mx = (((z >> 5) ^ (y << 2)) + ((y >> 3) ^ (z << 4))) ^ ((sum ^ y) + (k[((n - 1) & 3u) ^ e] ^ z));
        v[n - 1] += mx;
        z = v[n - 1];
    }
}

static void xxteaDecrypt(std::vector<uint32_t>& v, const uint32_t k[4]) {
    size_t n = v.size();
    if (n < 2) return;
    uint32_t rounds = 6 + 52 / (uint32_t)n;
    uint32_t sum = rounds * DELTA_CPP;
    uint32_t z = v[n - 1];
    while (sum) {
        uint32_t e = (sum >> 2) & 3u;
        for (size_t p = n - 1; p > 0; --p) {
            z = v[p - 1];
            uint32_t mx = (((z >> 5) ^ (v[p] << 2)) + ((v[p] >> 3) ^ (z << 4))) ^ ((sum ^ v[p]) + (k[(p & 3u) ^ e] ^ z));
            v[p] -= mx;
        }
        uint32_t y = v[0];
        uint32_t mx = (((z >> 5) ^ (y << 2)) + ((y >> 3) ^ (z << 4))) ^ ((sum ^ y) + (k[(0 & 3u) ^ e] ^ z));
        v[0] -= mx;
        z = v[n - 1];
        sum -= DELTA_CPP;
    }
}

static std::string bytesToHex(const std::vector<uint8_t>& data) {
    static const char* hex = "0123456789abcdef";
    std::string s;
    s.reserve(data.size() * 2);
    for (uint8_t b : data) {
        s.push_back(hex[b >> 4]);
        s.push_back(hex[b & 0x0F]);
    }
    return s;
}

static bool hexToBytes(const std::string& s, std::vector<uint8_t>& out) {
    auto hexval = [](char c)->int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    };
    if (s.size() % 2 != 0) return false;
    out.clear();
    out.reserve(s.size() / 2);
    for (size_t i = 0; i < s.size(); i += 2) {
        int hi = hexval(s[i]);
        int lo = hexval(s[i + 1]);
        if (hi < 0 || lo < 0) return false;
        out.push_back((uint8_t)((hi << 4) | lo));
    }
    return true;
}

std::string encrypt(const std::string& plaintext, const std::string& keyHex) {
    if (keyHex.empty()) return std::string();
    uint32_t k[4];
    if (!parseHexKey(keyHex, k)) return std::string();
    std::vector<uint8_t> data(plaintext.begin(), plaintext.end());
    if (data.empty()) return std::string();
    auto v = toUint32Array(data, true);
    xxteaEncrypt(v, k);
    auto out = toByteArray(v, false);
    return bytesToHex(out);
}

std::string decrypt(const std::string& cipherHex, const std::string& keyHex) {
    if (keyHex.empty()) return std::string();
    uint32_t k[4];
    if (!parseHexKey(keyHex, k)) return std::string();
    std::vector<uint8_t> enc;
    if (!hexToBytes(cipherHex, enc) || enc.empty()) return std::string();
    auto v = toUint32Array(enc, false);
    if (v.empty()) return std::string();
    xxteaDecrypt(v, k);
    auto out = toByteArray(v, true);
    if (out.empty() && !v.empty() && v.back() != 0) return std::string();
    return std::string(out.begin(), out.end());
}

int main() {
    std::string key = "00112233445566778899aabbccddeeff";
    std::string tests[5] = {
        "",
        "hello",
        "The quick brown fox jumps over the lazy dog.",
        "1234567890",
        "Sensitive data!"
    };
    for (const auto& t : tests) {
        std::string enc = encrypt(t, key);
        std::string dec = decrypt(enc, key);
        std::cout << "PT: " << t << "\n";
        std::cout << "CT: " << enc << "\n";
        std::cout << "DC: " << dec << "\n";
        std::cout << "----\n";
    }
    return 0;
}