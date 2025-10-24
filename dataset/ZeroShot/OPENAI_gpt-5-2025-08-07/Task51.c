#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define DELTA_C 0x9E3779B9u

static int hex_to_bytes(const char* hex, uint8_t** out, size_t* outlen) {
    if (!hex || !out || !outlen) return 0;
    size_t len = strlen(hex);
    if (len % 2 != 0) return 0;
    size_t n = len / 2;
    uint8_t* buf = (uint8_t*)malloc(n ? n : 1);
    if (!buf) return 0;
    for (size_t i = 0; i < n; ++i) {
        char h = hex[2 * i], l = hex[2 * i + 1];
        int hi = isdigit((unsigned char)h) ? h - '0' :
                 (h >= 'a' && h <= 'f') ? (h - 'a' + 10) :
                 (h >= 'A' && h <= 'F') ? (h - 'A' + 10) : -1;
        int lo = isdigit((unsigned char)l) ? l - '0' :
                 (l >= 'a' && l <= 'f') ? (l - 'a' + 10) :
                 (l >= 'A' && l <= 'F') ? (l - 'A' + 10) : -1;
        if (hi < 0 || lo < 0) { free(buf); return 0; }
        buf[i] = (uint8_t)((hi << 4) | lo);
    }
    *out = buf;
    *outlen = n;
    return 1;
}

static char* bytes_to_hex(const uint8_t* data, size_t len) {
    static const char* HEX = "0123456789abcdef";
    if (!data) return NULL;
    size_t n = len * 2;
    char* s = (char*)malloc(n + 1);
    if (!s) return NULL;
    for (size_t i = 0; i < len; ++i) {
        s[2 * i]     = HEX[data[i] >> 4];
        s[2 * i + 1] = HEX[data[i] & 0x0F];
    }
    s[n] = '\0';
    return s;
}

static int parse_hex_key(const char* keyHex, uint32_t k[4]) {
    if (!keyHex || strlen(keyHex) != 32) return 0;
    uint8_t* kb = NULL; size_t kblen = 0;
    if (!hex_to_bytes(keyHex, &kb, &kblen) || kblen != 16) {
        if (kb) free(kb);
        return 0;
    }
    for (int i = 0; i < 4; ++i) {
        size_t j = (size_t)i * 4;
        k[i] = (uint32_t)kb[j] |
               ((uint32_t)kb[j + 1] << 8) |
               ((uint32_t)kb[j + 2] << 16) |
               ((uint32_t)kb[j + 3] << 24);
    }
    free(kb);
    return 1;
}

static uint32_t* to_uint32_array(const uint8_t* data, size_t len, int include_length, size_t* out_len) {
    if (!data || !out_len) return NULL;
    size_t n = (len + 3) >> 2;
    size_t size = include_length ? (n + 1) : (n == 0 ? 1 : n);
    uint32_t* v = (uint32_t*)calloc(size, sizeof(uint32_t));
    if (!v) return NULL;
    for (size_t i = 0; i < len; ++i) {
        v[i >> 2] |= ((uint32_t)data[i] & 0xFFu) << ((i & 3) << 3);
    }
    if (include_length) {
        v[size - 1] = (uint32_t)len;
    }
    *out_len = size;
    return v;
}

static uint8_t* to_byte_array(const uint32_t* v, size_t vlen, int include_length, size_t* out_len) {
    if (!v || !out_len) return NULL;
    size_t n = vlen << 2;
    size_t m = n;
    if (include_length) {
        if (vlen == 0) {
            *out_len = 0;
            return (uint8_t*)calloc(1, 1);
        }
        uint32_t len = v[vlen - 1];
        if (len > n) return NULL;
        m = len;
    }
    uint8_t* out = (uint8_t*)malloc(m ? m : 1);
    if (!out) return NULL;
    for (size_t i = 0; i < m; ++i) {
        out[i] = (uint8_t)((v[i >> 2] >> ((i & 3) << 3)) & 0xFFu);
    }
    *out_len = m;
    return out;
}

static void xxtea_encrypt(uint32_t* v, size_t n, const uint32_t k[4]) {
    if (!v || n < 2) return;
    uint32_t rounds = 6u + (uint32_t)(52u / n);
    uint32_t sum = 0;
    uint32_t z = v[n - 1], y;
    while (rounds--) {
        sum += DELTA_C;
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

static void xxtea_decrypt(uint32_t* v, size_t n, const uint32_t k[4]) {
    if (!v || n < 2) return;
    uint32_t rounds = 6u + (uint32_t)(52u / n);
    uint32_t sum = rounds * DELTA_C;
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
        sum -= DELTA_C;
    }
}

char* encrypt(const char* plaintext, const char* keyHex) {
    if (!plaintext || !keyHex) return strdup("");
    uint32_t k[4];
    if (!parse_hex_key(keyHex, k)) return strdup("");
    size_t len = strlen(plaintext);
    if (len == 0) return strdup("");
    size_t vlen = 0;
    uint32_t* v = to_uint32_array((const uint8_t*)plaintext, len, 1, &vlen);
    if (!v) return strdup("");
    xxtea_encrypt(v, vlen, k);
    size_t outlen = 0;
    uint8_t* out = to_byte_array(v, vlen, 0, &outlen);
    free(v);
    if (!out) return strdup("");
    char* hex = bytes_to_hex(out, outlen);
    free(out);
    if (!hex) return strdup("");
    return hex;
}

char* decrypt(const char* cipherHex, const char* keyHex) {
    if (!cipherHex || !keyHex) return strdup("");
    uint32_t k[4];
    if (!parse_hex_key(keyHex, k)) return strdup("");
    uint8_t* enc = NULL; size_t enclen = 0;
    if (!hex_to_bytes(cipherHex, &enc, &enclen) || enclen == 0) {
        if (enc) free(enc);
        return strdup("");
    }
    size_t vlen = 0;
    uint32_t* v = to_uint32_array(enc, enclen, 0, &vlen);
    free(enc);
    if (!v) return strdup("");
    xxtea_decrypt(v, vlen, k);
    size_t outlen = 0;
    uint8_t* out = to_byte_array(v, vlen, 1, &outlen);
    free(v);
    if (!out) return strdup("");
    char* pt = (char*)malloc(outlen + 1);
    if (!pt) { free(out); return strdup(""); }
    memcpy(pt, out, outlen);
    pt[outlen] = '\0';
    free(out);
    return pt;
}

int main(void) {
    const char* key = "00112233445566778899aabbccddeeff";
    const char* tests[5] = {
        "",
        "hello",
        "The quick brown fox jumps over the lazy dog.",
        "1234567890",
        "Sensitive data!"
    };
    for (int i = 0; i < 5; ++i) {
        char* enc = encrypt(tests[i], key);
        char* dec = decrypt(enc, key);
        printf("PT: %s\n", tests[i]);
        printf("CT: %s\n", enc);
        printf("DC: %s\n", dec);
        printf("----\n");
        free(enc);
        free(dec);
    }
    return 0;
}