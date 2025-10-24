#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Chain-of-Through Step 1: Problem understanding
// - XTEA in CTR mode using 128-bit key and 64-bit nonce; encrypt message to hex.
// Step 2: Security requirements
// - Enforce key = 16 bytes, require explicit nonce, careful unsigned ops.
// Step 3: Secure coding generation with bounds checks and explicit sizes.
// Step 4: Code review via clear structure.
// Step 5: Secure code output.

static void xtea_encrypt_block(uint32_t v[2], const uint32_t k[4]) {
    uint32_t v0 = v[0], v1 = v[1];
    uint32_t sum = 0;
    const uint32_t delta = 0x9E3779B9u;
    for (int i = 0; i < 32; ++i) {
        v0 += (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + k[sum & 3u]);
        sum += delta;
        v1 += (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + k[(sum >> 11) & 3u]);
    }
    v[0] = v0;
    v[1] = v1;
}

static void key_to_words_le(const char *key16, uint32_t k[4]) {
    for (int i = 0; i < 4; ++i) {
        int base = i * 4;
        k[i] = (uint32_t)(unsigned char)key16[base]
             | ((uint32_t)(unsigned char)key16[base + 1] << 8)
             | ((uint32_t)(unsigned char)key16[base + 2] << 16)
             | ((uint32_t)(unsigned char)key16[base + 3] << 24);
    }
}

static char *to_hex(const unsigned char *data, size_t len) {
    static const char *digits = "0123456789abcdef";
    char *out = (char *)malloc(len * 2 + 1);
    if (!out) return NULL;
    for (size_t i = 0; i < len; ++i) {
        out[i * 2] = digits[data[i] >> 4];
        out[i * 2 + 1] = digits[data[i] & 0x0F];
    }
    out[len * 2] = '\0';
    return out;
}

char *encrypt_message(const char *message, const char *key16, uint64_t nonce) {
    if (!message || !key16) {
        return NULL;
    }
    size_t key_len = strlen(key16);
    if (key_len != 16) {
        return NULL; // enforce 16-byte key
    }
    size_t msg_len = strlen(message);
    if (msg_len == 0) {
        char *empty = (char *)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    uint32_t k[4];
    key_to_words_le(key16, k);

    unsigned char *out = (unsigned char *)malloc(msg_len);
    if (!out) return NULL;

    const size_t blockSize = 8;
    size_t blocks = (msg_len + blockSize - 1) / blockSize;
    for (size_t i = 0; i < blocks; ++i) {
        uint64_t ctr = nonce + i;
        uint32_t v[2];
        v[0] = (uint32_t)(ctr & 0xFFFFFFFFull);
        v[1] = (uint32_t)((ctr >> 32) & 0xFFFFFFFFull);
        xtea_encrypt_block(v, k);
        unsigned char ks[8];
        ks[0] = (unsigned char)(v[0] & 0xFF);
        ks[1] = (unsigned char)((v[0] >> 8) & 0xFF);
        ks[2] = (unsigned char)((v[0] >> 16) & 0xFF);
        ks[3] = (unsigned char)((v[0] >> 24) & 0xFF);
        ks[4] = (unsigned char)(v[1] & 0xFF);
        ks[5] = (unsigned char)((v[1] >> 8) & 0xFF);
        ks[6] = (unsigned char)((v[1] >> 16) & 0xFF);
        ks[7] = (unsigned char)((v[1] >> 24) & 0xFF);

        size_t off = i * blockSize;
        size_t remain = blockSize;
        if (off + remain > msg_len) remain = msg_len - off;
        for (size_t j = 0; j < remain; ++j) {
            out[off + j] = ((const unsigned char *)message)[off + j] ^ ks[j];
        }
    }
    char *hex = to_hex(out, msg_len);
    free(out);
    return hex;
}

int main(void) {
    const char *messages[5] = {
        "",
        "Hello, world!",
        "The quick brown fox jumps over the lazy dog",
        "Sample message with multiple blocks that exceeds eight bytes.",
        "Edge case message 1234567890!@#$%^&*()_+-=[]{}|;:',.<>/?"
    };
    const char *keys[5] = {
        "0123456789ABCDEF",
        "A1B2C3D4E5F60708",
        "Sixteen byte key",
        "0123456789ABCDE!",
        "Uniq16ByteKey!OK"
    };
    uint64_t nonces[5] = {
        0x0123456789ABCDEFull,
        0x0F1E2D3C4B5A6978ull,
        0x1122334455667788ull,
        0x8877665544332211ull,
        0xCAFEBABEDEADBEEFull
    };
    for (int i = 0; i < 5; ++i) {
        char *ct = encrypt_message(messages[i], keys[i], nonces[i]);
        if (ct) {
            printf("Test %d ciphertext (hex): %s\n", i + 1, ct);
            free(ct);
        } else {
            printf("Test %d encryption failed (invalid input or memory error)\n", i + 1);
        }
    }
    return 0;
}