
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#define ITERATIONS 100000
#define SALT_LENGTH 16
#define KEY_LENGTH 32
#define MAX_USERNAME_LENGTH 256
#define MAX_STORED_PASSWORD_LENGTH 512

typedef struct {
    char username[MAX_USERNAME_LENGTH];
    char storedPassword[MAX_STORED_PASSWORD_LENGTH];
} UserCredentials;

static const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void base64_encode(const unsigned char* input, size_t length, char* output) {
    size_t i = 0, j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    
    while (length--) {
        char_array_3[i++] = *(input++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            
            for (i = 0; i < 4; i++)
                output[j++] = base64_table[char_array_4[i]];
            i = 0;
        }
    }
    
    if (i) {
        for (size_t k = i; k < 3; k++)
            char_array_3[k] = '\\0';
        
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        
        for (size_t k = 0; k < i + 1; k++)
            output[j++] = base64_table[char_array_4[k]];
        
        while (i++ < 3)
            output[j++] = '=';
    }
    output[j] = '\\0';
}

size_t base64_decode(const char* input, unsigned char* output) {
    size_t length = strlen(input);
    size_t i = 0, j = 0;
    unsigned char char_array_4[4], char_array_3[3];
    
    while (length-- && input[i] != '=') {
        const char* pos = strchr(base64_table, input[i]);
        if (!pos) {
            i++;
            continue;
        }
        char_array_4[j++] = pos - base64_table;
        i++;
        
        if (j == 4) {
            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
            
            memcpy(output, char_array_3, 3);
            output += 3;
            j = 0;
        }
    }
    
    if (j) {
        for (size_t k = j; k < 4; k++)
            char_array_4[k] = 0;
        
        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        
        for (size_t k = 0; k < j - 1; k++)
            *(output++) = char_array_3[k];
    }
    
    return i;
}

void generateSalt(unsigned char* salt) {
    RAND_bytes(salt, SALT_LENGTH);
}

void hashPassword(const char* password, const unsigned char* salt, unsigned char* hash) {
    PKCS5_PBKDF2_HMAC(password, strlen(password), salt, SALT_LENGTH, 
                      ITERATIONS, EVP_sha256(), KEY_LENGTH, hash);
}

int slowEquals(const unsigned char* a, const unsigned char* b, size_t length) {
    unsigned char result = 0;
    for (size_t i = 0; i < length; i++) {
        result |= a[i] ^ b[i];
    }
    return result == 0;
}

void registerUser(const char* username, const char* password, UserCredentials* creds) {
    unsigned char salt[SALT_LENGTH];
    unsigned char hash[KEY_LENGTH];
    char saltBase64[64];
    char hashBase64[64];
    
    generateSalt(salt);
    hashPassword(password, salt, hash);
    
    base64_encode(salt, SALT_LENGTH, saltBase64);
    base64_encode(hash, KEY_LENGTH, hashBase64);
    
    strncpy(creds->username, username, MAX_USERNAME_LENGTH - 1);
    creds->username[MAX_USERNAME_LENGTH - 1] = '\\0';
    
    snprintf(creds->storedPassword, MAX_STORED_PASSWORD_LENGTH, "%s:%s", saltBase64, hashBase64);
}

int verifyPassword(const char* password, const char* storedPassword) {
    char storedPasswordCopy[MAX_STORED_PASSWORD_LENGTH];
    strncpy(storedPasswordCopy, storedPassword, MAX_STORED_PASSWORD_LENGTH - 1);
    storedPasswordCopy[MAX_STORED_PASSWORD_LENGTH - 1] = '\\0';
    
    char* colon = strchr(storedPasswordCopy, ':');
    if (!colon) return 0;
    
    *colon = '\\0';
    char* saltBase64 = storedPasswordCopy;
    char* hashBase64 = colon + 1;
    
    unsigned char salt[SALT_LENGTH];
    unsigned char storedHash[KEY_LENGTH];
    unsigned char computedHash[KEY_LENGTH];
    
    base64_decode(saltBase64, salt);
    base64_decode(hashBase64, storedHash);
    
    hashPassword(password, salt, computedHash);
    
    return slowEquals(storedHash, computedHash, KEY_LENGTH);
}

int main() {
    UserCredentials creds1, creds2, creds3, creds4, creds5;
    
    // Test Case 1: Basic registration
    registerUser("alice", "SecurePass123!", &creds1);
    printf("Test 1 - Registered: %s\\n", creds1.username);
    printf("Verification: %s\\n", verifyPassword("SecurePass123!", creds1.storedPassword) ? "true" : "false");
    
    // Test Case 2: Different user
    registerUser("bob", "MyPassword456@", &creds2);
    printf("\\nTest 2 - Registered: %s\\n", creds2.username);
    printf("Verification: %s\\n", verifyPassword("MyPassword456@", creds2.storedPassword) ? "true" : "false");
    
    // Test Case 3: Wrong password
    printf("\\nTest 3 - Wrong password: %s\\n", verifyPassword("WrongPass", creds1.storedPassword) ? "true" : "false");
    
    // Test Case 4: Complex password
    registerUser("charlie", "C0mpl3x!P@ssw0rd#2024", &creds3);
    printf("\\nTest 4 - Registered: %s\\n", creds3.username);
    printf("Verification: %s\\n", verifyPassword("C0mpl3x!P@ssw0rd#2024", creds3.storedPassword) ? "true" : "false");
    
    // Test Case 5: Another user
    registerUser("david", "AnotherSecure987$", &creds4);
    printf("\\nTest 5 - Registered: %s\\n", creds4.username);
    printf("Verification: %s\\n", verifyPassword("AnotherSecure987$", creds4.storedPassword) ? "true" : "false");
    
    return 0;
}
