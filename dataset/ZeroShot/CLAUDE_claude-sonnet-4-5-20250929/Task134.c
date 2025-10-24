
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

#define KEY_SIZE 32
#define IV_SIZE 12
#define TAG_SIZE 16
#define MAX_KEYS 100
#define MAX_KEY_ID_LEN 64

typedef struct {
    char keyId[MAX_KEY_ID_LEN];
    unsigned char key[KEY_SIZE];
    int active;
} KeyEntry;

typedef struct {
    KeyEntry keys[MAX_KEYS];
    int count;
} KeyStore;

KeyStore keyStore = {0};

static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char* base64_encode(const unsigned char* data, int input_length, int* output_length) {
    *output_length = 4 * ((input_length + 2) / 3);
    char* encoded = malloc(*output_length + 1);
    if (!encoded) return NULL;
    
    int i, j;
    for (i = 0, j = 0; i < input_length;) {
        uint32_t octet_a = i < input_length ? data[i++] : 0;
        uint32_t octet_b = i < input_length ? data[i++] : 0;
        uint32_t octet_c = i < input_length ? data[i++] : 0;
        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;
        
        encoded[j++] = base64_chars[(triple >> 3 * 6) & 0x3F];
        encoded[j++] = base64_chars[(triple >> 2 * 6) & 0x3F];
        encoded[j++] = base64_chars[(triple >> 1 * 6) & 0x3F];
        encoded[j++] = base64_chars[(triple >> 0 * 6) & 0x3F];
    }
    
    for (i = 0; i < (3 - input_length % 3) % 3; i++)
        encoded[*output_length - 1 - i] = '=';
    
    encoded[*output_length] = '\\0';
    return encoded;
}

unsigned char* base64_decode(const char* data, int input_length, int* output_length) {
    if (input_length % 4 != 0) return NULL;
    
    *output_length = input_length / 4 * 3;
    if (data[input_length - 1] == '=') (*output_length)--;
    if (data[input_length - 2] == '=') (*output_length)--;
    
    unsigned char* decoded = malloc(*output_length);
    if (!decoded) return NULL;
    
    int i, j;
    for (i = 0, j = 0; i < input_length;) {
        uint32_t sextet_a = data[i] == '=' ? 0 & i++ : strchr(base64_chars, data[i++]) - base64_chars;
        uint32_t sextet_b = data[i] == '=' ? 0 & i++ : strchr(base64_chars, data[i++]) - base64_chars;
        uint32_t sextet_c = data[i] == '=' ? 0 & i++ : strchr(base64_chars, data[i++]) - base64_chars;
        uint32_t sextet_d = data[i] == '=' ? 0 & i++ : strchr(base64_chars, data[i++]) - base64_chars;
        uint32_t triple = (sextet_a << 3 * 6) + (sextet_b << 2 * 6) + (sextet_c << 1 * 6) + (sextet_d << 0 * 6);
        
        if (j < *output_length) decoded[j++] = (triple >> 2 * 8) & 0xFF;
        if (j < *output_length) decoded[j++] = (triple >> 1 * 8) & 0xFF;
        if (j < *output_length) decoded[j++] = (triple >> 0 * 8) & 0xFF;
    }
    
    return decoded;
}

char* generateKey(const char* keyId) {
    if (keyStore.count >= MAX_KEYS) {
        return "Key store is full";
    }
    
    for (int i = 0; i < keyStore.count; i++) {
        if (keyStore.keys[i].active && strcmp(keyStore.keys[i].keyId, keyId) == 0) {
            return "Key already exists";
        }
    }
    
    KeyEntry* entry = &keyStore.keys[keyStore.count];
    strncpy(entry->keyId, keyId, MAX_KEY_ID_LEN - 1);
    entry->keyId[MAX_KEY_ID_LEN - 1] = '\\0';
    
    if (RAND_bytes(entry->key, KEY_SIZE) != 1) {
        return "Error generating key";
    }
    
    entry->active = 1;
    keyStore.count++;
    
    static char result[128];
    snprintf(result, sizeof(result), "Key generated successfully: %s", keyId);
    return result;
}

char* encryptData(const char* keyId, const char* plaintext) {
    KeyEntry* entry = NULL;
    for (int i = 0; i < keyStore.count; i++) {
        if (keyStore.keys[i].active && strcmp(keyStore.keys[i].keyId, keyId) == 0) {
            entry = &keyStore.keys[i];
            break;
        }
    }
    
    if (!entry) return "Key not found";
    
    unsigned char iv[IV_SIZE];
    if (RAND_bytes(iv, IV_SIZE) != 1) {
        return "Error generating IV";
    }
    
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return "Error creating context";
    
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, entry->key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return "Error initializing encryption";
    }
    
    int plaintext_len = strlen(plaintext);
    unsigned char* ciphertext = malloc(plaintext_len + EVP_CIPHER_block_size(EVP_aes_256_gcm()));
    int len, ciphertext_len;
    
    if (EVP_EncryptUpdate(ctx, ciphertext, &len, (unsigned char*)plaintext, plaintext_len) != 1) {
        free(ciphertext);
        EVP_CIPHER_CTX_free(ctx);
        return "Error encrypting";
    }
    ciphertext_len = len;
    
    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) {
        free(ciphertext);
        EVP_CIPHER_CTX_free(ctx);
        return "Error finalizing encryption";
    }
    ciphertext_len += len;
    
    unsigned char tag[TAG_SIZE];
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_SIZE, tag) != 1) {
        free(ciphertext);
        EVP_CIPHER_CTX_free(ctx);
        return "Error getting tag";
    }
    
    EVP_CIPHER_CTX_free(ctx);
    
    int combined_len = IV_SIZE + ciphertext_len + TAG_SIZE;
    unsigned char* combined = malloc(combined_len);
    memcpy(combined, iv, IV_SIZE);
    memcpy(combined + IV_SIZE, ciphertext, ciphertext_len);
    memcpy(combined + IV_SIZE + ciphertext_len, tag, TAG_SIZE);
    
    int encoded_len;
    char* encoded = base64_encode(combined, combined_len, &encoded_len);
    
    free(ciphertext);
    free(combined);
    
    return encoded;
}

char* decryptData(const char* keyId, const char* encryptedData) {
    KeyEntry* entry = NULL;
    for (int i = 0; i < keyStore.count; i++) {
        if (keyStore.keys[i].active && strcmp(keyStore.keys[i].keyId, keyId) == 0) {
            entry = &keyStore.keys[i];
            break;
        }
    }
    
    if (!entry) return "Key not found";
    
    int combined_len;
    unsigned char* combined = base64_decode(encryptedData, strlen(encryptedData), &combined_len);
    if (!combined || combined_len < IV_SIZE + TAG_SIZE) {
        free(combined);
        return "Invalid encrypted data";
    }
    
    unsigned char* iv = combined;
    unsigned char* ciphertext = combined + IV_SIZE;
    int ciphertext_len = combined_len - IV_SIZE - TAG_SIZE;
    unsigned char* tag = combined + IV_SIZE + ciphertext_len;
    
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        free(combined);
        return "Error creating context";
    }
    
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, entry->key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(combined);
        return "Error initializing decryption";
    }
    
    unsigned char* plaintext = malloc(ciphertext_len + EVP_CIPHER_block_size(EVP_aes_256_gcm()));
    int len, plaintext_len;
    
    if (EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len) != 1) {
        free(plaintext);
        EVP_CIPHER_CTX_free(ctx);
        free(combined);
        return "Error decrypting";
    }
    plaintext_len = len;
    
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_SIZE, tag) != 1) {
        free(plaintext);
        EVP_CIPHER_CTX_free(ctx);
        free(combined);
        return "Error setting tag";
    }
    
    if (EVP_DecryptFinal_ex(ctx, plaintext + len, &len) != 1) {
        free(plaintext);
        EVP_CIPHER_CTX_free(ctx);
        free(combined);
        return "Error finalizing decryption";
    }
    plaintext_len += len;
    plaintext[plaintext_len] = '\\0';
    
    EVP_CIPHER_CTX_free(ctx);
    free(combined);
    
    return (char*)plaintext;
}

char* deleteKey(const char* keyId) {
    for (int i = 0; i < keyStore.count; i++) {
        if (keyStore.keys[i].active && strcmp(keyStore.keys[i].keyId, keyId) == 0) {
            keyStore.keys[i].active = 0;
            static char result[128];
            snprintf(result, sizeof(result), "Key deleted: %s", keyId);
            return result;
        }
    }
    return "Key not found";
}

void listKeys() {
    printf("Keys: ");
    for (int i = 0; i < keyStore.count; i++) {
        if (keyStore.keys[i].active) {
            printf("%s ", keyStore.keys[i].keyId);
        }
    }
    printf("\\n");
}

int main() {
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();
    
    // Test Case 1: Generate a key
    printf("Test 1 - Generate Key:\\n");
    printf("%s\\n\\n", generateKey("key1"));
    
    // Test Case 2: Encrypt data
    printf("Test 2 - Encrypt Data:\\n");
    char* encrypted = encryptData("key1", "Hello, Secure World!");
    printf("Encrypted: %s\\n\\n", encrypted);
    
    // Test Case 3: Decrypt data
    printf("Test 3 - Decrypt Data:\\n");
    char* decrypted = decryptData("key1", encrypted);
    printf("Decrypted: %s\\n\\n", decrypted);
    free(decrypted);
    
    // Test Case 4: List all keys
    printf("Test 4 - List Keys:\\n");
    generateKey("key2");
    generateKey("key3");
    listKeys();
    printf("\\n");
    
    // Test Case 5: Delete a key
    printf("Test 5 - Delete Key:\\n");
    printf("%s\\n", deleteKey("key2"));
    printf("Remaining ");
    listKeys();
    
    free(encrypted);
    return 0;
}
