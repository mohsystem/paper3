
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_KEYS 100
#define MAX_KEY_ID_LEN 50
#define MAX_KEY_LEN 256

typedef struct {
    char keyId[MAX_KEY_ID_LEN];
    char key[MAX_KEY_LEN];
    int isActive;
} KeyEntry;

typedef struct {
    KeyEntry keys[MAX_KEYS];
    int count;
} KeyStore;

void initKeyStore(KeyStore* store) {
    store->count = 0;
    for (int i = 0; i < MAX_KEYS; i++) {
        store->keys[i].isActive = 0;
    }
    srand(time(NULL));
}

void generateKey(KeyStore* store, const char* keyId, int keySize, char* output) {
    int keySizeBytes = keySize / 8;
    
    for (int i = 0; i < store->count; i++) {
        if (strcmp(store->keys[i].keyId, keyId) == 0 && store->keys[i].isActive) {
            // Update existing key
            output[0] = '\\0';
            for (int j = 0; j < keySizeBytes && j < MAX_KEY_LEN / 2; j++) {
                unsigned char byte = rand() % 256;
                sprintf(output + strlen(output), "%02x", byte);
            }
            strcpy(store->keys[i].key, output);
            return;
        }
    }
    
    if (store->count < MAX_KEYS) {
        strcpy(store->keys[store->count].keyId, keyId);
        output[0] = '\\0';
        for (int j = 0; j < keySizeBytes && j < MAX_KEY_LEN / 2; j++) {
            unsigned char byte = rand() % 256;
            sprintf(output + strlen(output), "%02x", byte);
        }
        strcpy(store->keys[store->count].key, output);
        store->keys[store->count].isActive = 1;
        store->count++;
    } else {
        strcpy(output, "Error: Key store full");
    }
}

void getKey(KeyStore* store, const char* keyId, char* output) {
    for (int i = 0; i < store->count; i++) {
        if (strcmp(store->keys[i].keyId, keyId) == 0 && store->keys[i].isActive) {
            strcpy(output, store->keys[i].key);
            return;
        }
    }
    strcpy(output, "Key not found");
}

int deleteKey(KeyStore* store, const char* keyId) {
    for (int i = 0; i < store->count; i++) {
        if (strcmp(store->keys[i].keyId, keyId) == 0 && store->keys[i].isActive) {
            store->keys[i].isActive = 0;
            return 1;
        }
    }
    return 0;
}

void listKeys(KeyStore* store) {
    printf("Active keys: ");
    for (int i = 0; i < store->count; i++) {
        if (store->keys[i].isActive) {
            printf("%s ", store->keys[i].keyId);
        }
    }
    printf("\\n");
}

void rotateKey(KeyStore* store, const char* keyId, int keySize, char* output) {
    for (int i = 0; i < store->count; i++) {
        if (strcmp(store->keys[i].keyId, keyId) == 0 && store->keys[i].isActive) {
            generateKey(store, keyId, keySize, output);
            return;
        }
    }
    strcpy(output, "Key not found");
}

int main() {
    KeyStore store;
    initKeyStore(&store);
    char buffer[MAX_KEY_LEN];
    
    printf("Test Case 1: Generate and retrieve keys\\n");
    generateKey(&store, "key1", 128, buffer);
    printf("Generated key1: %s\\n", buffer);
    getKey(&store, "key1", buffer);
    printf("Retrieved key1: %s\\n\\n", buffer);
    
    printf("Test Case 2: List all keys\\n");
    generateKey(&store, "key2", 256, buffer);
    generateKey(&store, "key3", 128, buffer);
    listKeys(&store);
    printf("\\n");
    
    printf("Test Case 3: Encrypt and decrypt data\\n");
    printf("(Simplified demonstration - not actual encryption)\\n");
    getKey(&store, "key1", buffer);
    printf("Using key1 for operations: %s\\n\\n", buffer);
    
    printf("Test Case 4: Rotate key\\n");
    getKey(&store, "key2", buffer);
    printf("Old key2: %s\\n", buffer);
    rotateKey(&store, "key2", 256, buffer);
    printf("New key2: %s\\n\\n", buffer);
    
    printf("Test Case 5: Delete key\\n");
    printf("Delete key3: %s\\n", deleteKey(&store, "key3") ? "true" : "false");
    listKeys(&store);
    getKey(&store, "key3", buffer);
    printf("Try to get deleted key: %s\\n", buffer);
    
    return 0;
}
