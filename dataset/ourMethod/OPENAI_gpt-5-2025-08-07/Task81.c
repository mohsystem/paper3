/*
Program: SSL Certificate Hash Checker (C, OpenSSL)
Build (example):
  gcc -Wall -Wextra -O2 -std=c11 task.c -o task -lssl -lcrypto

This program computes the SHA-256 fingerprint of an X.509 certificate (PEM or DER)
and verifies it against a known hash using a constant-time comparison.

Note: Requires OpenSSL development libraries.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/bn.h>
#include <openssl/err.h>

#ifndef O_BINARY
#define O_BINARY 0
#endif

/* Portable explicit zeroization */
static void secure_zero(void *v, size_t n) {
    if (v == NULL || n == 0) return;
    volatile unsigned char *p = (volatile unsigned char *)v;
    while (n--) {
        *p++ = 0;
    }
}

/* Constant-time compare for fixed-size buffers. Returns 1 if equal, 0 otherwise. */
static int constant_time_eq(const unsigned char *a, const unsigned char *b, size_t len) {
    if (a == NULL || b == NULL) return 0;
    unsigned char diff = 0;
    for (size_t i = 0; i < len; i++) {
        diff |= (unsigned char)(a[i] ^ b[i]);
    }
    return diff == 0 ? 1 : 0;
}

/* Convert bytes to lowercase hex string. out_len must be at least (2*len + 1). */
static int bytes_to_hex_lower(const unsigned char *in, size_t len, char *out, size_t out_len) {
    static const char *hex = "0123456789abcdef";
    if (in == NULL || out == NULL) return -1;
    if (out_len < (len * 2 + 1)) return -1;
    for (size_t i = 0; i < len; i++) {
        out[2*i]     = hex[(in[i] >> 4) & 0x0F];
        out[2*i + 1] = hex[in[i] & 0x0F];
    }
    out[len * 2] = '\0';
    return 0;
}

/* Parse hex string into bytes, ignoring ':' and whitespace. Require exact expected_len bytes. */
static int parse_hex_strict(const char *hex_in, unsigned char *out, size_t expected_len) {
    if (hex_in == NULL || out == NULL) return -1;
    size_t nibbles = 0;
    const size_t max_nibbles = expected_len * 2;
    unsigned char tmp_byte = 0;
    int have_high_nibble = 0;

    for (const char *p = hex_in; *p; p++) {
        unsigned char c = (unsigned char)*p;
        if (c == ':' || c == ' ' || c == '\t' || c == '\n' || c == '\r') continue;
        unsigned char v;
        if (c >= '0' && c <= '9') v = (unsigned char)(c - '0');
        else if (c >= 'a' && c <= 'f') v = (unsigned char)(c - 'a' + 10);
        else if (c >= 'A' && c <= 'F') v = (unsigned char)(c - 'A' + 10);
        else return -1; /* invalid char */

        if (!have_high_nibble) {
            tmp_byte = (unsigned char)(v << 4);
            have_high_nibble = 1;
            nibbles++;
        } else {
            tmp_byte |= v;
            have_high_nibble = 0;
            nibbles++;
            size_t byte_index = (nibbles / 2) - 1;
            if (byte_index >= expected_len) return -1; /* overflow */
            out[byte_index] = tmp_byte;
            tmp_byte = 0;
        }
        if (nibbles > max_nibbles) return -1;
    }
    if (have_high_nibble) return -1; /* odd number of nibbles */
    if (nibbles != max_nibbles) return -1; /* not exact size */
    return 0;
}

/* Load X509 certificate from file (PEM or DER). Caller must free with X509_free. */
static X509* load_cert_from_file(const char *path) {
    if (path == NULL) return NULL;
    FILE *fp = fopen(path, "rb");
    if (!fp) return NULL;

    ERR_clear_error();
    X509 *cert = PEM_read_X509(fp, NULL, NULL, NULL);
    if (!cert) {
        /* Try DER */
        ERR_clear_error();
        rewind(fp);
        cert = d2i_X509_fp(fp, NULL);
    }
    fclose(fp);
    return cert;
}

/* Compute SHA-256 fingerprint from X509 object. out must be 32 bytes. */
static int sha256_fingerprint_from_X509(const X509 *x, unsigned char out[32]) {
    if (!x || !out) return -1;
    unsigned int len = 0;
    if (X509_digest((X509*)x, EVP_sha256(), out, &len) != 1) {
        return -1;
    }
    if (len != 32) return -1;
    return 0;
}

/* Compute SHA-256 fingerprint from certificate file (PEM or DER). */
static int sha256_fingerprint_from_cert_file(const char *path, unsigned char out[32]) {
    if (path == NULL || out == NULL) return -1;
    X509 *x = load_cert_from_file(path);
    if (!x) return -1;
    int rc = sha256_fingerprint_from_X509(x, out);
    X509_free(x);
    return rc;
}

/* Check if certificate's SHA-256 fingerprint matches known hex string. 
   Returns:
   1 if match,
   0 if not match,
  -1 on error (invalid inputs, parse failure, or fingerprint compute error).
*/
static int check_cert_matches_known_hash_file(const char *cert_path, const char *known_sha256_hex) {
    if (cert_path == NULL || known_sha256_hex == NULL) return -1;

    unsigned char expected[32];
    if (parse_hex_strict(known_sha256_hex, expected, sizeof(expected)) != 0) {
        return -1;
    }

    unsigned char actual[32];
    if (sha256_fingerprint_from_cert_file(cert_path, actual) != 0) {
        secure_zero(expected, sizeof(expected));
        return -1;
    }

    int match = constant_time_eq(expected, actual, sizeof(expected));
    secure_zero(expected, sizeof(expected));
    secure_zero(actual, sizeof(actual));
    return match;
}

/* Helper: write X509 to a secure temp file in PEM format. Returns 0 on success, path in out_path. */
static int write_cert_pem_to_temp(const X509 *x, char *out_path, size_t out_path_len) {
    if (!x || !out_path || out_path_len < 1) return -1;
    char tmpl[] = "/tmp/certchk_pem_XXXXXX";
    int fd = mkstemp(tmpl);
    if (fd == -1) return -1;

    FILE *fp = fdopen(fd, "wb");
    if (!fp) {
        close(fd);
        unlink(tmpl);
        return -1;
    }
    int ok = PEM_write_X509(fp, (X509*)x);
    fflush(fp);
    fsync(fd);
    fclose(fp);
    if (ok != 1) {
        unlink(tmpl);
        return -1;
    }
    if (snprintf(out_path, out_path_len, "%s", tmpl) < 0) {
        unlink(tmpl);
        return -1;
    }
    return 0;
}

/* Helper: write X509 to a secure temp file in DER format. Returns 0 on success, path in out_path. */
static int write_cert_der_to_temp(const X509 *x, char *out_path, size_t out_path_len) {
    if (!x || !out_path || out_path_len < 1) return -1;
    char tmpl[] = "/tmp/certchk_der_XXXXXX";
    int fd = mkstemp(tmpl);
    if (fd == -1) return -1;

    FILE *fp = fdopen(fd, "wb");
    if (!fp) {
        close(fd);
        unlink(tmpl);
        return -1;
    }
    int ok = i2d_X509_fp(fp, (X509*)x);
    fflush(fp);
    fsync(fd);
    fclose(fp);
    if (ok != 1) {
        unlink(tmpl);
        return -1;
    }
    if (snprintf(out_path, out_path_len, "%s", tmpl) < 0) {
        unlink(tmpl);
        return -1;
    }
    return 0;
}

/* Generate a self-signed X.509 certificate with RSA-2048 key and CN=common_name. Caller must free both. */
static int generate_self_signed_cert(const char *common_name, X509 **out_cert, EVP_PKEY **out_pkey) {
    if (!common_name || !out_cert || !out_pkey) return -1;
    *out_cert = NULL;
    *out_pkey = NULL;

    int ret = -1;
    EVP_PKEY *pkey = NULL;
    RSA *rsa = NULL;
    BIGNUM *e = NULL;
    X509 *x = NULL;

    pkey = EVP_PKEY_new();
    if (!pkey) goto cleanup;
    rsa = RSA_new();
    if (!rsa) goto cleanup;
    e = BN_new();
    if (!e) goto cleanup;
    if (BN_set_word(e, RSA_F4) != 1) goto cleanup;
    if (RSA_generate_key_ex(rsa, 2048, e, NULL) != 1) goto cleanup;
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    if (EVP_PKEY_assign_RSA(pkey, rsa) != 1) goto cleanup;
    rsa = NULL; /* pkey owns rsa now */
#else
    if (EVP_PKEY_assign_RSA(pkey, rsa) != 1) goto cleanup;
    rsa = NULL; /* pkey owns rsa now */
#endif

    x = X509_new();
    if (!x) goto cleanup;

    if (X509_set_version(x, 2L) != 1) goto cleanup; /* v3 cert (version 2) */

    ASN1_INTEGER *serial = ASN1_INTEGER_new();
    if (!serial) goto cleanup;
    if (ASN1_INTEGER_set(serial, 1) != 1) {
        ASN1_INTEGER_free(serial);
        goto cleanup;
    }
    if (X509_set_serialNumber(x, serial) != 1) {
        ASN1_INTEGER_free(serial);
        goto cleanup;
    }
    ASN1_INTEGER_free(serial);

    if (!X509_gmtime_adj(X509_get_notBefore(x), 0)) goto cleanup;
    if (!X509_gmtime_adj(X509_get_notAfter(x), 31536000L)) goto cleanup; /* +365 days */

    if (X509_set_pubkey(x, pkey) != 1) goto cleanup;

    X509_NAME *name = X509_get_subject_name(x);
    if (!name) goto cleanup;

    if (X509_NAME_add_entry_by_txt(name, "C", MBSTRING_ASC, (unsigned char *)"US", -1, -1, 0) != 1) goto cleanup;
    if (X509_NAME_add_entry_by_txt(name, "O", MBSTRING_ASC, (unsigned char *)"CertCheck", -1, -1, 0) != 1) goto cleanup;
    if (X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char *)common_name, -1, -1, 0) != 1) goto cleanup;

    if (X509_set_issuer_name(x, name) != 1) goto cleanup;

    /* Add basic constraints and key usage extensions */
    X509V3_CTX ctx;
    X509V3_set_ctx(&ctx, x, x, NULL, NULL, 0);

    X509_EXTENSION *ex = NULL;
    ex = X509V3_EXT_conf_nid(NULL, &ctx, NID_basic_constraints, "CA:FALSE");
    if (!ex) goto cleanup;
    if (X509_add_ext(x, ex, -1) != 1) { X509_EXTENSION_free(ex); goto cleanup; }
    X509_EXTENSION_free(ex);

    ex = X509V3_EXT_conf_nid(NULL, &ctx, NID_key_usage, "digitalSignature,keyEncipherment");
    if (!ex) goto cleanup;
    if (X509_add_ext(x, ex, -1) != 1) { X509_EXTENSION_free(ex); goto cleanup; }
    X509_EXTENSION_free(ex);

    ex = X509V3_EXT_conf_nid(NULL, &ctx, NID_subject_key_identifier, "hash");
    if (!ex) goto cleanup;
    if (X509_add_ext(x, ex, -1) != 1) { X509_EXTENSION_free(ex); goto cleanup; }
    X509_EXTENSION_free(ex);

    if (X509_sign(x, pkey, EVP_sha256()) == 0) goto cleanup;

    *out_cert = x; x = NULL;
    *out_pkey = pkey; pkey = NULL;
    ret = 0;

cleanup:
    if (x) X509_free(x);
    if (pkey) EVP_PKEY_free(pkey);
    if (rsa) RSA_free(rsa);
    if (e) BN_free(e);
    return ret;
}

/* ——————————————— Main with 5 test cases ——————————————— */
int main(void) {
    int rc = 0;

    /* Generate a self-signed certificate for testing */
    X509 *cert1 = NULL;
    EVP_PKEY *pkey1 = NULL;
    if (generate_self_signed_cert("test1.local", &cert1, &pkey1) != 0) {
        fprintf(stderr, "Error: failed to generate test certificate 1\n");
        return 1;
    }

    unsigned char fp1[32];
    if (sha256_fingerprint_from_X509(cert1, fp1) != 0) {
        fprintf(stderr, "Error: failed to compute fingerprint for cert1\n");
        X509_free(cert1);
        EVP_PKEY_free(pkey1);
        return 1;
    }
    char fp1_hex[65];
    if (bytes_to_hex_lower(fp1, sizeof(fp1), fp1_hex, sizeof(fp1_hex)) != 0) {
        fprintf(stderr, "Error: failed to hex-encode fingerprint\n");
        X509_free(cert1);
        EVP_PKEY_free(pkey1);
        return 1;
    }

    /* Write cert1 to a temp PEM file */
    char pem_path1[128];
    if (write_cert_pem_to_temp(cert1, pem_path1, sizeof(pem_path1)) != 0) {
        fprintf(stderr, "Error: failed to write cert1 PEM\n");
        X509_free(cert1);
        EVP_PKEY_free(pkey1);
        return 1;
    }

    /* Test case 1: Matching hash (PEM) */
    int res1 = check_cert_matches_known_hash_file(pem_path1, fp1_hex);
    printf("Test1 (PEM match): %s\n", res1 == 1 ? "PASS" : "FAIL");

    /* Test case 2: Mismatched hash (modify one nibble) */
    char bad_hex[65];
    memcpy(bad_hex, fp1_hex, sizeof(bad_hex));
    bad_hex[0] = (bad_hex[0] == 'a') ? 'b' : 'a';
    int res2 = check_cert_matches_known_hash_file(pem_path1, bad_hex);
    printf("Test2 (PEM mismatch): %s\n", res2 == 0 ? "PASS" : "FAIL");

    /* Test case 3: Different certificate should not match hash of cert1 */
    X509 *cert2 = NULL;
    EVP_PKEY *pkey2 = NULL;
    if (generate_self_signed_cert("test2.local", &cert2, &pkey2) != 0) {
        fprintf(stderr, "Error: failed to generate test certificate 2\n");
        unlink(pem_path1);
        X509_free(cert1); EVP_PKEY_free(pkey1);
        return 1;
    }
    char pem_path2[128];
    if (write_cert_pem_to_temp(cert2, pem_path2, sizeof(pem_path2)) != 0) {
        fprintf(stderr, "Error: failed to write cert2 PEM\n");
        unlink(pem_path1);
        X509_free(cert1); EVP_PKEY_free(pkey1);
        X509_free(cert2); EVP_PKEY_free(pkey2);
        return 1;
    }
    int res3 = check_cert_matches_known_hash_file(pem_path2, fp1_hex);
    printf("Test3 (different cert mismatch): %s\n", res3 == 0 ? "PASS" : "FAIL");

    /* Test case 4: Invalid hash string should error */
    int res4 = check_cert_matches_known_hash_file(pem_path1, "ZZZ-not-hex");
    printf("Test4 (invalid hex error): %s\n", res4 == -1 ? "PASS" : "FAIL");

    /* Test case 5: DER file match */
    char der_path1[128];
    if (write_cert_der_to_temp(cert1, der_path1, sizeof(der_path1)) != 0) {
        fprintf(stderr, "Error: failed to write cert1 DER\n");
        unlink(pem_path1);
        unlink(pem_path2);
        X509_free(cert1); EVP_PKEY_free(pkey1);
        X509_free(cert2); EVP_PKEY_free(pkey2);
        return 1;
    }
    int res5 = check_cert_matches_known_hash_file(der_path1, fp1_hex);
    printf("Test5 (DER match): %s\n", res5 == 1 ? "PASS" : "FAIL");

    /* Cleanup */
    secure_zero(fp1, sizeof(fp1));
    secure_zero(fp1_hex, sizeof(fp1_hex));
    secure_zero(bad_hex, sizeof(bad_hex));

    unlink(pem_path1);
    unlink(pem_path2);
    unlink(der_path1);

    X509_free(cert1);
    EVP_PKEY_free(pkey1);
    X509_free(cert2);
    EVP_PKEY_free(pkey2);

    /* Exit code indicates if all tests passed */
    rc = (res1 == 1 && res2 == 0 && res3 == 0 && res4 == -1 && res5 == 1) ? 0 : 2;
    return rc;
}