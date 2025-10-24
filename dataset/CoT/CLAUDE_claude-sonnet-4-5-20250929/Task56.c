
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

#define TOKEN_LENGTH 32
#define MAX_TOKEN_SIZE 256

static const char base64_chars[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

void base64_encode(const unsigned char* input, size_t length, char* output) {
    int i = 0, j = 0, idx = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    
    while (length--) {
        char_array_3[i++] = *(input++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            
            for(i = 0; i < 4; i++)
                output[idx++] = base64_chars[char_array_4[i]];
            i = 0;
        }
    }
    
    if (i) {
        for(j = i; j < 3; j++)
            char_array_3[j] = '\\0';
        
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        
        for (j = 0; j < i + 1; j++)
            output[idx++] = base64_chars[char_array_4[j]];
    }
    
    output[idx] = '\\0';
}

void generateToken(const char* userId, long long timestamp, char* token) {
    unsigned char randomBytes[TOKEN_LENGTH];
    char randomBytesB64[TOKEN_LENGTH * 2];
    char combinedData[MAX_TOKEN_SIZE];
    unsigned char hash[SHA256_DIGEST_LENGTH];
    
    // Get current timestamp if not provided
    if (timestamp == -1) {
        timestamp = (long long)time(NULL) * 1000;
    }
    
    // Generate secure random bytes using OpenSSL
    if (RAND_bytes(randomBytes, TOKEN_LENGTH) != 1) {
        fprintf(stderr, "Error generating random bytes\\n");
        token[0] = '\\0';
        return;
    }
    
    // Encode random bytes to Base64
    base64_encode(randomBytes, TOKEN_LENGTH, randomBytesB64);
    
    // Combine data
    snprintf(combinedData, MAX_TOKEN_SIZE, "%s:%lld:%s", userId, timestamp, randomBytesB64);
    
    // Hash using SHA-256
    SHA256((unsigned char*)combinedData, strlen(combinedData), hash);
    
    // Encode hash to Base64
    base64_encode(hash, SHA256_DIGEST_LENGTH, token);
}

int main() {
    char token[MAX_TOKEN_SIZE];
    
    // Test case 1: Generate token for user1
    generateToken("user1", -1, token);
    printf("Test 1 - Token for user1: %s\\n", token);
    
    // Test case 2: Generate token for user2
    generateToken("user2", -1, token);
    printf("Test 2 - Token for user2: %s\\n", token);
    
    // Test case 3: Generate token with specific timestamp
    generateToken("user3", 1700000000000LL, token);
    printf("Test 3 - Token for user3 with timestamp: %s\\n", token);
    
    // Test case 4: Verify tokens are unique for same user
    char token4a[MAX_TOKEN_SIZE];
    char token4b[MAX_TOKEN_SIZE];
    generateToken("user4", -1, token4a);
    generateToken("user4", -1, token4b);
    printf("Test 4 - Two tokens for user4 are different: %s\\n", 
           strcmp(token4a, token4b) != 0 ? "true" : "false");
    
    // Test case 5: Generate token for admin user
    generateToken("admin", -1, token);
    printf("Test 5 - Token for admin: %s\\n", token);
    
    return 0;
}
