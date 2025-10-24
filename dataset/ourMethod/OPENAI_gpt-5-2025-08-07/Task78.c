#include <openssl/rsa.h>
#include <openssl/bn.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    unsigned char* data;
    size_t len;
    size_t cap;
} buffer_t;

static void buf_init(buffer_t* b) {
    b->data = NULL;
    b->len = 0;
    b->cap = 0;
}

static void buf_free(buffer_t* b) {
    if (b->data) {
        // Best effort cleanse
        memset(b->data, 0, b->len);
        free(b->data);
    }
    b->data = NULL;
    b->len = 0;
    b->cap = 0;
}

static int buf_reserve(buffer_t* b, size_t add) {
    if (b->len + add <= b->cap) return 1;
    size_t ncap = b->cap ? b->cap : 256;
    while (ncap < b->len + add) ncap *= 2;
    unsigned char* nd = (unsigned char*)realloc(b->data, ncap);
    if (!nd) return 0;
    b->data = nd;
    b->cap = ncap;
    return 1;
}

static int buf_append(buffer_t* b, const void* data, size_t n) {
    if (!buf_reserve(b, n)) return 0;
    memcpy(b->data + b->len, data, n);
    b->len += n;
    return 1;
}

static int write_uint32(buffer_t* b, uint32_t v) {
    unsigned char tmp[4];
    tmp[0] = (unsigned char)((v >> 24) & 0xFF);
    tmp[1] = (unsigned char)((v >> 16) & 0xFF);
    tmp[2] = (unsigned char)((v >> 8) & 0xFF);
    tmp[3] = (unsigned char)(v & 0xFF);
    return buf_append(b, tmp, 4);
}

static int write_string_buf(buffer_t* b, const unsigned char* data, size_t n) {
    if (!write_uint32(b, (uint32_t)n)) return 0;
    if (n == 0) return 1;
    return buf_append(b, data, n);
}

static int write_string_cstr(buffer_t* b, const char* s) {
    size_t n = strlen(s);
    return write_string_buf(b, (const unsigned char*)s, n);
}

static int write_mpint(buffer_t* b, const BIGNUM* bn) {
    if (BN_is_zero(bn)) {
        return write_uint32(b, 0);
    }
    int num_bytes = BN_num_bytes(bn);
    unsigned char* tmp = (unsigned char*)malloc((size_t)num_bytes);
    if (!tmp) return 0;
    BN_bn2bin(bn, tmp);
    int need_zero = (tmp[0] & 0x80) != 0;
    uint32_t outlen = (uint32_t)num_bytes + (need_zero ? 1u : 0u);
    int ok = write_uint32(b, outlen);
    if (!ok) { free(tmp); return 0; }
    if (need_zero) {
        unsigned char z = 0;
        ok = buf_append(b, &z, 1);
        if (!ok) { free(tmp); return 0; }
    }
    ok = buf_append(b, tmp, (size_t)num_bytes);
    free(tmp);
    return ok;
}

static int build_public_blob(const BIGNUM* e, const BIGNUM* n, buffer_t* out) {
    buffer_t b; buf_init(&b);
    int ok = write_string_cstr(&b, "ssh-rsa")
          && write_mpint(&b, e)
          && write_mpint(&b, n);
    if (!ok) { buf_free(&b); return 0; }
    ok = write_string_buf(out, b.data, b.len);
    buf_free(&b);
    return ok;
}

static int build_private_block(const BIGNUM* n, const BIGNUM* e, const BIGNUM* d, const BIGNUM* iqmp, const BIGNUM* p, const BIGNUM* q, const char* comment, buffer_t* out) {
    buffer_t b; buf_init(&b);
    unsigned int check;
    RAND_bytes((unsigned char*)&check, sizeof(check));
    int ok = write_uint32(&b, check)
          && write_uint32(&b, check)
          && write_string_cstr(&b, "ssh-rsa")
          && write_mpint(&b, n)
          && write_mpint(&b, e)
          && write_mpint(&b, d)
          && write_mpint(&b, iqmp)
          && write_mpint(&b, p)
          && write_mpint(&b, q)
          && write_string_cstr(&b, comment ? comment : "");
    if (!ok) { buf_free(&b); return 0; }

    size_t pad_len = 8 - (b.len % 8);
    if (pad_len == 0) pad_len = 8;
    for (size_t i = 1; i <= pad_len; ++i) {
        unsigned char c = (unsigned char)i;
        if (!buf_append(&b, &c, 1)) { buf_free(&b); return 0; }
    }
    ok = write_string_buf(out, b.data, b.len);
    buf_free(&b);
    return ok;
}

static char* b64_wrap(const unsigned char* data, size_t len) {
    int outlen = 4 * ((int)len + 2) / 3;
    unsigned char* enc = (unsigned char*)malloc((size_t)outlen + 1);
    if (!enc) return NULL;
    int written = EVP_EncodeBlock(enc, data, (int)len);
    enc[written] = '\0';

    // Wrap at 70 chars
    size_t lines = (written + 69) / 70;
    size_t wrapped_len = written + lines; // account for '\n'
    char* wrapped = (char*)malloc(wrapped_len + 1);
    if (!wrapped) { free(enc); return NULL; }
    size_t pos = 0;
    for (int i = 0; i < written; i += 70) {
        int chunk = (i + 70 <= written) ? 70 : (written - i);
        memcpy(wrapped + pos, enc + i, (size_t)chunk);
        pos += (size_t)chunk;
        wrapped[pos++] = '\n';
    }
    wrapped[pos] = '\0';
    free(enc);
    return wrapped;
}

char* generate_openssh_rsa_private_key(int bits, const char* comment) {
    if (bits < 2048 || bits > 8192) {
        return NULL;
    }
    if (comment && strlen(comment) > 1024) {
        return NULL;
    }

    BIGNUM* e_bn = BN_new();
    if (!e_bn) return NULL;
    if (!BN_set_word(e_bn, RSA_F4)) { BN_free(e_bn); return NULL; }

    RSA* rsa = RSA_new();
    if (!rsa) { BN_free(e_bn); return NULL; }
    if (RSA_generate_key_ex(rsa, bits, e_bn, NULL) != 1) {
        RSA_free(rsa); BN_free(e_bn); return NULL;
    }
    BN_free(e_bn);

    const BIGNUM* n = NULL; const BIGNUM* e = NULL; const BIGNUM* d = NULL;
    const BIGNUM* p = NULL; const BIGNUM* q = NULL;
    const BIGNUM* dmp1 = NULL; const BIGNUM* dmq1 = NULL; const BIGNUM* iqmp = NULL;
    RSA_get0_key(rsa, &n, &e, &d);
    RSA_get0_factors(rsa, &p, &q);
    RSA_get0_crt_params(rsa, &dmp1, &dmq1, &iqmp);
    if (!n || !e || !d || !p || !q || !iqmp) {
        RSA_free(rsa);
        return NULL;
    }

    buffer_t out; buf_init(&out);
    // magic
    const char* magic = "openssh-key-v1";
    if (!buf_append(&out, magic, strlen(magic)) || !buf_append(&out, "\x00", 1)) { buf_free(&out); RSA_free(rsa); return NULL; }
    // ciphername, kdfname, kdfoptions
    if (!write_string_cstr(&out, "none") || !write_string_cstr(&out, "none") || !write_string_buf(&out, (const unsigned char*)"", 0)) { buf_free(&out); RSA_free(rsa); return NULL; }
    // number of keys
    if (!write_uint32(&out, 1)) { buf_free(&out); RSA_free(rsa); return NULL; }
    // public keys
    if (!build_public_blob(e, n, &out)) { buf_free(&out); RSA_free(rsa); return NULL; }
    // private key block
    if (!build_private_block(n, e, d, iqmp, p, q, comment ? comment : "", &out)) { buf_free(&out); RSA_free(rsa); return NULL; }

    RSA_free(rsa);

    char* b64 = b64_wrap(out.data, out.len);
    buf_free(&out);
    if (!b64) return NULL;

    const char* header = "-----BEGIN OPENSSH PRIVATE KEY-----\n";
    const char* footer = "-----END OPENSSH PRIVATE KEY-----\n";
    size_t total_len = strlen(header) + strlen(b64) + strlen(footer);
    char* result = (char*)malloc(total_len + 1);
    if (!result) { free(b64); return NULL; }
    strcpy(result, header);
    strcat(result, b64);
    strcat(result, footer);
    free(b64);
    return result;
}

int main(void) {
    const char* comments[5] = {"test1", "user@example", "device-key", "backup", "prod-key"};
    int sizes[5] = {2048, 2048, 3072, 4096, 2048};
    for (int i = 0; i < 5; ++i) {
        char* key = generate_openssh_rsa_private_key(sizes[i], comments[i]);
        int ok = (key && strncmp(key, "-----BEGIN OPENSSH PRIVATE KEY-----", 35) == 0);
        printf("Case %d: generated=%s, bits=%d, length=%zu\n", i + 1, ok ? "true" : "false", sizes[i], key ? strlen(key) : 0u);
        if (key) {
            // Do not print the key; just free it
            memset(key, 0, strlen(key));
            free(key);
        }
    }
    return 0;
}