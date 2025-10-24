#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <openssl/evp.h>
#include <openssl/crypto.h>

typedef struct {
    unsigned char* data;
    size_t len;
} bytes_t;

static void free_bytes(bytes_t b) {
    if (b.data) {
        OPENSSL_cleanse(b.data, b.len);
        free(b.data);
    }
}

static const EVP_MD* normalize_alg(const char* alg) {
    if (!alg) {
        return NULL;
    }
    // Uppercase copy
    size_t n = strlen(alg);
    char* up = (char*)malloc(n + 1);
    if (!up) return NULL;
    for (size_t i = 0; i < n; ++i) up[i] = (char)toupper((unsigned char)alg[i]);
    up[n] = '\0';

    const EVP_MD* md = NULL;
    if (strcmp(up, "SHA-256") == 0 || strcmp(up, "SHA256") == 0) md = EVP_sha256();
    else if (strcmp(up, "SHA-384") == 0 || strcmp(up, "SHA384") == 0) md = EVP_sha384();
    else if (strcmp(up, "SHA-512") == 0 || strcmp(up, "SHA512") == 0) md = EVP_sha512();

    free(up);
    return md;
}

static int hex_to_bytes(const char* hex, bytes_t* out) {
    if (!hex || !out) return 0;
    // count hex digits
    size_t count = 0;
    for (const char* p = hex; *p; ++p) {
        if (isxdigit((unsigned char)*p)) count++;
    }
    if (count == 0 || (count % 2) != 0) return 0;
    size_t len = count / 2;
    unsigned char* buf = (unsigned char*)malloc(len);
    if (!buf) return 0;

    size_t idx = 0;
    int hi = -1;
    for (const char* p = hex; *p; ++p) {
        if (!isxdigit((unsigned char)*p)) continue;
        int v = isdigit((unsigned char)*p) ? *p - '0' : (toupper((unsigned char)*p) - 'A' + 10);
        if (hi < 0) hi = v;
        else {
            buf[idx++] = (unsigned char)((hi << 4) | v);
            hi = -1;
        }
    }
    if (idx != len) {
        OPENSSL_cleanse(buf, len);
        free(buf);
        return 0;
    }
    out->data = buf;
    out->len = len;
    return 1;
}

static char* bytes_to_hex(const unsigned char* data, size_t len) {
    static const char* hexd = "0123456789ABCDEF";
    char* out = (char*)malloc(len * 2 + 1);
    if (!out) return NULL;
    for (size_t i = 0; i < len; ++i) {
        out[2*i] = hexd[(data[i] >> 4) & 0xF];
        out[2*i+1] = hexd[data[i] & 0xF];
    }
    out[len*2] = '\0';
    return out;
}

static int compute_digest(const unsigned char* der, size_t len, const char* alg, bytes_t* out) {
    if (!der || len == 0 || !out) return 0;
    const EVP_MD* md = normalize_alg(alg);
    if (!md) return 0;

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) return 0;

    unsigned char* buf = (unsigned char*)malloc((size_t)EVP_MD_size(md));
    if (!buf) {
        EVP_MD_CTX_free(ctx);
        return 0;
    }
    unsigned int olen = 0;

    int ok = (EVP_DigestInit_ex(ctx, md, NULL) == 1) &&
             (EVP_DigestUpdate(ctx, der, len) == 1) &&
             (EVP_DigestFinal_ex(ctx, buf, &olen) == 1);

    EVP_MD_CTX_free(ctx);

    if (!ok) {
        OPENSSL_cleanse(buf, (size_t)EVP_MD_size(md));
        free(buf);
        return 0;
    }
    out->data = buf;
    out->len = (size_t)olen;
    return 1;
}

static int constant_time_equals(const unsigned char* a, const unsigned char* b, size_t len) {
    return CRYPTO_memcmp(a, b, len) == 0;
}

static int pem_to_der_if_pem(const char* s, bytes_t* out) {
    if (!s || !out) return 0;
    const char* begin = "-----BEGIN CERTIFICATE-----";
    const char* end = "-----END CERTIFICATE-----";
    const char* pBegin = strstr(s, begin);
    const char* pEnd = strstr(s, end);
    if (!pBegin || !pEnd || pEnd < pBegin) return 0;
    pBegin += strlen(begin);

    // Extract base64 into cleaned buffer
    size_t cap = (size_t)(pEnd - pBegin);
    char* cleaned = (char*)malloc(cap + 1);
    if (!cleaned) return 0;
    size_t idx = 0;
    for (const char* p = pBegin; p < pEnd; ++p) {
        char c = *p;
        if ((c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
            c == '+' || c == '/' || c == '=') {
            cleaned[idx++] = c;
        }
    }
    if (idx == 0) {
        free(cleaned);
        return 0;
    }
    cleaned[idx] = '\0';

    // Decode base64 with OpenSSL EVP_DecodeBlock
    size_t outcap = ((idx + 3) / 4) * 3;
    unsigned char* der = (unsigned char*)malloc(outcap);
    if (!der) {
        free(cleaned);
        return 0;
    }
    int decodedLen = EVP_DecodeBlock(der, (const unsigned char*)cleaned, (int)idx);
    if (decodedLen < 0) {
        free(cleaned);
        OPENSSL_cleanse(der, outcap);
        free(der);
        return 0;
    }
    // Adjust for '=' padding
    int pad = 0;
    if (idx >= 1 && cleaned[idx-1] == '=') pad++;
    if (idx >= 2 && cleaned[idx-2] == '=') pad++;
    decodedLen -= pad;
    if (decodedLen < 0) decodedLen = 0;

    free(cleaned);
    out->data = der;
    out->len = (size_t)decodedLen;
    return 1;
}

int certificate_matches_hash(const unsigned char* certBytes, size_t certLen, const char* expectedHex, const char* alg) {
    if (!certBytes || certLen == 0 || !expectedHex || !alg) return 0;
    bytes_t expected = {0};
    bytes_t actual = {0};
    int result = 0;

    if (!hex_to_bytes(expectedHex, &expected)) goto cleanup;
    if (!compute_digest(certBytes, certLen, alg, &actual)) goto cleanup;
    if (expected.len != actual.len) goto cleanup;
    result = constant_time_equals(actual.data, expected.data, actual.len);

cleanup:
    free_bytes(expected);
    free_bytes(actual);
    return result;
}

int certificate_matches_hash_from_pem(const char* pem, const char* expectedHex, const char* alg) {
    if (!pem || !expectedHex || !alg) return 0;
    bytes_t der = {0};
    int result = 0;
    if (!pem_to_der_if_pem(pem, &der)) {
        free_bytes(der);
        return 0;
    }
    result = certificate_matches_hash(der.data, der.len, expectedHex, alg);
    free_bytes(der);
    return result;
}

char* compute_cert_digest_hex(const unsigned char* der, size_t derLen, const char* alg) {
    bytes_t d = {0};
    if (!compute_digest(der, derLen, alg, &d)) return NULL;
    char* hex = bytes_to_hex(d.data, d.len);
    free_bytes(d);
    return hex; // caller must free
}

static char* colonize_hex(const char* hex) {
    // Build AA:BB:...
    size_t n = strlen(hex);
    // First, clean to hex digits
    char* cleaned = (char*)malloc(n + 1);
    if (!cleaned) return NULL;
    size_t idx = 0;
    for (size_t i = 0; i < n; ++i) {
        if (isxdigit((unsigned char)hex[i])) {
            cleaned[idx++] = (char)toupper((unsigned char)hex[i]);
        }
    }
    cleaned[idx] = '\0';
    if (idx == 0) {
        free(cleaned);
        return NULL;
    }
    size_t outlen = idx + (idx/2 - 1);
    char* out = (char*)malloc(outlen + 1);
    if (!out) {
        free(cleaned);
        return NULL;
    }
    size_t o = 0;
    for (size_t i = 0; i < idx; i += 2) {
        if (i > 0) out[o++] = ':';
        out[o++] = cleaned[i];
        if (i + 1 < idx) out[o++] = cleaned[i+1];
    }
    out[o] = '\0';
    free(cleaned);
    return out;
}

static char* b64encode(const unsigned char* data, size_t len, size_t lineWidth) {
    // Use EVP_EncodeBlock, then insert line breaks
    size_t encCap = ((len + 2) / 3) * 4 + 4;
    unsigned char* enc = (unsigned char*)malloc(encCap);
    if (!enc) return NULL;
    int n = EVP_EncodeBlock(enc, data, (int)len);
    if (n < 0) {
        free(enc);
        return NULL;
    }
    char* tmp = (char*)malloc((size_t)n + 1);
    if (!tmp) { free(enc); return NULL; }
    memcpy(tmp, enc, (size_t)n);
    tmp[n] = '\0';
    free(enc);

    if (lineWidth == 0) return tmp;

    size_t outCap = (size_t)n + (n / lineWidth) + 8;
    char* out = (char*)malloc(outCap);
    if (!out) { free(tmp); return NULL; }

    size_t o = 0;
    for (size_t i = 0; i < (size_t)n; i += lineWidth) {
        size_t chunk = (i + lineWidth <= (size_t)n) ? lineWidth : ((size_t)n - i);
        memcpy(out + o, tmp + i, chunk);
        o += chunk;
        if (i + lineWidth < (size_t)n) out[o++] = '\n';
    }
    out[o] = '\0';
    free(tmp);
    return out;
}

int main(void) {
    // Test 1: DER match, SHA-256
    const unsigned char cert1[] = "DERCERT-ONE";
    char* expected1 = compute_cert_digest_hex(cert1, sizeof(cert1) - 1, "SHA-256");
    if (!expected1) { printf("Error computing digest\n"); return 1; }
    printf("Test1 (DER match, SHA-256): %s\n", certificate_matches_hash(cert1, sizeof(cert1) - 1, expected1, "SHA-256") ? "1" : "0");

    // Test 2: DER mismatch
    const unsigned char cert2[] = "ANOTHER-CERT";
    char wrong[65]; memset(wrong, 0, sizeof(wrong));
    for (int i = 0; i < 64; ++i) wrong[i] = '0';
    wrong[64] = '\0';
    printf("Test2 (DER mismatch): %s\n", certificate_matches_hash(cert2, sizeof(cert2) - 1, wrong, "SHA-256") ? "1" : "0");

    // Test 3: PEM match, SHA-256
    unsigned char der3[] = {1,2,3,4,5};
    char* b64 = b64encode(der3, sizeof(der3), 64);
    if (!b64) { OPENSSL_cleanse(expected1, strlen(expected1)); free(expected1); return 1; }
    size_t pemLen = strlen(b64) + 64;
    char* pem = (char*)malloc(pemLen);
    if (!pem) { free(b64); OPENSSL_cleanse(expected1, strlen(expected1)); free(expected1); return 1; }
    snprintf(pem, pemLen, "-----BEGIN CERTIFICATE-----\n%s\n-----END CERTIFICATE-----\n", b64);
    char* expected3 = compute_cert_digest_hex(der3, sizeof(der3), "SHA-256");
    printf("Test3 (PEM match, SHA-256): %s\n", certificate_matches_hash_from_pem(pem, expected3, "SHA-256") ? "1" : "0");

    // Test 4: Invalid algorithm handling
    int okInvalid = certificate_matches_hash(cert1, sizeof(cert1)-1, expected1, "MD5");
    printf("Test4 (Invalid alg): %s\n", okInvalid ? "0" : "1");

    // Test 5: Colon-separated fingerprint
    char* colon1 = colonize_hex(expected1);
    printf("Test5 (Colon-separated fingerprint): %s\n", certificate_matches_hash(cert1, sizeof(cert1)-1, colon1, "SHA-256") ? "1" : "0");

    // Cleanup
    OPENSSL_cleanse(expected1, strlen(expected1));
    free(expected1);
    free(expected3);
    free(b64);
    free(pem);
    free(colon1);
    return 0;
}