#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/err.h>

// Encodes data to a URL-safe Base64 string.
// The caller is responsible for freeing the returned string.
// Returns NULL on failure.
char* base64_url_encode(const unsigned char* input, size_t length) {
    BIO *bio = NULL, *b64 = NULL;
    BUF_MEM *bufferPtr;
    char *encoded_data = NULL;
    char *result = NULL;
    long encoded_len;

    b64 = BIO_new(BIO_f_base64());
    if (b64 == NULL) {
        fprintf(stderr, "Error: BIO_new(BIO_f_base64()) failed.\n");
        return NULL;
    }
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

    bio = BIO_new(BIO_s_mem());
    if (bio == NULL) {
        fprintf(stderr, "Error: BIO_new(BIO_s_mem()) failed.\n");
        BIO_free_all(b64);
        return NULL;
    }
    
    bio = BIO_push(b64, bio);

    if (BIO_write(bio, input, length) <= 0) {
        fprintf(stderr, "Error: BIO_write failed.\n");
        BIO_free_all(bio);
        return NULL;
    }
    BIO_flush(bio);
    
    encoded_len = BIO_get_mem_data(bio, &encoded_data);
    if (encoded_len <= 0) {
        fprintf(stderr, "Error: BIO_get_mem_data failed.\n");
        BIO_free_all(bio);
        return NULL;
    }
    
    result = (char *)malloc(encoded_len + 1);
    if (result == NULL) {
        fprintf(stderr, "Error: malloc failed for encoded data.\n");
        BIO_free_all(bio);
        return NULL;
    }
    memcpy(result, encoded_data, encoded_len);
    result[encoded_len] = '\0';
    
    BIO_free_all(bio);

    char *p = result;
    char *writer = result;
    while (*p) {
        if (*p == '+') *writer = '-';
        else if (*p == '/') *writer = '_';
        else if (*p != '=') *writer = *p;
        else break; // Stop at the first padding character
        p++;
        writer++;
    }
    *writer = '\0';

    return result;
}

// Generates an HMAC-SHA256 signature.
// The caller is responsible for freeing the *signature.
// Returns 0 on success, -1 on failure.
int generate_hmac_sha256(const char* data, const char* key, unsigned char** signature, unsigned int* sig_len) {
    if (!data || !key || !signature || !sig_len) return -1;
    
    *signature = (unsigned char*)malloc(EVP_MAX_MD_SIZE);
    if (*signature == NULL) {
        fprintf(stderr, "Error: malloc failed for signature.\n");
        return -1;
    }

    if (HMAC(EVP_sha256(), key, strlen(key), (const unsigned char*)data, strlen(data), *signature, sig_len) == NULL) {
        fprintf(stderr, "Error: HMAC failed.\n");
        free(*signature);
        *signature = NULL;
        return -1;
    }
    
    return 0;
}

/**
 * @brief Generates a secure authentication token.
 *
 * The token format is: base64url(payload).base64url(signature)
 * Payload is: "<user_id>:<expiration_timestamp>"
 * Signature is: HMAC-SHA256(payload, secret_key)
 *
 * The caller is responsible for freeing the returned string.
 *
 * @param user_id The user identifier to include in the token.
 * @param secret_key The secret key for signing. MUST be kept confidential.
 * @param validity_seconds The duration for which the token is valid.
 * @return A secure authentication token as a string, or NULL on failure.
 */
char* generate_token(const char* user_id, const char* secret_key, int validity_seconds) {
    char *payload = NULL;
    unsigned char *signature = NULL;
    char *b64_payload = NULL;
    char *b64_signature = NULL;
    char *token = NULL;
    int success = 0;

    if (user_id == NULL || secret_key == NULL || validity_seconds <= 0) {
        fprintf(stderr, "Error: Invalid arguments for token generation.\n");
        return NULL;
    }

    // 1. Create payload
    long long expiration_timestamp = (long long)time(NULL) + validity_seconds;
    int payload_len = snprintf(NULL, 0, "%s:%lld", user_id, expiration_timestamp);
    if (payload_len < 0) goto cleanup;

    payload = (char*)malloc(payload_len + 1);
    if (payload == NULL) goto cleanup;
    snprintf(payload, payload_len + 1, "%s:%lld", user_id, expiration_timestamp);
    
    // 2. Generate HMAC-SHA256 signature
    unsigned int sig_len = 0;
    if (generate_hmac_sha256(payload, secret_key, &signature, &sig_len) != 0) goto cleanup;
    
    // 3. Base64-URL-encode payload
    b64_payload = base64_url_encode((const unsigned char*)payload, strlen(payload));
    if (b64_payload == NULL) goto cleanup;

    // 4. Base64-URL-encode signature
    b64_signature = base64_url_encode(signature, sig_len);
    if (b64_signature == NULL) goto cleanup;

    // 5. Combine into final token
    int token_len = snprintf(NULL, 0, "%s.%s", b64_payload, b64_signature);
    if (token_len < 0) goto cleanup;
    
    token = (char*)malloc(token_len + 1);
    if (token == NULL) goto cleanup;
    snprintf(token, token_len + 1, "%s.%s", b64_payload, b64_signature);

    success = 1;

cleanup:
    free(payload);
    free(signature);
    free(b64_payload);
    free(b64_signature);

    if (!success) {
        free(token);
        token = NULL;
    }
    
    return token;
}

int main() {
    // WARNING: Do not hard-code secret keys in production code.
    // Load them from a secure source like environment variables or a secret management system.
    const char* secret_key = "a_very_long_and_secure_secret_key_that_is_at_least_32_bytes";

    printf("Generating Tokens with Secret Key: [REDACTED]\n");

    const char* test_users[] = {"user123", "admin", "guest-user", "another_user", "test@example.com"};
    const int test_validities[] = {3600, 86400, 60, 300, 1800};

    for (int i = 0; i < 5; ++i) {
        const char* user_id = test_users[i];
        int validity = test_validities[i];
        char* token = generate_token(user_id, secret_key, validity);

        printf("Test Case %d:\n", i + 1);
        printf("  User ID: %s\n", user_id);
        printf("  Validity: %ds\n", validity);

        if (token != NULL) {
            printf("  Token: %s\n\n", token);
            free(token); // Free the token after use
        } else {
            printf("  Token: <GENERATION FAILED>\n\n");
        }
    }

    return 0;
}