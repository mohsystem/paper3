
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#define AES_KEY_SIZE 32
#define RSA_KEY_SIZE 2048
#define GCM_IV_LENGTH 12
#define GCM_TAG_LENGTH 16
#define PBKDF2_ITERATIONS 100000
#define MAX_KEYS 100
#define MAX_KEY_ID_LENGTH 256

typedef struct {
    char keyId[MAX_KEY_ID_LENGTH];
    unsigned char* encryptedKey;
    size_t keyLength;
} KeyEntry;

typedef struct {
    KeyEntry keys[MAX_KEYS];
    int keyCount;
    unsigned char masterKey[AES_KEY_SIZE];
} Task134;

void deriveMasterKey(const char* password, unsigned char* masterKey) {
    unsigned char salt[32];
    RAND_bytes(salt, 32);
    
    PKCS5_PBKDF2_HMAC(password, strlen(password),
                      salt, 32,
                      PBKDF2_ITERATIONS,
                      EVP_sha256(),
                      AES_KEY_SIZE, masterKey);
}

Task134* Task134_create(const char* masterPassword) {
    Task134* manager = (Task134*)malloc(sizeof(Task134));
    if (!manager) return NULL;
    
    manager->keyCount = 0;
    deriveMasterKey(masterPassword, manager->masterKey);
    
    return manager;
}

unsigned char* encryptKey(Task134* manager, const unsigned char* keyBytes, size_t keyLen, size_t* outLen) {
    unsigned char iv[GCM_IV_LENGTH];
    RAND_bytes(iv, GCM_IV_LENGTH);
    
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return NULL;
    
    unsigned char* ciphertext = (unsigned char*)malloc(keyLen + EVP_CIPHER_block_size(EVP_aes_256_gcm()));
    unsigned char tag[GCM_TAG_LENGTH];
    
    int len;
    int ciphertext_len;
    
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, manager->masterKey, iv)) {
        EVP_CIPHER_CTX_free(ctx);
        free(ciphertext);
        return NULL;
    }
    
    if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, keyBytes, keyLen)) {
        EVP_CIPHER_CTX_free(ctx);
        free(ciphertext);
        return NULL;
    }
    ciphertext_len = len;
    
    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        free(ciphertext);
        return NULL;
    }
    ciphertext_len += len;
    
    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, GCM_TAG_LENGTH, tag)) {
        EVP_CIPHER_CTX_free(ctx);
        free(ciphertext);
        return NULL;
    }
    
    EVP_CIPHER_CTX_free(ctx);
    
    size_t totalLen = GCM_IV_LENGTH + ciphertext_len + GCM_TAG_LENGTH;
    unsigned char* result = (unsigned char*)malloc(totalLen);
    
    memcpy(result, iv, GCM_IV_LENGTH);
    memcpy(result + GCM_IV_LENGTH, ciphertext, ciphertext_len);
    memcpy(result + GCM_IV_LENGTH + ciphertext_len, tag, GCM_TAG_LENGTH);
    
    free(ciphertext);
    *outLen = totalLen;
    
    return result;
}

int generateAESKey(Task134* manager, const char* keyId) {
    if (!keyId || strlen(keyId) == 0 || manager->keyCount >= MAX_KEYS) {
        return 0;
    }
    
    unsigned char key[AES_KEY_SIZE];
    RAND_bytes(key, AES_KEY_SIZE);
    
    size_t encryptedLen;
    unsigned char* encryptedKey = encryptKey(manager, key, AES_KEY_SIZE, &encryptedLen);
    
    if (!encryptedKey) return 0;
    
    strncpy(manager->keys[manager->keyCount].keyId, keyId, MAX_KEY_ID_LENGTH - 1);
    manager->keys[manager->keyCount].keyId[MAX_KEY_ID_LENGTH - 1] = '\\0';
    manager->keys[manager->keyCount].encryptedKey = encryptedKey;
    manager->keys[manager->keyCount].keyLength = encryptedLen;
    manager->keyCount++;
    
    return 1;
}

int generateRSAKeyPair(Task134* manager, const char* keyId) {
    if (!keyId || strlen(keyId) == 0 || manager->keyCount >= MAX_KEYS - 1) {
        return 0;
    }
    
    EVP_PKEY* pkey = EVP_PKEY_new();
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    
    EVP_PKEY_keygen_init(ctx);
    EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, RSA_KEY_SIZE);
    EVP_PKEY_keygen(ctx, &pkey);
    
    BIO* priv_bio = BIO_new(BIO_s_mem());
    BIO* pub_bio = BIO_new(BIO_s_mem());
    
    PEM_write_bio_PrivateKey(priv_bio, pkey, NULL, NULL, 0, NULL, NULL);
    PEM_write_bio_PUBKEY(pub_bio, pkey);
    
    char* priv_data;
    char* pub_data;
    long priv_len = BIO_get_mem_data(priv_bio, &priv_data);
    long pub_len = BIO_get_mem_data(pub_bio, &pub_data);
    
    size_t encPrivLen, encPubLen;
    unsigned char* encPriv = encryptKey(manager, (unsigned char*)priv_data, priv_len, &encPrivLen);
    unsigned char* encPub = encryptKey(manager, (unsigned char*)pub_data, pub_len, &encPubLen);
    
    char privKeyId[MAX_KEY_ID_LENGTH];
    char pubKeyId[MAX_KEY_ID_LENGTH];
    snprintf(privKeyId, MAX_KEY_ID_LENGTH, "%s_private", keyId);
    snprintf(pubKeyId, MAX_KEY_ID_LENGTH, "%s_public", keyId);
    
    strncpy(manager->keys[manager->keyCount].keyId, privKeyId, MAX_KEY_ID_LENGTH - 1);
    manager->keys[manager->keyCount].encryptedKey = encPriv;
    manager->keys[manager->keyCount].keyLength = encPrivLen;
    manager->keyCount++;
    
    strncpy(manager->keys[manager->keyCount].keyId, pubKeyId, MAX_KEY_ID_LENGTH - 1);
    manager->keys[manager->keyCount].encryptedKey = encPub;
    manager->keys[manager->keyCount].keyLength = encPubLen;
    manager->keyCount++;
    
    BIO_free(priv_bio);
    BIO_free(pub_bio);
    EVP_PKEY_free(pkey);
    EVP_PKEY_CTX_free(ctx);
    
    return 1;
}

int keyExists(Task134* manager, const char* keyId) {
    for (int i = 0; i < manager->keyCount; i++) {
        if (strcmp(manager->keys[i].keyId, keyId) == 0) {
            return 1;
        }
    }
    return 0;
}

int deleteKey(Task134* manager, const char* keyId) {
    for (int i = 0; i < manager->keyCount; i++) {
        if (strcmp(manager->keys[i].keyId, keyId) == 0) {
            free(manager->keys[i].encryptedKey);
            for (int j = i; j < manager->keyCount - 1; j++) {
                manager->keys[j] = manager->keys[j + 1];
            }
            manager->keyCount--;
            return 1;
        }
    }
    return 0;
}

void clearAllKeys(Task134* manager) {
    for (int i = 0; i < manager->keyCount; i++) {
        free(manager->keys[i].encryptedKey);
    }
    manager->keyCount = 0;
}

void Task134_destroy(Task134* manager) {
    if (manager) {
        clearAllKeys(manager);
        free(manager);
    }
}

int main() {
    printf("Test Case 1: Initialize Key Manager\\n");
    Task134* keyManager = Task134_create("SecurePassword123!@#");
    printf("Key manager initialized successfully\\n\\n");
    
    printf("Test Case 2: Generate AES Keys\\n");
    generateAESKey(keyManager, "aes-key-001");
    generateAESKey(keyManager, "aes-key-002");
    printf("Generated AES keys: aes-key-001, aes-key-002\\n");
    printf("Key exists (aes-key-001): %d\\n\\n", keyExists(keyManager, "aes-key-001"));
    
    printf("Test Case 3: Generate RSA Key Pairs\\n");
    generateRSAKeyPair(keyManager, "rsa-keypair-001");
    printf("Generated RSA key pair: rsa-keypair-001\\n");
    printf("Private key exists: %d\\n", keyExists(keyManager, "rsa-keypair-001_private"));
    printf("Public key exists: %d\\n\\n", keyExists(keyManager, "rsa-keypair-001_public"));
    
    printf("Test Case 4: List All Keys\\n");
    printf("Total keys stored: %d\\n", keyManager->keyCount);
    printf("Keys: ");
    for (int i = 0; i < keyManager->keyCount; i++) {
        printf("%s ", keyManager->keys[i].keyId);
    }
    printf("\\n\\n");
    
    printf("Test Case 5: Delete Keys\\n");
    int deleted = deleteKey(keyManager, "aes-key-001");
    printf("Deleted aes-key-001: %d\\n", deleted);
    printf("Key exists after deletion: %d\\n", keyExists(keyManager, "aes-key-001"));
    printf("Remaining keys: %d\\n", keyManager->keyCount);
    
    clearAllKeys(keyManager);
    printf("All keys cleared. Remaining keys: %d\\n", keyManager->keyCount);
    
    Task134_destroy(keyManager);
    
    return 0;
}
