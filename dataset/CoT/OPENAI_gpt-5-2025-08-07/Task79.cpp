#include <bits/stdc++.h>
using namespace std;

// Chain-of-Through Step 1: Problem understanding
// - XTEA in CTR mode: encrypt message with 128-bit key and 64-bit nonce.
// Step 2: Security requirements
// - Enforce 16-byte key length, explicit nonce, careful unsigned arithmetic.
// Step 3: Secure coding generation
// - Use uint32_t/uint64_t, no UB, bounds-checked operations.
// Step 4: Code review through clear functions.
// Step 5: Secure code output.

static void xtea_encrypt_block(uint32_t &v0, uint32_t &v1, const uint32_t k[4]) {
    uint32_t sum = 0;
    const uint32_t delta = 0x9E3779B9u;
    for (int i = 0; i < 32; ++i) {
        v0 += (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + k[sum & 3u]);
        sum += delta;
        v1 += (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + k[(sum >> 11) & 3u]);
    }
}

static array<uint32_t,4> key_to_words_le(const string &key16) {
    array<uint32_t,4> k{};
    for (int i = 0; i < 4; ++i) {
        int base = i * 4;
        k[i] = (uint32_t)(uint8_t)key16[base]
             | ((uint32_t)(uint8_t)key16[base + 1] << 8)
             | ((uint32_t)(uint8_t)key16[base + 2] << 16)
             | ((uint32_t)(uint8_t)key16[base + 3] << 24);
    }
    return k;
}

static string to_hex(const vector<uint8_t> &data) {
    static const char* digits = "0123456789abcdef";
    string out;
    out.reserve(data.size() * 2);
    for (uint8_t b : data) {
        out.push_back(digits[b >> 4]);
        out.push_back(digits[b & 0x0F]);
    }
    return out;
}

string encryptMessage(const string &message, const string &key16, uint64_t nonce) {
    if (key16.size() != 16) {
        throw invalid_argument("Key must be exactly 16 bytes");
    }
    vector<uint8_t> msg(message.begin(), message.end());
    if (msg.empty()) return string();

    auto k = key_to_words_le(key16);
    vector<uint8_t> out(msg.size());

    const size_t blockSize = 8;
    size_t blocks = (msg.size() + blockSize - 1) / blockSize;
    for (size_t i = 0; i < blocks; ++i) {
        uint64_t ctr = nonce + i; // wrap naturally
        uint32_t lo = (uint32_t)(ctr & 0xFFFFFFFFull);
        uint32_t hi = (uint32_t)((ctr >> 32) & 0xFFFFFFFFull);
        uint32_t v0 = lo, v1 = hi;
        xtea_encrypt_block(v0, v1, k.data());
        uint8_t ks[8];
        ks[0] = (uint8_t)(v0 & 0xFF);
        ks[1] = (uint8_t)((v0 >> 8) & 0xFF);
        ks[2] = (uint8_t)((v0 >> 16) & 0xFF);
        ks[3] = (uint8_t)((v0 >> 24) & 0xFF);
        ks[4] = (uint8_t)(v1 & 0xFF);
        ks[5] = (uint8_t)((v1 >> 8) & 0xFF);
        ks[6] = (uint8_t)((v1 >> 16) & 0xFF);
        ks[7] = (uint8_t)((v1 >> 24) & 0xFF);
        size_t off = i * blockSize;
        size_t remain = min(blockSize, msg.size() - off);
        for (size_t j = 0; j < remain; ++j) {
            out[off + j] = msg[off + j] ^ ks[j];
        }
    }
    return to_hex(out);
}

int main() {
    vector<string> messages = {
        "",
        "Hello, world!",
        "The quick brown fox jumps over the lazy dog",
        "Sample message with multiple blocks that exceeds eight bytes.",
        "Edge case message 1234567890!@#$%^&*()_+-=[]{}|;:',.<>/?"
    };
    vector<string> keys = {
        "0123456789ABCDEF",
        "A1B2C3D4E5F60708",
        "Sixteen byte key",
        "0123456789ABCDE!",
        "Uniq16ByteKey!OK"
    };
    vector<uint64_t> nonces = {
        0x0123456789ABCDEFull,
        0x0F1E2D3C4B5A6978ull,
        0x1122334455667788ull,
        0x8877665544332211ull,
        0xCAFEBABEDEADBEEFull
    };
    for (int i = 0; i < 5; ++i) {
        try {
            string ct = encryptMessage(messages[i], keys[i], nonces[i]);
            cout << "Test " << (i+1) << " ciphertext (hex): " << ct << "\n";
        } catch (const exception &e) {
            cout << "Error: " << e.what() << "\n";
        }
    }
    return 0;
}