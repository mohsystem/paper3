
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

#define KEY_SIZE 32
#define IV_SIZE 12
#define TAG_SIZE 16
#define MAX_STORAGE 100
#define MAX_KEY_LEN 256
#define MAX_DATA_LEN 2048

typedef struct {
    char key[MAX_KEY_LEN];
    char encrypted_data[MAX_DATA_LEN];
} StorageEntry;

typedef struct {
    unsigned char encryption_key[KEY_SIZE];
    StorageEntry storage[MAX_STORAGE];
    int storage_count;
} Task124;

static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char* base64_encode(const unsigned char* data, size_t input_length, size_t* output_length) {
    *output_length = 4 * ((input_length + 2) / 3);
    char* encoded_data = malloc(*output_length + 1);
    if (encoded_data == NULL) return NULL;
    
    size_t i, j;
    for (i = 0, j = 0; i < input_length;) {
        uint32_t octet_a = i < input_length ? data[i++] : 0;
        uint32_t octet_b = i < input_length ? data[i++] : 0;
        uint32_t octet_c = i < input_length ? data[i++] : 0;
        uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;
        
        encoded_data[j++] = base64_chars[(triple >> 18) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 12) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 6) & 0x3F];
        encoded_data[j++] = base64_chars[triple & 0x3F];
    }
    
    for (i = 0; i < (3 - input_length % 3) % 3; i++) {
        encoded_data[*output_length - 1 - i] = '=';
    }
    
    encoded_data[*output_length] = '\\0';
    return encoded_data;
}

unsigned char* base64_decode(const char* data, size_t input_length, size_t* output_length) {
    if (input_length % 4 != 0) return NULL;
    
    *output_length = input_length / 4 * 3;
    if (data[input_length - 1] == '=') (*output_length)--;
    if (data[input_length - 2] == '=') (*output_length)--;
    
    unsigned char* decoded_data = malloc(*output_length);
    if (decoded_data == NULL) return NULL;
    
    unsigned char decoding_table[256];
    for (int i = 0; i < 64; i++) {
        decoding_table[(unsigned char)base64_chars[i]] = i;
    }
    
    size_t i, j;
    for (i = 0, j = 0; i < input_length;) {
        uint32_t sextet_a = data[i] == '=' ? 0 : decoding_table[(unsigned char)data[i]]; i++;
        uint32_t sextet_b = data[i] == '=' ? 0 : decoding_table[(unsigned char)data[i]]; i++;
        uint32_t sextet_c = data[i] == '=' ? 0 : decoding_table[(unsigned char)data[i]]; i++;
        uint32_t sextet_d = data[i] == '=' ? 0 : decoding_table[(unsigned char)data[i]]; i++;
        
        uint32_t triple = (sextet_a << 18) + (sextet_b << 12) + (sextet_c << 6) + sextet_d;
        
        if (j < *output_length) decoded_data[j++] = (triple >> 16) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = triple & 0xFF;
    }
    
    return decoded_data;
}

Task124* task124_init() {
    Task124* storage = malloc(sizeof(Task124));
    if (storage == NULL) return NULL;
    
    if (RAND_bytes(storage->encryption_key, KEY_SIZE) != 1) {
        free(storage);
        return NULL;
    }
    
    storage->storage_count = 0;
    return storage;
}

char* encrypt_data(Task124* storage, const char* plain_text) {
    if (plain_text == NULL || strlen(plain_text) == 0) return NULL;
    
    unsigned char iv[IV_SIZE];
    if (RAND_bytes(iv, IV_SIZE) != 1) return NULL;
    
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL) return NULL;
    
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, storage->encryption_key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }
    
    int len;
    int plaintext_len = strlen(plain_text);
    unsigned char* ciphertext = malloc(plaintext_len + EVP_CIPHER_block_size(EVP_aes_256_gcm()));
    
    if (EVP_EncryptUpdate(ctx, ciphertext, &len, (unsigned char*)plain_text, plaintext_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(ciphertext);
        return NULL;
    }
    int ciphertext_len = len;
    
    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(ciphertext);
        return NULL;
    }
    ciphertext_len += len;
    
    unsigned char tag[TAG_SIZE];
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_SIZE, tag) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(ciphertext);
        return NULL;
    }
    
    EVP_CIPHER_CTX_free(ctx);
    
    size_t total_len = IV_SIZE + ciphertext_len + TAG_SIZE;
    unsigned char* result = malloc(total_len);
    memcpy(result, iv, IV_SIZE);
    memcpy(result + IV_SIZE, ciphertext, ciphertext_len);
    memcpy(result + IV_SIZE + ciphertext_len, tag, TAG_SIZE);
    
    free(ciphertext);
    
    size_t encoded_len;
    char* encoded = base64_encode(result, total_len, &encoded_len);
    free(result);
    
    return encoded;
}

char* decrypt_data(Task124* storage, const char* encrypted_text) {
    if (encrypted_text == NULL || strlen(encrypted_text) == 0) return NULL;
    
    size_t decoded_len;
    unsigned char* decoded = base64_decode(encrypted_text, strlen(encrypted_text), &decoded_len);
    if (decoded == NULL) return NULL;
    
    if (decoded_len < IV_SIZE + TAG_SIZE) {
        free(decoded);
        return NULL;
    }
    
    unsigned char* iv = decoded;
    unsigned char* ciphertext = decoded + IV_SIZE;
    int ciphertext_len = decoded_len - IV_SIZE - TAG_SIZE;
    unsigned char* tag = decoded + IV_SIZE + ciphertext_len;
    
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL) {
        free(decoded);
        return NULL;
    }
    
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, storage->encryption_key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(decoded);
        return NULL;
    }
    
    int len;
    unsigned char* plaintext = malloc(ciphertext_len + 1);
    
    if (EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(decoded);
        free(plaintext);
        return NULL;
    }
    int plaintext_len = len;
    
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_SIZE, tag) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(decoded);
        free(plaintext);
        return NULL;
    }
    
    if (EVP_DecryptFinal_ex(ctx, plaintext + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(decoded);
        free(plaintext);
        return NULL;
    }
    plaintext_len += len;
    plaintext[plaintext_len] = '\\0';
    
    EVP_CIPHER_CTX_free(ctx);
    free(decoded);
    
    return (char*)plaintext;
}

int store_sensitive_data(Task124* storage, const char* key, const char* data) {
    if (key == NULL || strlen(key) == 0 || storage->storage_count >= MAX_STORAGE) {
        return -1;
    }
    
    char* encrypted = encrypt_data(storage, data);
    if (encrypted == NULL) return -1;
    
    strncpy(storage->storage[storage->storage_count].key, key, MAX_KEY_LEN - 1);
    strncpy(storage->storage[storage->storage_count].encrypted_data, encrypted, MAX_DATA_LEN - 1);
    storage->storage_count++;
    
    free(encrypted);
    return 0;
}

char* retrieve_sensitive_data(Task124* storage, const char* key) {
    if (key == NULL || strlen(key) == 0) return NULL;
    
    for (int i = 0; i < storage->storage_count; i++) {
        if (strcmp(storage->storage[i].key, key) == 0) {
            return decrypt_data(storage, storage->storage[i].encrypted_data);
        }
    }
    return NULL;
}

void task124_free(Task124* storage) {
    if (storage != NULL) {
        memset(storage->encryption_key, 0, KEY_SIZE);
        free(storage);
    }
}

int main() {
    Task124* secure_storage = task124_init();
    if (secure_storage == NULL) {
        fprintf(stderr, "Failed to initialize secure storage\\n");
        return 1;
    }
    
    // Test Case 1: Store and retrieve credit card number
    printf("Test Case 1: Credit Card Storage\\n");
    store_sensitive_data(secure_storage, "creditCard1", "4532-1234-5678-9010");
    char* retrieved_cc = retrieve_sensitive_data(secure_storage, "creditCard1");
    printf("Retrieved: %s\\n\\n", retrieved_cc ? retrieved_cc : "NULL");
    free(retrieved_cc);
    
    // Test Case 2: Store and retrieve personal information
    printf("Test Case 2: Personal Information\\n");
    store_sensitive_data(secure_storage, "ssn1", "123-45-6789");
    char* retrieved_ssn = retrieve_sensitive_data(secure_storage, "ssn1");
    printf("Retrieved: %s\\n\\n", retrieved_ssn ? retrieved_ssn : "NULL");
    free(retrieved_ssn);
    
    // Test Case 3: Store and retrieve email
    printf("Test Case 3: Email Storage\\n");
    store_sensitive_data(secure_storage, "email1", "user@example.com");
    char* retrieved_email = retrieve_sensitive_data(secure_storage, "email1");
    printf("Retrieved: %s\\n\\n", retrieved_email ? retrieved_email : "NULL");
    free(retrieved_email);
    
    // Test Case 4: Store and retrieve medical records
    printf("Test Case 4: Medical Records\\n");
    store_sensitive_data(secure_storage, "medical1", "Patient: John Doe, Condition: Confidential");
    char* retrieved_medical = retrieve_sensitive_data(secure_storage, "medical1");
    printf("Retrieved: %s\\n\\n", retrieved_medical ? retrieved_medical : "NULL");
    free(retrieved_medical);
    
    // Test Case 5: Retrieve non-existent data
    printf("Test Case 5: Non-existent Key\\n");
    char* non_existent = retrieve_sensitive_data(secure_storage, "nonExistent");
    printf("Retrieved: %s\\n", non_existent ? non_existent : "NULL");
    free(non_existent);
    
    task124_free(secure_storage);
    return 0;
}
