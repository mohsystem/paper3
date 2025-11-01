
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

#define SALT_LENGTH 16
#define ITERATIONS 100000
#define KEY_LENGTH 32
#define MAX_USERS 100
#define MAX_USERNAME_LEN 256
#define MAX_STORED_VALUE_LEN 512

typedef struct {
    char username[MAX_USERNAME_LEN];
    char storedValue[MAX_STORED_VALUE_LEN];
} UserRecord;

static UserRecord database[MAX_USERS];
static int userCount = 0;

static const char base64_table[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void base64_encode(const unsigned char* input, size_t len, char* output) {
    int i = 0, j = 0;
    unsigned char array3[3], array4[4];
    
    while (len--) {
        array3[i++] = *(input++);
        if (i == 3) {
            array4[0] = (array3[0] & 0xfc) >> 2;
            array4[1] = ((array3[0] & 0x03) << 4) + ((array3[1] & 0xf0) >> 4);
            array4[2] = ((array3[1] & 0x0f) << 2) + ((array3[2] & 0xc0) >> 6);
            array4[3] = array3[2] & 0x3f;
            
            for (i = 0; i < 4; i++)
                output[j++] = base64_table[array4[i]];
            i = 0;
        }
    }
    
    if (i) {
        for (int k = i; k < 3; k++)
            array3[k] = '\\0';
        
        array4[0] = (array3[0] & 0xfc) >> 2;
        array4[1] = ((array3[0] & 0x03) << 4) + ((array3[1] & 0xf0) >> 4);
        array4[2] = ((array3[1] & 0x0f) << 2) + ((array3[2] & 0xc0) >> 6);
        
        for (int k = 0; k < i + 1; k++)
            output[j++] = base64_table[array4[k]];
        
        while (i++ < 3)
            output[j++] = '=';
    }
    output[j] = '\\0';
}

size_t base64_decode(const char* input, unsigned char* output) {
    size_t len = strlen(input);
    size_t j = 0;
    int val = 0, valb = -8;
    
    for (size_t i = 0; i < len; i++) {
        if (input[i] == '=') break;
        const char* pos = strchr(base64_table, input[i]);
        if (!pos) continue;
        val = (val << 6) + (pos - base64_table);
        valb += 6;
        if (valb >= 0) {
            output[j++] = (val >> valb) & 0xFF;
            valb -= 8;
        }
    }
    return j;
}

int generateSalt(char* salt) {
    unsigned char saltBytes[SALT_LENGTH];
    if (RAND_bytes(saltBytes, SALT_LENGTH) != 1) {
        return 0;
    }
    base64_encode(saltBytes, SALT_LENGTH, salt);
    return 1;
}

int hashPassword(const char* password, const char* salt, char* hashedOutput) {
    unsigned char saltBytes[SALT_LENGTH * 2];
    size_t saltLen = base64_decode(salt, saltBytes);
    unsigned char hash[KEY_LENGTH];
    
    if (PKCS5_PBKDF2_HMAC(password, strlen(password),
                          saltBytes, saltLen,
                          ITERATIONS, EVP_sha256(),
                          KEY_LENGTH, hash) != 1) {
        return 0;
    }
    
    base64_encode(hash, KEY_LENGTH, hashedOutput);
    return 1;
}

int registerUser(const char* username, const char* password) {
    if (!username || strlen(username) == 0 || !password || strlen(password) < 8) {
        return 0;
    }
    
    if (userCount >= MAX_USERS) {
        return 0;
    }
    
    char salt[100];
    char hashedPassword[100];
    
    if (!generateSalt(salt)) {
        return 0;
    }
    
    if (!hashPassword(password, salt, hashedPassword)) {
        return 0;
    }
    
    strncpy(database[userCount].username, username, MAX_USERNAME_LEN - 1);
    snprintf(database[userCount].storedValue, MAX_STORED_VALUE_LEN, 
             "%s:%s", salt, hashedPassword);
    userCount++;
    
    return 1;
}

int verifyUser(const char* username, const char* password) {
    for (int i = 0; i < userCount; i++) {
        if (strcmp(database[i].username, username) == 0) {
            char* colonPos = strchr(database[i].storedValue, ':');
            if (!colonPos) return 0;
            
            char salt[100];
            char storedHash[100];
            size_t saltLen = colonPos - database[i].storedValue;
            strncpy(salt, database[i].storedValue, saltLen);
            salt[saltLen] = '\\0';
            strcpy(storedHash, colonPos + 1);
            
            char computedHash[100];
            if (!hashPassword(password, salt, computedHash)) {
                return 0;
            }
            
            return strcmp(storedHash, computedHash) == 0;
        }
    }
    return 0;
}

int main() {
    printf("Test 1 - Valid registration: %d\\n", 
           registerUser("user1", "SecurePass123!"));
    
    registerUser("user2", "MyP@ssw0rd");
    printf("Test 2 - Valid login: %d\\n", 
           verifyUser("user2", "MyP@ssw0rd"));
    
    printf("Test 3 - Short password: %d\\n", 
           registerUser("user3", "short"));
    
    registerUser("user4", "ValidPass123");
    printf("Test 4 - Wrong password: %d\\n", 
           verifyUser("user4", "WrongPass123"));
    
    printf("Test 5 - Empty username: %d\\n", 
           registerUser("", "ValidPass123"));
    
    return 0;
}
