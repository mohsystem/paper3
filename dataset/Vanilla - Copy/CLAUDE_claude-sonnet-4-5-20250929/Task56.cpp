
#include <iostream>
#include <string>
#include <random>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <cstring>

// Simple SHA-256 implementation
class SHA256 {
private:
    static const unsigned int K[64];
    
    static unsigned int rotr(unsigned int x, unsigned int n) {
        return (x >> n) | (x << (32 - n));
    }
    
    static unsigned int ch(unsigned int x, unsigned int y, unsigned int z) {
        return (x & y) ^ (~x & z);
    }
    
    static unsigned int maj(unsigned int x, unsigned int y, unsigned int z) {
        return (x & y) ^ (x & z) ^ (y & z);
    }
    
    static unsigned int sigma0(unsigned int x) {
        return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
    }
    
    static unsigned int sigma1(unsigned int x) {
        return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
    }
    
    static unsigned int gamma0(unsigned int x) {
        return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
    }
    
    static unsigned int gamma1(unsigned int x) {
        return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
    }
    
public:
    static std::string hash(const std::string& input) {
        unsigned int h[8] = {
            0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
            0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
        };
        
        std::string padded = input;
        size_t originalLength = input.length() * 8;
        padded += (char)0x80;
        
        while ((padded.length() * 8 + 64) % 512 != 0) {
            padded += (char)0x00;
        }
        
        for (int i = 7; i >= 0; i--) {
            padded += (char)((originalLength >> (i * 8)) & 0xFF);
        }
        
        for (size_t chunk = 0; chunk < padded.length(); chunk += 64) {
            unsigned int w[64] = {0};
            
            for (int i = 0; i < 16; i++) {
                w[i] = ((unsigned char)padded[chunk + i * 4] << 24) |
                       ((unsigned char)padded[chunk + i * 4 + 1] << 16) |
                       ((unsigned char)padded[chunk + i * 4 + 2] << 8) |
                       ((unsigned char)padded[chunk + i * 4 + 3]);
            }
            
            for (int i = 16; i < 64; i++) {
                w[i] = gamma1(w[i - 2]) + w[i - 7] + gamma0(w[i - 15]) + w[i - 16];
            }
            
            unsigned int a = h[0], b = h[1], c = h[2], d = h[3];
            unsigned int e = h[4], f = h[5], g = h[6], hh = h[7];
            
            for (int i = 0; i < 64; i++) {
                unsigned int t1 = hh + sigma1(e) + ch(e, f, g) + K[i] + w[i];
                unsigned int t2 = sigma0(a) + maj(a, b, c);
                hh = g;
                g = f;
                f = e;
                e = d + t1;
                d = c;
                c = b;
                b = a;
                a = t1 + t2;
            }
            
            h[0] += a; h[1] += b; h[2] += c; h[3] += d;
            h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
        }
        
        std::stringstream ss;
        for (int i = 0; i < 8; i++) {
            ss << std::hex << std::setw(8) << std::setfill('0') << h[i];
        }
        return ss.str();
    }
};

const unsigned int SHA256::K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

std::string base64_encode(const std::string& input) {
    static const char base64_chars[] = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    size_t in_len = input.length();
    const char* bytes_to_encode = input.c_str();
    
    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            
            for(i = 0; i < 4; i++)
                ret += base64_chars[char_array_4[i]];
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
            ret += base64_chars[char_array_4[j]];
    }
    
    return ret;
}

std::string generateToken(const std::string& username, long long timestamp) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    std::string randomBytes;
    for (int i = 0; i < 32; i++) {
        randomBytes += (char)dis(gen);
    }
    
    std::string data = username + ":" + std::to_string(timestamp) + ":" + base64_encode(randomBytes);
    std::string hash = SHA256::hash(data);
    
    std::string binaryHash;
    for (size_t i = 0; i < hash.length(); i += 2) {
        std::string byteString = hash.substr(i, 2);
        char byte = (char)strtol(byteString.c_str(), nullptr, 16);
        binaryHash += byte;
    }
    
    return base64_encode(binaryHash);
}

int main() {
    std::cout << "Test Case 1:" << std::endl;
    std::string token1 = generateToken("user123", std::time(nullptr) * 1000);
    std::cout << "Token for user123: " << token1 << std::endl;
    
    std::cout << "\\nTest Case 2:" << std::endl;
    std::string token2 = generateToken("admin", std::time(nullptr) * 1000);
    std::cout << "Token for admin: " << token2 << std::endl;
    
    std::cout << "\\nTest Case 3:" << std::endl;
    std::string token3 = generateToken("testuser@email.com", std::time(nullptr) * 1000);
    std::cout << "Token for testuser@email.com: " << token3 << std::endl;
    
    std::cout << "\\nTest Case 4:" << std::endl;
    std::string token4 = generateToken("john_doe", 1234567890000LL);
    std::cout << "Token for john_doe with fixed timestamp: " << token4 << std::endl;
    
    std::cout << "\\nTest Case 5:" << std::endl;
    std::string token5 = generateToken("guest", std::time(nullptr) * 1000);
    std::cout << "Token for guest: " << token5 << std::endl;
    
    return 0;
}
