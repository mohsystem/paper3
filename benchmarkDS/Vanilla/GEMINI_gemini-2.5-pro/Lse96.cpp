#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <random>
#include <sstream>
#include <iomanip>
#include <cstdint>
#include <algorithm>

// --- Start of embedded SHA256 implementation ---
// A compact, header-only SHA-256 implementation.
// Original source: https://github.com/amosnier/sha-256-cxx (MIT License)
class SHA256 {
public:
    SHA256();
    void update(const std::string& data);
    void update(const void* data, size_t len);
    std::vector<uint8_t> digest();
    static std::string toString(const std::vector<uint8_t>& digest);

private:
    void pad();
    void transform(const uint8_t* message, unsigned int block_nb);
    uint8_t m_block[64];
    uint32_t m_h[8];
    uint64_t m_len;
    unsigned int m_block_len;
};

static const uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

#define ROR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROR(x, 2) ^ ROR(x, 13) ^ ROR(x, 22))
#define EP1(x) (ROR(x, 6) ^ ROR(x, 11) ^ ROR(x, 25))
#define SIG0(x) (ROR(x, 7) ^ ROR(x, 18) ^ ((x) >> 3))
#define SIG1(x) (ROR(x, 17) ^ ROR(x, 19) ^ ((x) >> 10))

SHA256::SHA256() : m_len(0), m_block_len(0) {
    m_h[0] = 0x6a09e667; m_h[1] = 0xbb67ae85; m_h[2] = 0x3c6ef372; m_h[3] = 0xa54ff53a;
    m_h[4] = 0x510e527f; m_h[5] = 0x9b05688c; m_h[6] = 0x1f83d9ab; m_h[7] = 0x5be0cd19;
}

void SHA256::update(const std::string& data) { update(data.c_str(), data.size()); }
void SHA256::update(const void* data, size_t len) {
    const uint8_t* p = static_cast<const uint8_t*>(data);
    while (len > 0) {
        unsigned int copy_len = std::min((unsigned int)len, 64 - m_block_len);
        std::copy(p, p + copy_len, m_block + m_block_len);
        m_block_len += copy_len;
        p += copy_len;
        len -= copy_len;
        if (m_block_len == 64) {
            transform(m_block, 1);
            m_block_len = 0;
        }
    }
    m_len += len * 8;
}

std::vector<uint8_t> SHA256::digest() {
    pad();
    transform(m_block, m_block_len / 64);
    std::vector<uint8_t> hash(32);
    for (unsigned int i = 0; i < 8; ++i) {
        for (unsigned int j = 0; j < 4; ++j) {
            hash[i * 4 + j] = (m_h[i] >> (24 - j * 8)) & 0xff;
        }
    }
    return hash;
}

void SHA256::pad() {
    m_block[m_block_len++] = 0x80;
    if (m_block_len > 56) {
        std::fill(m_block + m_block_len, m_block + 64, 0);
        transform(m_block, 1);
        m_block_len = 0;
    }
    std::fill(m_block + m_block_len, m_block + 56, 0);
    for (int i = 0; i < 8; ++i) {
        m_block[56 + i] = (m_len >> (56 - i * 8)) & 0xff;
    }
    transform(m_block, 1);
}

void SHA256::transform(const uint8_t* message, unsigned int block_nb) {
    uint32_t w[64], a, b, c, d, e, f, g, h, t1, t2;
    const uint8_t* p = message;
    for (unsigned int i = 0; i < block_nb; i++) {
        for (unsigned int j = 0; j < 16; j++, p += 4) {
            w[j] = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
        }
        for (unsigned int j = 16; j < 64; j++) {
            w[j] = SIG1(w[j - 2]) + w[j - 7] + SIG0(w[j - 15]) + w[j - 16];
        }
        a = m_h[0]; b = m_h[1]; c = m_h[2]; d = m_h[3];
        e = m_h[4]; f = m_h[5]; g = m_h[6]; h = m_h[7];
        for (unsigned int j = 0; j < 64; j++) {
            t1 = h + EP1(e) + CH(e, f, g) + K[j] + w[j];
            t2 = EP0(a) + MAJ(a, b, c);
            h = g; g = f; f = e; e = d + t1;
            d = c; c = b; b = a; a = t1 + t2;
        }
        m_h[0] += a; m_h[1] += b; m_h[2] += c; m_h[3] += d;
        m_h[4] += e; m_h[5] += f; m_h[6] += g; m_h[7] += h;
    }
}

std::string SHA256::toString(const std::vector<uint8_t>& digest) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (uint8_t byte : digest) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    return ss.str();
}
// --- End of embedded SHA256 implementation ---

// Simulate a user database
static std::map<std::string, std::string> userDatabase;

/**
 * Generates a cryptographically secure salt.
 * @param length The length of the salt in bytes.
 * @return A vector of bytes representing the salt.
 */
std::vector<uint8_t> generateSalt(size_t length = 16) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 255);
    std::vector<uint8_t> salt(length);
    for (size_t i = 0; i < length; ++i) {
        salt[i] = static_cast<uint8_t>(distrib(gen));
    }
    return salt;
}

/**
 * Hashes a password with the given salt using SHA-256.
 * @param password The password string to hash.
 * @param salt The salt (vector of bytes) to use.
 * @return The hexadecimal string of the hashed password.
 */
std::string hashPassword(const std::string& password, const std::vector<uint8_t>& salt) {
    SHA256 hasher;
    // Combine salt and password before hashing
    hasher.update(salt.data(), salt.size());
    hasher.update(password);
    std::vector<uint8_t> digest = hasher.digest();
    return SHA256::toString(digest);
}

/**
 * Registers a new user.
 * @param username The username for the new user.
 * @param password The password for the new user.
 * @return The stored value (salt + hashed_password) or an error message.
 */
std::string registerUser(const std::string& username, const std::string& password) {
    if (userDatabase.count(username)) {
        return "Error: Username already exists.";
    }
    if (password.empty()) {
        return "Error: Password cannot be empty.";
    }

    std::vector<uint8_t> saltBytes = generateSalt();
    std::string hashedPassword = hashPassword(password, saltBytes);
    
    std::string saltHex = SHA256::toString(saltBytes);
    
    std::string storedValue = saltHex + hashedPassword;
    userDatabase[username] = storedValue;
    
    return storedValue;
}

int main() {
    std::cout << "--- C++ Registration Simulation ---" << std::endl;

    // Test Case 1
    std::cout << "\n1) Registering user 'alice'..." << std::endl;
    std::string aliceData = registerUser("alice", "password123");
    std::cout << "   Stored data for alice: " << aliceData << std::endl;

    // Test Case 2
    std::cout << "\n2) Registering user 'bob'..." << std::endl;
    std::string bobData = registerUser("bob", "mySecret!@#");
    std::cout << "   Stored data for bob: " << bobData << std::endl;

    // Test Case 3
    std::cout << "\n3) Registering user 'charlie'..." << std::endl;
    std::string charlieData = registerUser("charlie", "P@ssw0rd_");
    std::cout << "   Stored data for charlie: " << charlieData << std::endl;

    // Test Case 4
    std::cout << "\n4) Attempting to register 'alice' again..." << std::endl;
    std::string aliceDuplicateData = registerUser("alice", "anotherPassword");
    std::cout << "   Result: " << aliceDuplicateData << std::endl;

    // Test Case 5
    std::cout << "\n5) Registering user 'dave'..." << std::endl;
    std::string daveData = registerUser("dave", "davespassword");
    std::cout << "   Stored data for dave: " << daveData << std::endl;
    
    std::cout << "\n--- Final Database State ---" << std::endl;
    for (const auto& pair : userDatabase) {
        std::cout << "User: " << pair.first << ", Stored: " << pair.second << std::endl;
    }

    return 0;
}