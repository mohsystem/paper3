#include <openssl/rsa.h>
#include <openssl/bn.h>
#include <openssl/evp.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    unsigned char* data;
    size_t len;
    size_t cap;
} Buf;

static void buf_init(Buf* b) {
    b->data = NULL; b->len = 0; b->cap = 0;
}
static void buf_free(Buf* b) {
    if (b->data) free(b->data);
    b->data = NULL; b->len = b->cap = 0;
}
static int buf_reserve(Buf* b, size_t need) {
    if (b->len + need <= b->cap) return 1;
    size_t ncap = b->cap ? b->cap : 256;
    while (ncap < b->len + need) ncap *= 2;
    unsigned char* nd = (unsigned char*)realloc(b->data, ncap);
    if (!nd) return 0;
    b->data = nd; b->cap = ncap;
    return 1;
}
static int buf_append(Buf* b, const void* data, size_t n) {
    if (!buf_reserve(b, n)) return 0;
    memcpy(b->data + b->len, data, n);
    b->len += n;
    return 1;
}
static int buf_append_u32(Buf* b, uint32_t v) {
    unsigned char tmp[4];
    tmp[0] = (v >> 24) & 0xFF;
    tmp[1] = (v >> 16) & 0xFF;
    tmp[2] = (v >> 8) & 0xFF;
    tmp[3] = v & 0xFF;
    return buf_append(b, tmp, 4);
}
static int buf_append_string(Buf* b, const char* s) {
    uint32_t n = (uint32_t)strlen(s);
    if (!buf_append_u32(b, n)) return 0;
    if (!buf_append(b, s, n)) return 0;
    return 1;
}
static int buf_append_string_bytes(Buf* b, const unsigned char* data, size_t n) {
    if (!buf_append_u32(b, (uint32_t)n)) return 0;
    if (n && !buf_append(b, data, n)) return 0;
    return 1;
}
static int buf_append_mpint(Buf* b, const BIGNUM* bn) {
    if (BN_is_zero(bn)) {
        return buf_append_u32(b, 0);
    }
    int nbytes = BN_num_bytes(bn);
    unsigned char* tmp = (unsigned char*)malloc((size_t)nbytes + 1);
    if (!tmp) return 0;
    BN_bn2bin(bn, tmp);
    size_t out_len = (size_t)nbytes;
    if (tmp[0] & 0x80) {
        memmove(tmp + 1, tmp, out_len);
        tmp[0] = 0x00;
        out_len += 1;
    }
    int ok = buf_append_string_bytes(b, tmp, out_len);
    free(tmp);
    return ok;
}

static char* wrap_b64(const char* in, size_t len, size_t width) {
    size_t lines = (len + width - 1) / width;
    size_t out_len = len + lines - 1;
    char* out = (char*)malloc(out_len + 1);
    if (!out) return NULL;
    size_t pos = 0;
    size_t i = 0;
    while (i < len) {
        size_t n = (len - i > width) ? width : (len - i);
        memcpy(out + pos, in + i, n);
        pos += n;
        i += n;
        if (i < len) out[pos++] = '\n';
    }
    out[pos] = '\0';
    return out;
}

char* generate_openssh_rsa_private_key(int bits, const char* comment) {
    RSA* rsa = RSA_new();
    BIGNUM* e = BN_new();
    if (!rsa || !e) { if (rsa) RSA_free(rsa); if (e) BN_free(e); return NULL; }
    BN_set_word(e, RSA_F4);
    if (RSA_generate_key_ex(rsa, bits, e, NULL) != 1) {
        BN_free(e); RSA_free(rsa); return NULL;
    }

    const BIGNUM *n, *e_bn, *d, *p, *q, *dmp1, *dmq1, *iqmp;
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    RSA_get0_key(rsa, &n, &e_bn, &d);
    RSA_get0_factors(rsa, &p, &q);
    RSA_get0_crt_params(rsa, &dmp1, &dmq1, &iqmp);
#else
    n = rsa->n; e_bn = rsa->e; d = rsa->d; p = rsa->p; q = rsa->q; dmp1 = rsa->dmp1; dmq1 = rsa->dmq1; iqmp = rsa->iqmp;
#endif

    Buf pub; buf_init(&pub);
    if (!buf_append_string(&pub, "ssh-rsa")) goto err;
    if (!buf_append_mpint(&pub, e_bn)) goto err;
    if (!buf_append_mpint(&pub, n)) goto err;

    Buf priv; buf_init(&priv);
    uint32_t check = (uint32_t)rand() ^ ((uint32_t)rand() << 16);
    if (!buf_append_u32(&priv, check)) goto err;
    if (!buf_append_u32(&priv, check)) goto err;
    if (!buf_append_string(&priv, "ssh-rsa")) goto err;
    if (!buf_append_mpint(&priv, n)) goto err;
    if (!buf_append_mpint(&priv, e_bn)) goto err;
    if (!buf_append_mpint(&priv, d)) goto err;
    if (!buf_append_mpint(&priv, iqmp)) goto err;
    if (!buf_append_mpint(&priv, p)) goto err;
    if (!buf_append_mpint(&priv, q)) goto err;
    if (!buf_append_string(&priv, comment ? comment : "")) goto err;
    {
        size_t block = 8;
        size_t pad = (block - (priv.len % block)) % block;
        for (size_t i = 1; i <= pad; ++i) {
            unsigned char c = (unsigned char)i;
            if (!buf_append(&priv, &c, 1)) goto err;
        }
    }

    Buf all; buf_init(&all);
    if (!buf_append(&all, "openssh-key-v1", strlen("openssh-key-v1"))) goto err_all;
    {
        unsigned char zero = 0;
        if (!buf_append(&all, &zero, 1)) goto err_all;
    }
    if (!buf_append_string(&all, "none")) goto err_all;
    if (!buf_append_string(&all, "none")) goto err_all;
    if (!buf_append_string_bytes(&all, NULL, 0)) goto err_all;
    if (!buf_append_u32(&all, 1)) goto err_all;
    if (!buf_append_string_bytes(&all, pub.data, pub.len)) goto err_all;
    if (!buf_append_string_bytes(&all, priv.data, priv.len)) goto err_all;

    int inlen = (int)all.len;
    int outlen = 4 * ((inlen + 2) / 3);
    unsigned char* b64 = (unsigned char*)malloc((size_t)outlen + 1);
    if (!b64) goto err_all;
    int actual = EVP_EncodeBlock(b64, all.data, inlen);
    b64[actual] = '\0';

    char* wrapped = wrap_b64((char*)b64, (size_t)actual, 70);
    free(b64);
    if (!wrapped) goto err_all;

    const char* header = "-----BEGIN OPENSSH PRIVATE KEY-----\n";
    const char* footer = "\n-----END OPENSSH PRIVATE KEY-----\n";
    size_t total_len = strlen(header) + strlen(wrapped) + strlen(footer);
    char* out = (char*)malloc(total_len + 1);
    if (!out) { free(wrapped); goto err_all; }
    strcpy(out, header);
    strcat(out, wrapped);
    strcat(out, footer);
    free(wrapped);

    BN_free(e);
    RSA_free(rsa);
    buf_free(&pub);
    buf_free(&priv);
    buf_free(&all);
    return out;

err:
    buf_free(&pub);
    buf_free(&priv);
    BN_free(e);
    RSA_free(rsa);
    return NULL;

err_all:
    BN_free(e);
    RSA_free(rsa);
    buf_free(&pub);
    buf_free(&priv);
    buf_free(&all);
    return NULL;
}

int main() {
    char* k1 = generate_openssh_rsa_private_key(2048, "c-test-1");
    char* k2 = generate_openssh_rsa_private_key(1024, "c-test-2");
    char* k3 = generate_openssh_rsa_private_key(3072, "c-test-3");
    char* k4 = generate_openssh_rsa_private_key(4096, "c-test-4");
    char* k5 = generate_openssh_rsa_private_key(1536, "c-test-5");

    if (k1) { printf("%s\n", k1); free(k1); }
    if (k2) { printf("%s\n", k2); free(k2); }
    if (k3) { printf("%s\n", k3); free(k3); }
    if (k4) { printf("%s\n", k4); free(k4); }
    if (k5) { printf("%s\n", k5); free(k5); }

    return 0;
}