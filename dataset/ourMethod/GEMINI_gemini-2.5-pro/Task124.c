#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/kdf.h>
#include <openssl/err.h>

#define SALT_LENGTH_BYTES 16
#define IV_LENGTH_BYTES 12
#define TAG_LENGTH_BYTES 16
#define KEY_LENGTH_BYTES 32 // 256 bits
#define PBKDF2_ITERATIONS 600000

// On success, returns a newly allocated buffer with the ciphertext blob.
// The caller is responsible for freeing this buffer.
// On failure, returns NULL. The size of the buffer is written to out_len.
unsigned char* encrypt_data(const char* plaintext, const char* passphrase, int* out_len) {
    if (!plaintext || !passphrase || !out_len) return NULL;
    int plaintext_len = strlen(plaintext);
    if (plaintext_len == 0 || strlen(passphrase) == 0) return NULL;

    unsigned char salt[SALT_LENGTH_BYTES];
    if (RAND_bytes(salt, sizeof(salt)) != 1) return NULL;

    unsigned char iv[IV_LENGTH_BYTES];
    if (RAND_bytes(iv, sizeof(iv)) != 1) return NULL;

    unsigned char key[KEY_LENGTH_BYTES];
    if (PKCS5_PBKDF2_HMAC(passphrase, strlen(passphrase), salt, sizeof(salt), PBKDF2_ITERATIONS,
                          EVP_sha256(), sizeof(key), key) != 1) {
        return NULL;
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return NULL;

    unsigned char* ciphertext = malloc(plaintext_len);
    if (!ciphertext) { EVP_CIPHER_CTX_free(ctx); return NULL; }

    unsigned char tag[TAG_LENGTH_BYTES];
    int len = 0;
    int ciphertext_len = 0;

    int ret = 1;
    if (ret == 1) ret = EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);
    if (ret == 1) ret = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, sizeof(iv), NULL);
    if (ret == 1) ret = EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv);
    if (ret == 1) ret = EVP_EncryptUpdate(ctx, ciphertext, &len, (const unsigned char*)plaintext, plaintext_len);
    if (ret == 1) {
        ciphertext_len = len;
        ret = EVP_EncryptFinal_ex(ctx, ciphertext + len, &len);
    }
    if (ret == 1) {
        ciphertext_len += len;
        ret = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, sizeof(tag), tag);
    }
    
    if (ret != 1) {
        free(ciphertext);
        EVP_CIPHER_CTX_free(ctx);
        OPENSSL_cleanse(key, sizeof(key));
        return NULL;
    }

    *out_len = sizeof(salt) + sizeof(iv) + ciphertext_len + sizeof(tag);
    unsigned char* result = malloc(*out_len);
    if (!result) {
        free(ciphertext);
        EVP_CIPHER_CTX_free(ctx);
        OPENSSL_cleanse(key, sizeof(key));
        return NULL;
    }

    memcpy(result, salt, sizeof(salt));
    memcpy(result + sizeof(salt), iv, sizeof(iv));
    memcpy(result + sizeof(salt) + sizeof(iv), ciphertext, ciphertext_len);
    memcpy(result + sizeof(salt) + sizeof(iv) + ciphertext_len, tag, sizeof(tag));

    free(ciphertext);
    EVP_CIPHER_CTX_free(ctx);
    OPENSSL_cleanse(key, sizeof(key));
    return result;
}

// On success, returns a newly allocated null-terminated string with the plaintext.
// The caller is responsible for freeing this buffer.
// On failure (e.g., bad password, corrupt data), returns NULL.
char* decrypt_data(const unsigned char* ciphertext_blob, int blob_len, const char* passphrase) {
    if (!ciphertext_blob || !passphrase || blob_len < SALT_LENGTH_BYTES + IV_LENGTH_BYTES + TAG_LENGTH_BYTES) {
        return NULL;
    }

    const unsigned char* p = ciphertext_blob;
    unsigned char salt[SALT_LENGTH_BYTES];
    memcpy(salt, p, sizeof(salt));
    p += sizeof(salt);
    
    unsigned char iv[IV_LENGTH_BYTES];
    memcpy(iv, p, sizeof(iv));
    p += sizeof(iv);

    int ciphertext_len = blob_len - sizeof(salt) - sizeof(iv) - TAG_LENGTH_BYTES;
    const unsigned char* ciphertext = p;
    p += ciphertext_len;
    
    unsigned char tag[TAG_LENGTH_BYTES];
    memcpy(tag, p, sizeof(tag));

    unsigned char key[KEY_LENGTH_BYTES];
    if (PKCS5_PBKDF2_HMAC(passphrase, strlen(passphrase), salt, sizeof(salt), PBKDF2_ITERATIONS,
                          EVP_sha256(), sizeof(key), key) != 1) {
        return NULL;
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) { OPENSSL_cleanse(key, sizeof(key)); return NULL; }
    
    char* plaintext = malloc(ciphertext_len + 1); // +1 for null terminator
    if (!plaintext) {
        EVP_CIPHER_CTX_free(ctx);
        OPENSSL_cleanse(key, sizeof(key));
        return NULL;
    }

    int len = 0;
    int plaintext_len = 0;

    int ret = 1;
    if (ret == 1) ret = EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);
    if (ret == 1) ret = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, sizeof(iv), NULL);
    if (ret == 1) ret = EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv);
    if (ret == 1) ret = EVP_DecryptUpdate(ctx, (unsigned char*)plaintext, &len, ciphertext, ciphertext_len);
    if (ret == 1) {
        plaintext_len = len;
        ret = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, sizeof(tag), tag);
    }
    if (ret == 1) {
        // Final check happens here
        ret = EVP_DecryptFinal_ex(ctx, (unsigned char*)plaintext + len, &len);
    }
    
    EVP_CIPHER_CTX_free(ctx);
    OPENSSL_cleanse(key, sizeof(key));

    if (ret > 0) {
        plaintext_len += len;
        plaintext[plaintext_len] = '\0';
        return plaintext;
    } else {
        // Authentication failed
        free(plaintext);
        return NULL;
    }
}

void print_hex(const char* label, const unsigned char* data, int len) {
    printf("%s", label);
    for (int i = 0; i < len; ++i) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

void run_test_case(int i, const char* original_data, const char* passphrase) {
    printf("--- Test Case %d ---\n", i);
    printf("Original: %s\n", original_data);
    printf("Passphrase: %s\n", passphrase);

    int encrypted_len = 0;
    unsigned char* encrypted_data = encrypt_data(original_data, passphrase, &encrypted_len);
    if (encrypted_data) {
        print_hex("Encrypted (Hex): ", encrypted_data, encrypted_len);

        char* decrypted_data = decrypt_data(encrypted_data, encrypted_len, passphrase);
        if (decrypted_data) {
            printf("Decrypted (Success): %s\n", decrypted_data);
            if (strcmp(decrypted_data, original_data) == 0) {
                printf("SUCCESS: Decrypted data matches original.\n");
            } else {
                printf("FAILURE: Decrypted data does NOT match original.\n");
            }
            free(decrypted_data);
        } else {
            printf("FAILURE: Decryption failed with correct passphrase.\n");
        }
        
        char* failed_decryption = decrypt_data(encrypted_data, encrypted_len, "wrong-password");
        if (!failed_decryption) {
            printf("SUCCESS: Decryption failed as expected with wrong passphrase.\n");
        } else {
            printf("FAILURE: Decryption succeeded with wrong passphrase.\n");
            free(failed_decryption);
        }

        free(encrypted_data);
    } else {
        fprintf(stderr, "ERROR: Encryption failed.\n");
    }
    printf("\n");
}

int main() {
    const char* test_passphrases[] = {
        "correct-horse-battery-staple", "Tr0ub4dor&3", "!@#$%^&*()_+",
        "a long passphrase with spaces and punctuation", "short"
    };
    const char* test_data[] = {
        "Credit Card: 1234-5678-9012-3456", "SSN: 000-00-0000",
        "{\"user\":\"admin\",\"password\":\"password123\"}", "This is a secret message.",
        "Another piece of sensitive info."
    };

    for (int i = 0; i < 5; ++i) {
        run_test_case(i + 1, test_data[i], test_passphrases[i]);
    }

    return 0;
}