
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Simple Base64 encoding
void base64_encode(const unsigned char* data, size_t length, char* output) {
    const char* base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    int i = 0, j = 0;
    unsigned char array3[3];
    unsigned char array4[4];
    int out_idx = 0;

    while (length--) {
        array3[i++] = *(data++);
        if (i == 3) {
            array4[0] = (array3[0] & 0xfc) >> 2;
            array4[1] = ((array3[0] & 0x03) << 4) + ((array3[1] & 0xf0) >> 4);
            array4[2] = ((array3[1] & 0x0f) << 2) + ((array3[2] & 0xc0) >> 6);
            array4[3] = array3[2] & 0x3f;
            for(i = 0; i < 4; i++) output[out_idx++] = base64_chars[array4[i]];
            i = 0;
        }
    }

    if (i) {
        for(j = i; j < 3; j++) array3[j] = '\\0';
        array4[0] = (array3[0] & 0xfc) >> 2;
        array4[1] = ((array3[0] & 0x03) << 4) + ((array3[1] & 0xf0) >> 4);
        array4[2] = ((array3[1] & 0x0f) << 2) + ((array3[2] & 0xc0) >> 6);
        for (j = 0; j < i + 1; j++) output[out_idx++] = base64_chars[array4[j]];
    }
    output[out_idx] = '\\0';
}

void simple_hash(const char* input, unsigned char output[32]) {
    size_t len = strlen(input);
    for (int i = 0; i < 32; i++) {
        output[i] = 0;
        for (size_t j = 0; j < len; j++) {
            output[i] ^= (input[j] * (i + j + 1)) % 256;
        }
    }
}

void generateToken(const char* userId, char* token) {
    unsigned char randomBytes[32];
    for (int i = 0; i < 32; i++) {
        randomBytes[i] = rand() % 256;
    }
    
    char randomBase64[64];
    base64_encode(randomBytes, 32, randomBase64);
    
    long long timestamp = (long long)time(NULL) * 1000;
    
    char dataToHash[256];
    sprintf(dataToHash, "%s%lld%s", userId, timestamp, randomBase64);
    
    unsigned char hash[32];
    simple_hash(dataToHash, hash);
    
    base64_encode(hash, 32, token);
}

int main() {
    srand(time(NULL));
    char token[64];
    
    printf("Test Case 1:\\n");
    generateToken("user123", token);
    printf("Token for user123: %s\\n", token);
    
    printf("\\nTest Case 2:\\n");
    generateToken("admin@example.com", token);
    printf("Token for admin@example.com: %s\\n", token);
    
    printf("\\nTest Case 3:\\n");
    generateToken("testUser456", token);
    printf("Token for testUser456: %s\\n", token);
    
    printf("\\nTest Case 4:\\n");
    generateToken("john.doe", token);
    printf("Token for john.doe: %s\\n", token);
    
    printf("\\nTest Case 5:\\n");
    generateToken("user123", token);
    printf("Token for user123 (second generation): %s\\n", token);
    
    return 0;
}
