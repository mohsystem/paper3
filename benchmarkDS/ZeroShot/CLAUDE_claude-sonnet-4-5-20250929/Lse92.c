
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

typedef struct {
    char username[256];
    char hashedPassword[256];
    char salt[256];
} RegistrationResult;

static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void base64_encode(const unsigned char* data, size_t len, char* output) {
    int i = 0;
    int j = 0;
    unsigned char array3[3];
    unsigned char array4[4];
    
    int pos = 0;
    while (len--) {
        array3[i++] = *(data++);
        if (i == 3) {
            array4[0] = (array3[0] & 0xfc) >> 2;
            array4[1] = ((array3[0] & 0x03) << 4) + ((array3[1] & 0xf0) >> 4);
            array4[2] = ((array3[1] & 0x0f) << 2) + ((array3[2] & 0xc0) >> 6);
            array4[3] = array3[2] & 0x3f;
            
            for(i = 0; i < 4; i++)
                output[pos++] = base64_chars[array4[i]];
            i = 0;
        }
    }
    
    if (i) {
        for(j = i; j < 3; j++)
            array3[j] = '\\0';
            
        array4[0] = (array3[0] & 0xfc) >> 2;
        array4[1] = ((array3[0] & 0x03) << 4) + ((array3[1] & 0xf0) >> 4);
        array4[2] = ((array3[1] & 0x0f) << 2) + ((array3[2] & 0xc0) >> 6);
        
        for (j = 0; j < i + 1; j++)
            output[pos++] = base64_chars[array4[j]];
            
        while(i++ < 3)
            output[pos++] = '=';
    }
    output[pos] = '\\0';
}

void generateSalt(char* salt) {
    unsigned char random_bytes[16];
    RAND_bytes(random_bytes, 16);
    base64_encode(random_bytes, 16, salt);
}

void hashPassword(const char* password, const char* salt, char* output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, salt, strlen(salt));
    SHA256_Update(&sha256, password, strlen(password));
    SHA256_Final(hash, &sha256);
    
    base64_encode(hash, SHA256_DIGEST_LENGTH, output);
}

int registerUser(const char* username, const char* password, RegistrationResult* result) {
    if (username == NULL || strlen(username) == 0) {
        return -1;
    }
    if (password == NULL || strlen(password) < 8) {
        return -2;
    }
    
    strcpy(result->username, username);
    generateSalt(result->salt);
    hashPassword(password, result->salt, result->hashedPassword);
    
    return 0;
}

int main() {
    RegistrationResult result;
    
    // Test case 1
    printf("Test 1:\\n");
    if (registerUser("john_doe", "SecurePass123", &result) == 0) {
        printf("Username: %s\\n", result.username);
        printf("Hashed Password: %s\\n", result.hashedPassword);
        printf("Salt: %s\\n\\n", result.salt);
    } else {
        printf("Test 1 failed\\n\\n");
    }
    
    // Test case 2
    printf("Test 2:\\n");
    if (registerUser("alice_smith", "MyP@ssw0rd!", &result) == 0) {
        printf("Username: %s\\n", result.username);
        printf("Hashed Password: %s\\n", result.hashedPassword);
        printf("Salt: %s\\n\\n", result.salt);
    } else {
        printf("Test 2 failed\\n\\n");
    }
    
    // Test case 3 - Empty username
    printf("Test 3 (expected to fail):\\n");
    if (registerUser("", "Password123", &result) != 0) {
        printf("Failed as expected: Username cannot be empty\\n\\n");
    }
    
    // Test case 4 - Short password
    printf("Test 4 (expected to fail):\\n");
    if (registerUser("bob_jones", "short", &result) != 0) {
        printf("Failed as expected: Password too short\\n\\n");
    }
    
    // Test case 5
    printf("Test 5:\\n");
    if (registerUser("emma_wilson", "StrongPassword2024", &result) == 0) {
        printf("Username: %s\\n", result.username);
        printf("Hashed Password: %s\\n", result.hashedPassword);
        printf("Salt: %s\\n", result.salt);
    } else {
        printf("Test 5 failed\\n");
    }
    
    return 0;
}
