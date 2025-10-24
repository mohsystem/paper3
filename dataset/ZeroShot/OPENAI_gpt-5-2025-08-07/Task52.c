#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    uint32_t state[16];
    uint8_t buffer[64];
    size_t pos;
} chacha20_ctx;

static const uint32_t SIGMA[4] = {0x61707865u, 0x3320646eu, 0x79622d32u, 0x6b206574u};

static uint32_t rotl32(uint32_t v, int c) {
    return (v << c) | (v >> (32 - c));
}

static void quarterround(uint32_t x[16], int a, int b, int c, int d) {
    x[a] += x[b]; x[d] = rotl32(x[d] ^ x[a], 16);
    x[c] += x[d]; x[b] = rotl32(x[b] ^ x[c], 12);
    x[a] += x[b]; x[d] = rotl32(x[d] ^ x[a], 8);
    x[c] += x[d]; x[b] = rotl32(x[b] ^ x[c], 7);
}

static uint32_t load32_le(const uint8_t* p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static void store32_le(uint32_t v, uint8_t* p) {
    p[0] = (uint8_t)(v);
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

static void chacha20_init(chacha20_ctx* ctx, const uint8_t key[32], const uint8_t nonce[12], uint32_t counter) {
    ctx->state[0] = SIGMA[0];
    ctx->state[1] = SIGMA[1];
    ctx->state[2] = SIGMA[2];
    ctx->state[3] = SIGMA[3];
    for (int i = 0; i < 8; i++) {
        ctx->state[4 + i] = load32_le(&key[i*4]);
    }
    ctx->state[12] = counter;
    ctx->state[13] = load32_le(&nonce[0]);
    ctx->state[14] = load32_le(&nonce[4]);
    ctx->state[15] = load32_le(&nonce[8]);
    ctx->pos = 64;
}

static void chacha20_gen_block(chacha20_ctx* ctx) {
    uint32_t x[16];
    for (int i = 0; i < 16; i++) x[i] = ctx->state[i];
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
        uint32_t o = x[i] + ctx->state[i];
        store32_le(o, &ctx->buffer[i*4]);
    }
    ctx->state[12] += 1;
    ctx->pos = 0;
}

static void chacha20_xor_inplace(chacha20_ctx* ctx, uint8_t* data, size_t len) {
    size_t i = 0;
    while (i < len) {
        if (ctx->pos >= 64) chacha20_gen_block(ctx);
        size_t take = (len - i < 64 - ctx->pos) ? (len - i) : (64 - ctx->pos);
        for (size_t j = 0; j < take; j++) {
            data[i + j] ^= ctx->buffer[ctx->pos + j];
        }
        ctx->pos += take;
        i += take;
    }
}

uint8_t* chacha20_xor_bytes(const uint8_t key[32], const uint8_t nonce[12], uint32_t counter, const uint8_t* data, size_t len, size_t* out_len) {
    if (!data || !out_len) return NULL;
    uint8_t* out = (uint8_t*)malloc(len);
    if (!out) return NULL;
    memcpy(out, data, len);
    chacha20_ctx ctx;
    chacha20_init(&ctx, key, nonce, counter);
    chacha20_xor_inplace(&ctx, out, len);
    *out_len = len;
    return out;
}

int stream_xor_file(const char* inputPath, const char* outputPath, const uint8_t key[32], const uint8_t nonce[12], uint32_t counter) {
    FILE* in = fopen(inputPath, "rb");
    if (!in) return 0;
    FILE* out = fopen(outputPath, "wb");
    if (!out) { fclose(in); return 0; }
    chacha20_ctx ctx;
    chacha20_init(&ctx, key, nonce, counter);
    uint8_t* buf = (uint8_t*)malloc(8192);
    if (!buf) { fclose(in); fclose(out); return 0; }
    size_t n;
    while ((n = fread(buf, 1, 8192, in)) > 0) {
        chacha20_xor_inplace(&ctx, buf, n);
        if (fwrite(buf, 1, n, out) != n) { free(buf); fclose(in); fclose(out); return 0; }
    }
    free(buf);
    fclose(in);
    fclose(out);
    return 1;
}

int encrypt_file(const char* inputPath, const char* outputPath, const uint8_t key[32], const uint8_t nonce[12]) {
    return stream_xor_file(inputPath, outputPath, key, nonce, 1);
}

int decrypt_file(const char* inputPath, const char* outputPath, const uint8_t key[32], const uint8_t nonce[12]) {
    return stream_xor_file(inputPath, outputPath, key, nonce, 1);
}

static uint8_t* pattern_bytes(size_t n) {
    uint8_t* p = (uint8_t*)malloc(n);
    if (!p) return NULL;
    for (size_t i = 0; i < n; i++) p[i] = (uint8_t)(i & 0xff);
    return p;
}

int main(void) {
    uint8_t key[32], nonce[12];
    for (int i = 0; i < 32; i++) key[i] = (uint8_t)((i * 7 + 3) & 0xff);
    for (int i = 0; i < 12; i++) nonce[i] = (uint8_t)((i * 5 + 1) & 0xff);

    const char* tmp =
#ifdef _WIN32
        getenv("TEMP") ? getenv("TEMP") : ".";
#else
        getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp";
#endif

    size_t sizes[5] = {0, 13, 65, 1000, 50000};
    int all_ok = 1;

    for (int t = 0; t < 5; t++) {
        char in[512], enc[512], dec[512];
        snprintf(in, sizeof(in), "%s/c_in_%d.bin", tmp, t);
        snprintf(enc, sizeof(enc), "%s/c_enc_%d.bin", tmp, t);
        snprintf(dec, sizeof(dec), "%s/c_dec_%d.bin", tmp, t);

        uint8_t* content;
        size_t clen;
        if (t == 1) {
            const char* msg = "Hello, world!";
            clen = strlen(msg);
            content = (uint8_t*)malloc(clen);
            memcpy(content, msg, clen);
        } else {
            clen = sizes[t];
            content = pattern_bytes(clen);
        }

        FILE* f = fopen(in, "wb");
        if (!f) { free(content); all_ok = 0; continue; }
        fwrite(content, 1, clen, f);
        fclose(f);

        int ok1 = encrypt_file(in, enc, key, nonce);
        int ok2 = decrypt_file(enc, dec, key, nonce);

        int ok3 = 0;
        if (ok1 && ok2) {
            f = fopen(dec, "rb");
            if (f) {
                uint8_t* back = (uint8_t*)malloc(clen);
                size_t read = fread(back, 1, clen, f);
                fclose(f);
                ok3 = (read == clen) && (memcmp(back, content, clen) == 0);
                free(back);
            }
        }

        printf("Test %d: %s\n", t, (ok1 && ok2 && ok3) ? "OK" : "FAIL");
        all_ok &= (ok1 && ok2 && ok3);

        remove(in);
        remove(enc);
        remove(dec);
        free(content);
    }
    printf("All tests: %s\n", all_ok ? "PASS" : "FAIL");
    return all_ok ? 0 : 1;
}