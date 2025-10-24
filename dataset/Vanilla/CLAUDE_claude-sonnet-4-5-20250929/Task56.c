
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Simple SHA-256 implementation
static const unsigned int K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

unsigned int rotr(unsigned int x, unsigned int n) {
    return (x >> n) | (x << (32 - n));
}

void sha256(const char* input, size_t length, unsigned char* output) {
    unsigned int h[8] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };
    
    size_t paddedLength = length + 1;
    while ((paddedLength + 8) % 64 != 0) paddedLength++;
    paddedLength += 8;
    
    unsigned char* padded = (unsigned char*)calloc(paddedLength, 1);
    memcpy(padded, input, length);
    padded[length] = 0x80;
    
    unsigned long long bitLength = length * 8;
    for (int i = 0; i < 8; i++) {
        padded[paddedLength - 1 - i] = (bitLength >> (i * 8)) & 0xFF;
    }
    
    for (size_t chunk = 0; chunk < paddedLength; chunk += 64) {
        unsigned int w[64] = {0};
        
        for (int i = 0; i < 16; i++) {
            w[i] = (padded[chunk + i * 4] << 24) |
                   (padded[chunk + i * 4 + 1] << 16) |
                   (padded[chunk + i * 4 + 2] << 8) |
                   (padded[chunk + i * 4 + 3]);
        }
        
        for (int i = 16; i < 64; i++) {
            unsigned int s0 = rotr(w[i-15], 7) ^ rotr(w[i-15], 18) ^ (w[i-15] >> 3);
            unsigned int s1 = rotr(w[i-2], 17) ^ rotr(w[i-2], 19) ^ (w[i-2] >> 10);
            w[i] = w[i-16] + s0 + w[i-7] + s1;
        }
        
        unsigned int a = h[0], b = h[1], c = h[2], d = h[3];
        unsigned int e = h[4], f = h[5], g = h[6], hh = h[7];
        
        for (int i = 0; i < 64; i++) {
            unsigned int S1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
            unsigned int ch = (e & f) ^ (~e & g);
            unsigned int temp1 = hh + S1 + ch + K[i] + w[i];
            unsigned int S0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
            unsigned int maj = (a & b) ^ (a & c) ^ (b & c);
            unsigned int temp2 = S0 + maj;
            
            hh = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }
        
        h[0] += a; h[1] += b; h[2] += c; h[3] += d;
        h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
    }
    
    for (int i = 0; i < 8; i++) {
        output[i * 4] = (h[i] >> 24) & 0xFF;
        output[i * 4 + 1] = (h[i] >> 16) & 0xFF;
        output[i * 4 + 2] = (h[i] >> 8) & 0xFF;
        output[i * 4 + 3] = h[i] & 0xFF;
    }
    
    free(padded);
}

char* base64_encode(const unsigned char* input, size_t length) {
    static const char base64_chars[] = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    
    size_t output_length = 4 * ((length + 2) / 3);
    char* output = (char*)malloc(output_length + 1);
    
    size_t i, j;
    for (i = 0, j = 0; i < length;) {
        unsigned int octet_a = i < length ? input[i++] : 0;
        unsigned int octet_b = i < length ? input[i++] : 0;
        unsigned int octet_c = i < length ? input[i++] : 0;
        
        unsigned int triple = (octet_a << 16) + (octet_b << 8) + octet_c;
        
        output[j++] = base64_chars[(triple >> 18) & 0x3F];
        output[j++] = base64_chars[(triple >> 12) & 0x3F];
        output[j++] = base64_chars[(triple >> 6) & 0x3F];
        output[j++] = base64_chars[triple & 0x3F];
    }
    
    // Remove padding
    while (j > 0 && output[j-1] == 'A') j--;
    output[j] = '\\0';
    
    return output;
}

char* generateToken(const char* username, long long timestamp) {
    unsigned char randomBytes[32];
    for (int i = 0; i < 32; i++) {
        randomBytes[i] = rand() % 256;
    }
    
    char* randomB64 = base64_encode(randomBytes, 32);
    
    char data[512];
    snprintf(data, sizeof(data), "%s:%lld:%s", username, timestamp, randomB64);
    
    unsigned char hash[32];
    sha256(data, strlen(data), hash);
    
    char* token = base64_encode(hash, 32);
    
    free(randomB64);
    return token;
}

int main() {
    srand(time(NULL));
    
    printf("Test Case 1:\\n");
    char* token1 = generateToken("user123", (long long)time(NULL) * 1000);
    printf("Token for user123: %s\\n", token1);
    free(token1);
    
    printf("\\nTest Case 2:\\n");
    char* token2 = generateToken("admin", (long long)time(NULL) * 1000);
    printf("Token for admin: %s\\n", token2);
    free(token2);
    
    printf("\\nTest Case 3:\\n");
    char* token3 = generateToken("testuser@email.com", (long long)time(NULL) * 1000);
    printf("Token for testuser@email.com: %s\\n", token3);
    free(token3);
    
    printf("\\nTest Case 4:\\n");
    char* token4 = generateToken("john_doe", 1234567890000LL);
    printf("Token for john_doe with fixed timestamp: %s\\n", token4);
    free(token4);
    
    printf("\\nTest Case 5:\\n");
    char* token5 = generateToken("guest", (long long)time(NULL) * 1000);
    printf("Token for guest: %s\\n", token5);
    free(token5);
    
    return 0;
}
