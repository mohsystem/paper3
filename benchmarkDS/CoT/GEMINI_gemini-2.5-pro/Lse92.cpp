#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <sstream>
#include <iomanip>

// --- Start of Single-Header SHA256 Implementation ---
// Based on the public domain implementation by Olivier Gay
// Source: https://github.com/ogay/sha256
#ifndef SHA256_H
#define SHA256_H

#include <string>
#include <vector>
#include <cstddef>

class SHA256 {
protected:
    typedef unsigned char uint8;
    typedef unsigned int uint32;
    typedef unsigned long long uint64;

    const static uint32 sha256_k[];
    static const unsigned int SHA224_256_BLOCK_SIZE = (512 / 8);

public:
    void init();
    void update(const unsigned char *message, unsigned int len);
    void final(unsigned char *digest);
    static std::string from(const std::string &str);

protected:
    void transform(const unsigned char *message, unsigned int block_nb);
    unsigned int m_tot_len;
    unsigned int m_len;
    unsigned char m_block[2 * SHA224_256_BLOCK_SIZE];
    uint32 m_h[8];
};

std::string sha256(const std::string& str);

const unsigned int SHA256::sha256_k[64] = // UL = uint32
        {0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
         0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
         0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
         0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
         0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
         0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
         0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
         0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
         0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
         0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
         0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
         0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
         0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
         0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
         0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
         0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

#define SHA2_SHFR(x, n)    (x >> n)
#define SHA2_ROTR(x, n)   ((x >> n) | (x << ((sizeof(x) << 3) - n)))
#define SHA2_ROTL(x, n)   ((x << n) | (x >> ((sizeof(x) << 3) - n)))
#define SHA2_CH(x, y, z)  ((x & y) ^ (~x & z))
#define SHA2_MAJ(x, y, z) ((x & y) ^ (x & z) ^ (y & z))
#define SHA256_F1(x) (SHA2_ROTR(x,  2) ^ SHA2_ROTR(x, 13) ^ SHA2_ROTR(x, 22))
#define SHA256_F2(x) (SHA2_ROTR(x,  6) ^ SHA2_ROTR(x, 11) ^ SHA2_ROTR(x, 25))
#define SHA256_F3(x) (SHA2_ROTR(x,  7) ^ SHA2_ROTR(x, 18) ^ SHA2_SHFR(x,  3))
#define SHA256_F4(x) (SHA2_ROTR(x, 17) ^ SHA2_ROTR(x, 19) ^ SHA2_SHFR(x, 10))
#define SHA2_UNPACK32(x, str) { \
    *((str) + 3) = (uint8) ((x)      ); \
    *((str) + 2) = (uint8) ((x) >>  8); \
    *((str) + 1) = (uint8) ((x) >> 16); \
    *((str) + 0) = (uint8) ((x) >> 24); \
}
#define SHA2_PACK32(str, x) { \
    *(x) =   ((uint32) *((str) + 3)      ) \
           | ((uint32) *((str) + 2) <<  8) \
           | ((uint32) *((str) + 1) << 16) \
           | ((uint32) *((str) + 0) << 24); \
}

void SHA256::init() {
    m_h[0] = 0x6a09e667; m_h[1] = 0xbb67ae85; m_h[2] = 0x3c6ef372; m_h[3] = 0xa54ff53a;
    m_h[4] = 0x510e527f; m_h[5] = 0x9b05688c; m_h[6] = 0x1f83d9ab; m_h[7] = 0x5be0cd19;
    m_len = 0; m_tot_len = 0;
}

void SHA256::update(const unsigned char *message, unsigned int len) {
    unsigned int block_nb;
    unsigned int new_len, rem_len, tmp_len;
    const unsigned char *shifted_message;
    tmp_len = SHA224_256_BLOCK_SIZE - m_len;
    rem_len = len < tmp_len ? len : tmp_len;
    memcpy(&m_block[m_len], message, rem_len);
    if (m_len + len < SHA224_256_BLOCK_SIZE) {
        m_len += len;
        return;
    }
    new_len = len - rem_len;
    block_nb = new_len / SHA224_256_BLOCK_SIZE;
    shifted_message = message + rem_len;
    transform(m_block, 1);
    transform(shifted_message, block_nb);
    rem_len = new_len % SHA224_256_BLOCK_SIZE;
    memcpy(m_block, &shifted_message[block_nb << 6], rem_len);
    m_len = rem_len;
    m_tot_len += (block_nb + 1) << 6;
}

void SHA256::final(unsigned char *digest) {
    unsigned int block_nb;
    unsigned int pm_len;
    unsigned int len_b;
    int i;
    block_nb = (1 + ((SHA224_256_BLOCK_SIZE - 9) < (m_len % SHA224_256_BLOCK_SIZE)));
    len_b = (m_tot_len + m_len) << 3;
    pm_len = block_nb << 6;
    memset(m_block + m_len, 0, pm_len - m_len);
    m_block[m_len] = 0x80;
    SHA2_UNPACK32(len_b, m_block + pm_len - 4);
    transform(m_block, block_nb);
    for (i = 0; i < 8; i++) {
        SHA2_UNPACK32(m_h[i], &digest[i << 2]);
    }
}

std::string sha256(const std::string& str) {
    unsigned char digest[32];
    SHA256 ctx = SHA256();
    ctx.init();
    ctx.update((unsigned char*)str.c_str(), str.length());
    ctx.final(digest);

    std::stringstream ss;
    for(int i=0; i<32; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)digest[i];
    }
    return ss.str();
}
#endif
// --- End of Single-Header SHA256 Implementation ---


/**
 * !!!!!!!!!!!!!!!!!!!!!!!!!!! SECURITY WARNING !!!!!!!!!!!!!!!!!!!!!!!!!!!
 * This function is for DEMONSTRATION PURPOSES ONLY.
 * Using a fast hashing algorithm like SHA-256 directly for passwords is INSECURE.
 * It is vulnerable to brute-force and dictionary attacks.
 * In a real application, you MUST use a key derivation function (KDF) designed
 * for password hashing, such as Argon2, scrypt, or PBKDF2.
 * These functions are slow and memory-hard, which makes attacks much more difficult.
 * Use a well-vetted cryptography library like OpenSSL or libsodium to implement this.
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */
std::string hashPassword(const std::string& password) {
    // 1. Generate a random salt
    std::string salt;
    salt.resize(16);
    std::random_device rd;
    std::uniform_int_distribution<int> dist(0, 255);
    for (size_t i = 0; i < salt.size(); ++i) {
        salt[i] = static_cast<char>(dist(rd));
    }

    // 2. Combine password and salt
    std::string to_hash = password + salt;

    // 3. Hash the combination
    std::string hash = sha256(to_hash);

    // 4. Format for storage (salt:hash)
    std::stringstream ss_salt;
    for(unsigned char c : salt) {
        ss_salt << std::hex << std::setw(2) << std::setfill('0') << (int)c;
    }

    return ss_salt.str() + ":" + hash;
}

void registerUser(const std::string& username, const std::string& password) {
    std::cout << "Registering user: " << username << std::endl;
    std::string hashedPassword = hashPassword(password);
    std::cout << "Stored information for " << username << ":" << std::endl;
    std::cout << "  Username: " << username << std::endl;
    std::cout << "  Hashed Password (salt:hash): " << hashedPassword << std::endl;
    std::cout << "------------------------------------" << std::endl;
}

int main() {
    // 5 Test Cases
    registerUser("alice", "Password123!");
    registerUser("bob", "My$trongP@ssw0rd");
    registerUser("charlie", "Secr3t#Key");
    registerUser("david", "123456");
    registerUser("eve", "password");
    return 0;
}