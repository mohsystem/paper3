
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

#define KEY_SIZE 32
#define IV_LENGTH 12
#define TAG_LENGTH 16

typedef struct {
    unsigned char* data;
    size_t size;
} ByteArray;

ByteArray* createByteArray(size_t size) {
    ByteArray* arr = (ByteArray*)malloc(sizeof(ByteArray));
    if (!arr) return NULL;
    arr->data = (unsigned char*)malloc(size);
    if (!arr->data) {
        free(arr);
        return NULL;
    }
    arr->size = size;
    return arr;
}

void freeByteArray(ByteArray* arr) {
    if (arr) {
        if (arr->data) free(arr->data);
        free(arr);
    }
}

ByteArray* generateKey() {
    ByteArray* key = createByteArray(KEY_SIZE);
    if (!key) return NULL;
    
    if (RAND_bytes(key->data, KEY_SIZE) != 1) {
        freeByteArray(key);
        return NULL;
    }
    return key;
}

ByteArray* encryptFile(const unsigned char* fileContent, size_t contentSize, const unsigned char* key) {
    if (!fileContent || !key || contentSize == 0) {
        return NULL;
    }

    unsigned char iv[IV_LENGTH];
    if (RAND_bytes(iv, IV_LENGTH) != 1) {
        return NULL;
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return NULL;

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }

    unsigned char* ciphertext = (unsigned char*)malloc(contentSize + EVP_CIPHER_block_size(EVP_aes_256_gcm()));
    if (!ciphertext) {
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }

    int len = 0;
    int ciphertext_len = 0;

    if (EVP_EncryptUpdate(ctx, ciphertext, &len, fileContent, contentSize) != 1) {
        free(ciphertext);
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }
    ciphertext_len = len;

    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) {
        free(ciphertext);
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }
    ciphertext_len += len;

    unsigned char tag[TAG_LENGTH];
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_LENGTH, tag) != 1) {
        free(ciphertext);
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }

    EVP_CIPHER_CTX_free(ctx);

    ByteArray* result = createByteArray(IV_LENGTH + ciphertext_len + TAG_LENGTH);
    if (!result) {
        free(ciphertext);
        return NULL;
    }

    memcpy(result->data, iv, IV_LENGTH);
    memcpy(result->data + IV_LENGTH, ciphertext, ciphertext_len);
    memcpy(result->data + IV_LENGTH + ciphertext_len, tag, TAG_LENGTH);

    free(ciphertext);
    return result;
}

ByteArray* decryptFile(const unsigned char* encryptedContent, size_t contentSize, const unsigned char* key) {
    if (!encryptedContent || !key || contentSize < IV_LENGTH + TAG_LENGTH) {
        return NULL;
    }

    unsigned char iv[IV_LENGTH];
    unsigned char tag[TAG_LENGTH];
    size_t ciphertext_len = contentSize - IV_LENGTH - TAG_LENGTH;

    memcpy(iv, encryptedContent, IV_LENGTH);
    memcpy(tag, encryptedContent + contentSize - TAG_LENGTH, TAG_LENGTH);

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return NULL;

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }

    unsigned char* plaintext = (unsigned char*)malloc(ciphertext_len + EVP_CIPHER_block_size(EVP_aes_256_gcm()));
    if (!plaintext) {
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }

    int len = 0;
    int plaintext_len = 0;

    if (EVP_DecryptUpdate(ctx, plaintext, &len, encryptedContent + IV_LENGTH, ciphertext_len) != 1) {
        free(plaintext);
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }
    plaintext_len = len;

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_LENGTH, tag) != 1) {
        free(plaintext);
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }

    if (EVP_DecryptFinal_ex(ctx, plaintext + len, &len) != 1) {
        free(plaintext);
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }
    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    ByteArray* result = createByteArray(plaintext_len);
    if (!result) {
        free(plaintext);
        return NULL;
    }

    memcpy(result->data, plaintext, plaintext_len);
    free(plaintext);

    return result;
}

int main() {
    printf("=== Secure File Encryption/Decryption Test Cases ===\\n\\n");

    // Test Case 1: Simple text encryption/decryption
    printf("Test Case 1: Simple text encryption\\n");
    ByteArray* key1 = generateKey();
    if (key1) {
        const char* text1 = "Hello, World!";
        ByteArray* encrypted1 = encryptFile((unsigned char*)text1, strlen(text1), key1->data);
        if (encrypted1) {
            ByteArray* decrypted1 = decryptFile(encrypted1->data, encrypted1->size, key1->data);
            if (decrypted1) {
                printf("Original: %s\\n", text1);
                printf("Decrypted: %.*s\\n", (int)decrypted1->size, decrypted1->data);
                printf("Match: %s\\n\\n", (memcmp(text1, decrypted1->data, strlen(text1)) == 0) ? "true" : "false");
                freeByteArray(decrypted1);
            }
            freeByteArray(encrypted1);
        }
        freeByteArray(key1);
    }

    // Test Case 2: Empty content
    printf("Test Case 2: Empty content\\n");
    ByteArray* key2 = generateKey();
    if (key2) {
        const char* text2 = "";
        ByteArray* encrypted2 = encryptFile((unsigned char*)text2, 0, key2->data);
        if (encrypted2) {
            ByteArray* decrypted2 = decryptFile(encrypted2->data, encrypted2->size, key2->data);
            if (decrypted2) {
                printf("Original length: 0\\n");
                printf("Decrypted length: %zu\\n", decrypted2->size);
                printf("Match: %s\\n\\n", (decrypted2->size == 0) ? "true" : "false");
                freeByteArray(decrypted2);
            }
            freeByteArray(encrypted2);
        }
        freeByteArray(key2);
    }

    // Test Case 3: Large text
    printf("Test Case 3: Large text encryption\\n");
    ByteArray* key3 = generateKey();
    if (key3) {
        size_t bufferSize = 50000;
        char* text3 = (char*)malloc(bufferSize);
        if (text3) {
            int offset = 0;
            for (int i = 0; i < 1000 && offset < bufferSize - 50; i++) {
                offset += snprintf(text3 + offset, bufferSize - offset, "Secure encryption test %d ", i);
            }
            ByteArray* encrypted3 = encryptFile((unsigned char*)text3, offset, key3->data);
            if (encrypted3) {
                ByteArray* decrypted3 = decryptFile(encrypted3->data, encrypted3->size, key3->data);
                if (decrypted3) {
                    printf("Original length: %d\\n", offset);
                    printf("Decrypted length: %zu\\n", decrypted3->size);
                    printf("Match: %s\\n\\n", (memcmp(text3, decrypted3->data, offset) == 0) ? "true" : "false");
                    freeByteArray(decrypted3);
                }
                freeByteArray(encrypted3);
            }
            free(text3);
        }
        freeByteArray(key3);
    }

    // Test Case 4: Special characters
    printf("Test Case 4: Special characters\\n");
    ByteArray* key4 = generateKey();
    if (key4) {
        const char* text4 = "Special chars: !@#$%^&*()_+-=[]{}|;':\\",./<>?";
        ByteArray* encrypted4 = encryptFile((unsigned char*)text4, strlen(text4), key4->data);
        if (encrypted4) {
            ByteArray* decrypted4 = decryptFile(encrypted4->data, encrypted4->size, key4->data);
            if (decrypted4) {
                printf("Original: %s\\n", text4);
                printf("Decrypted: %.*s\\n", (int)decrypted4->size, decrypted4->data);
                printf("Match: %s\\n\\n", (memcmp(text4, decrypted4->data, strlen(text4)) == 0) ? "true" : "false");
                freeByteArray(decrypted4);
            }
            freeByteArray(encrypted4);
        }
        freeByteArray(key4);
    }

    // Test Case 5: Binary data
    printf("Test Case 5: Binary data\\n");
    ByteArray* key5 = generateKey();
    if (key5) {
        unsigned char binaryData[256];
        for (int i = 0; i < 256; i++) {
            binaryData[i] = (unsigned char)i;
        }
        ByteArray* encrypted5 = encryptFile(binaryData, 256, key5->data);
        if (encrypted5) {
            ByteArray* decrypted5 = decryptFile(encrypted5->data, encrypted5->size, key5->data);
            if (decrypted5) {
                printf("Original length: 256\\n");
                printf("Decrypted length: %zu\\n", decrypted5->size);
                printf("Match: %s\\n\\n", (memcmp(binaryData, decrypted5->data, 256) == 0) ? "true" : "false");
                freeByteArray(decrypted5);
            }
            freeByteArray(encrypted5);
        }
        freeByteArray(key5);
    }

    return 0;
}
