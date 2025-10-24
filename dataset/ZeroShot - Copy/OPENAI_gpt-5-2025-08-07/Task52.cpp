#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <array>

struct ChaCha20 {
    std::array<uint32_t, 16> state{};
    std::array<uint8_t, 64> buffer{};
    size_t pos = 64;

    static constexpr uint32_t SIGMA[4] = {
        0x61707865u, 0x3320646eu, 0x79622d32u, 0x6b206574u
    };

    static uint32_t rotl(uint32_t v, int c) {
        return (v << c) | (v >> (32 - c));
    }

    static void quarterround(uint32_t x[16], int a, int b, int c, int d) {
        x[a] += x[b]; x[d] = rotl(x[d] ^ x[a], 16);
        x[c] += x[d]; x[b] = rotl(x[b] ^ x[c], 12);
        x[a] += x[b]; x[d] = rotl(x[d] ^ x[a], 8);
        x[c] += x[d]; x[b] = rotl(x[b] ^ x[c], 7);
    }

    static uint32_t load32(const uint8_t* p) {
        return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
    }

    static void store32(uint32_t v, uint8_t* p) {
        p[0] = (uint8_t)(v);
        p[1] = (uint8_t)(v >> 8);
        p[2] = (uint8_t)(v >> 16);
        p[3] = (uint8_t)(v >> 24);
    }

    ChaCha20(const std::array<uint8_t,32>& key, const std::array<uint8_t,12>& nonce, uint32_t counter) {
        state[0] = SIGMA[0];
        state[1] = SIGMA[1];
        state[2] = SIGMA[2];
        state[3] = SIGMA[3];
        for (int i = 0; i < 8; i++) {
            state[4 + i] = load32(&key[i*4]);
        }
        state[12] = counter;
        state[13] = load32(&nonce[0]);
        state[14] = load32(&nonce[4]);
        state[15] = load32(&nonce[8]);
    }

    void gen_block() {
        uint32_t x[16];
        for (int i = 0; i < 16; i++) x[i] = state[i];
        for (int i = 0; i < 10; i++) {
            quarterround(x, 0, 4, 8, 12);
            quarterround(x, 1, 5, 9, 13);
            quarterround(x, 2, 6, 10, 14);
            quarterround(x, 3, 7, 11, 15);
            quarterround(x, 0, 5, 10, 15);
            quarterround(x, 1, 6, 11, 12);
            quarterround(x, 2, 7, 8, 13);
            quarterround(x, 3, 4, 9, 14);
        }
        for (int i = 0; i < 16; i++) {
            uint32_t o = x[i] + state[i];
            store32(o, &buffer[i*4]);
        }
        state[12] += 1;
        pos = 0;
    }

    void xor_inplace(uint8_t* data, size_t len) {
        size_t i = 0;
        while (i < len) {
            if (pos >= 64) gen_block();
            size_t take = (len - i < (64 - pos)) ? (len - i) : (64 - pos);
            for (size_t j = 0; j < take; j++) {
                data[i + j] ^= buffer[pos + j];
            }
            pos += take;
            i += take;
        }
    }

    std::vector<uint8_t> xor_bytes(const std::vector<uint8_t>& data) {
        std::vector<uint8_t> out = data;
        xor_inplace(out.data(), out.size());
        return out;
    }
};

std::vector<uint8_t> chacha20_xor_bytes(const std::array<uint8_t,32>& key, const std::array<uint8_t,12>& nonce, uint32_t counter, const std::vector<uint8_t>& data) {
    ChaCha20 ctx(key, nonce, counter);
    return ctx.xor_bytes(data);
}

bool stream_xor_file(const std::string& inputPath, const std::string& outputPath, const std::array<uint8_t,32>& key, const std::array<uint8_t,12>& nonce, uint32_t counter) {
    std::ifstream in(inputPath, std::ios::binary);
    if (!in) return false;
    std::ofstream out(outputPath, std::ios::binary | std::ios::trunc);
    if (!out) return false;
    ChaCha20 ctx(key, nonce, counter);
    std::vector<uint8_t> buf(8192);
    while (in) {
        in.read(reinterpret_cast<char*>(buf.data()), static_cast<std::streamsize>(buf.size()));
        std::streamsize got = in.gcount();
        if (got > 0) {
            ctx.xor_inplace(buf.data(), static_cast<size_t>(got));
            out.write(reinterpret_cast<const char*>(buf.data()), got);
            if (!out) return false;
        }
    }
    return true;
}

bool encrypt_file(const std::string& inputPath, const std::string& outputPath, const std::array<uint8_t,32>& key, const std::array<uint8_t,12>& nonce) {
    return stream_xor_file(inputPath, outputPath, key, nonce, 1);
}

bool decrypt_file(const std::string& inputPath, const std::string& outputPath, const std::array<uint8_t,32>& key, const std::array<uint8_t,12>& nonce) {
    return stream_xor_file(inputPath, outputPath, key, nonce, 1);
}

static std::vector<uint8_t> pattern_bytes(size_t n) {
    std::vector<uint8_t> v(n);
    for (size_t i = 0; i < n; i++) v[i] = static_cast<uint8_t>(i & 0xff);
    return v;
}

int main() {
    std::array<uint8_t,32> key{};
    std::array<uint8_t,12> nonce{};
    for (size_t i = 0; i < key.size(); i++) key[i] = static_cast<uint8_t>((i * 7 + 3) & 0xff);
    for (size_t i = 0; i < nonce.size(); i++) nonce[i] = static_cast<uint8_t>((i * 5 + 1) & 0xff);

    std::string tmpDir = ".";
#ifdef _WIN32
    char* env = std::getenv("TEMP");
    if (env) tmpDir = env;
#else
    char* env = std::getenv("TMPDIR");
    if (env) tmpDir = env; else tmpDir = "/tmp";
#endif

    std::vector<size_t> sizes = {0, 13, 65, 1000, 50000};
    bool all_ok = true;

    for (int t = 0; t < 5; t++) {
        std::string in = tmpDir + "/cpp_in_" + std::to_string(t) + ".bin";
        std::string enc = tmpDir + "/cpp_enc_" + std::to_string(t) + ".bin";
        std::string dec = tmpDir + "/cpp_dec_" + std::to_string(t) + ".bin";

        std::vector<uint8_t> content;
        if (t == 1) {
            const char* msg = "Hello, world!";
            content.assign(msg, msg + std::strlen(msg));
        } else {
            content = pattern_bytes(sizes[t]);
        }

        {
            std::ofstream f(in, std::ios::binary | std::ios::trunc);
            f.write(reinterpret_cast<const char*>(content.data()), static_cast<std::streamsize>(content.size()));
        }

        bool ok1 = encrypt_file(in, enc, key, nonce);
        bool ok2 = decrypt_file(enc, dec, key, nonce);

        bool ok3 = false;
        if (ok1 && ok2) {
            std::ifstream fd(dec, std::ios::binary);
            std::vector<uint8_t> readback((std::istreambuf_iterator<char>(fd)), std::istreambuf_iterator<char>());
            ok3 = (readback == content);
        }

        std::cout << "Test " << t << ": " << ((ok1 && ok2 && ok3) ? "OK" : "FAIL") << "\n";
        all_ok &= (ok1 && ok2 && ok3);

        std::remove(in.c_str());
        std::remove(enc.c_str());
        std::remove(dec.c_str());
    }
    std::cout << "All tests: " << (all_ok ? "PASS" : "FAIL") << "\n";
    return all_ok ? 0 : 1;
}