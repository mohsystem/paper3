#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <cstdint>
#include <algorithm>

// ---- BEGIN: Self-contained SHA-256 implementation ----
// Based on the public domain implementation by `zvr/hmac_sha256`
// This is included to make the code a single, dependency-free file.
class SHA256 {
public:
    SHA256();
    void update(const uint8_t *data, size_t length);
    void update(const std::string &data);
    std::vector<uint8_t> digest();

private:
    void transform(const uint8_t *message);
    uint8_t m_buffer[64];
    size_t m_buffer_len;
    uint32_t m_state[8];
    uint64_t m_bit_count;
};

#define SHA256_ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
#define SHA256_CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define SHA256_MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define SHA256_BSIG0(x) (SHA256_ROTR(x, 2) ^ SHA256_ROTR(x, 13) ^ SHA256_ROTR(x, 22))
#define SHA256_BSIG1(x) (SHA256_ROTR(x, 6) ^ SHA256_ROTR(x, 11) ^ SHA256_ROTR(x, 25))
#define SHA256_SSIG0(x) (SHA256_ROTR(x, 7) ^ SHA256_ROTR(x, 18) ^ ((x) >> 3))
#define SHA256_SSIG1(x) (SHA256_ROTR(x, 17) ^ SHA256_ROTR(x, 19) ^ ((x) >> 10))

static const uint32_t k[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

SHA256::SHA256() : m_buffer_len(0), m_bit_count(0) {
    m_state[0] = 0x6a09e667;
    m_state[1] = 0xbb67ae85;
    m_state[2] = 0x3c6ef372;
    m_state[3] = 0xa54ff53a;
    m_state[4] = 0x510e527f;
    m_state[5] = 0x9b05688c;
    m_state[6] = 0x1f83d9ab;
    m_state[7] = 0x5be0cd19;
}

void SHA256::transform(const uint8_t *message) {
    uint32_t w[64];
    for (int i = 0; i < 16; ++i) {
        w[i] = (message[i * 4] << 24) | (message[i * 4 + 1] << 16) | (message[i * 4 + 2] << 8) | message[i * 4 + 3];
    }
    for (int i = 16; i < 64; ++i) {
        w[i] = SHA256_SSIG1(w[i - 2]) + w[i - 7] + SHA256_SSIG0(w[i - 15]) + w[i - 16];
    }
    uint32_t a = m_state[0], b = m_state[1], c = m_state[2], d = m_state[3],
             e = m_state[4], f = m_state[5], g = m_state[6], h = m_state[7];
    for (int i = 0; i < 64; ++i) {
        uint32_t t1 = h + SHA256_BSIG1(e) + SHA256_CH(e, f, g) + k[i] + w[i];
        uint32_t t2 = SHA256_BSIG0(a) + SHA256_MAJ(a, b, c);
        h = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
    }
    m_state[0] += a; m_state[1] += b; m_state[2] += c; m_state[3] += d;
    m_state[4] += e; m_state[5] += f; m_state[6] += g; m_state[7] += h;
}

void SHA256::update(const uint8_t *data, size_t length) {
    for (size_t i = 0; i < length; ++i) {
        m_buffer[m_buffer_len++] = data[i];
        if (m_buffer_len == 64) {
            transform(m_buffer);
            m_bit_count += 512;
            m_buffer_len = 0;
        }
    }
}

void SHA256::update(const std::string &data) {
    update(reinterpret_cast<const uint8_t*>(data.c_str()), data.length());
}

std::vector<uint8_t> SHA256::digest() {
    uint8_t PADDING[64] = { 0x80 };
    uint64_t original_bit_count = m_bit_count + (m_buffer_len * 8);
    size_t pad_len = (m_buffer_len < 56) ? (56 - m_buffer_len) : (120 - m_buffer_len);
    update(PADDING, pad_len);

    for (int i = 7; i >= 0; --i) {
        m_buffer[56 + i] = (original_bit_count >> ((7 - i) * 8)) & 0xFF;
    }
    transform(m_buffer);

    std::vector<uint8_t> hash(32);
    for (int i = 0; i < 8; ++i) {
        hash[i*4] = (m_state[i] >> 24) & 0xFF;
        hash[i*4 + 1] = (m_state[i] >> 16) & 0xFF;
        hash[i*4 + 2] = (m_state[i] >> 8) & 0xFF;
        hash[i*4 + 3] = m_state[i] & 0xFF;
    }
    return hash;
}
// ---- END: Self-contained SHA-256 implementation ----

/**
 * Hashes a password using SHA-256 and simulates inserting the user record.
 *
 * @param username The username to be stored.
 * @param password The plaintext password to be hashed.
 * @return The hex-encoded SHA-256 hash of the password.
 */
std::string registerUser(const std::string& username, const std::string& password) {
    if (username.empty() || password.empty()) {
        std::cout << "Error: Username and password cannot be empty." << std::endl;
        return "";
    }

    SHA256 sha;
    sha.update(password);
    std::vector<uint8_t> digest = sha.digest();
    
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for(uint8_t i : digest) {
        ss << std::setw(2) << static_cast<unsigned>(i);
    }
    std::string hashedPassword = ss.str();
    
    // Simulate inserting into the database
    std::cout << "--- User Registration ---" << std::endl;
    std::cout << "Simulating database insertion..." << std::endl;
    std::cout << "Username: " << username << std::endl;
    std::cout << "Stored Hashed Password: " << hashedPassword << std::endl;
    std::cout << "-------------------------" << std::endl;

    return hashedPassword;
}


int main() {
    std::cout << "Running 5 test cases..." << std::endl;

    // Test Case 1
    registerUser("alice", "Password123!");

    // Test Case 2
    registerUser("bob", "my$ecr3tP@ssw0rd");

    // Test Case 3
    registerUser("charlie", "admin");

    // Test Case 4
    registerUser("dave", "daveslongandsecurepassword");

    // Test Case 5: Empty password test
    registerUser("eve", "");
    
    return 0;
}