#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iomanip>
#include <random>
#include <algorithm>
#include <iterator>

// --- Start of embedded SHA-256 implementation ---
// NOTE: For production code, use a well-vetted cryptographic library like OpenSSL or Crypto++.
// This implementation is for demonstration purposes in a single file.
// Based on public domain implementations.

class SHA256 {
public:
    SHA256();
    void update(const uint8_t *data, size_t length);
    void update(const std::string &data);
    uint8_t* digest();

    static std::string toString(const uint8_t *digest);

private:
    uint8_t  m_data[64];
    uint32_t m_blocklen;
    uint64_t m_bitlen;
    uint32_t m_state[8]; // A-H

    static const uint32_t k[64];

    void transform();
    void pad();
    void revert(uint8_t *hash);

    uint32_t rotr(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }
    uint32_t choose(uint32_t e, uint32_t f, uint32_t g) { return (e & f) ^ (~e & g); }
    uint32_t majority(uint32_t a, uint32_t b, uint32_t c) { return (a & b) ^ (a & c) ^ (b & c); }
    uint32_t sig0(uint32_t x) { return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22); }
    uint32_t sig1(uint32_t x) { return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25); }
};

const uint32_t SHA256::k[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

SHA256::SHA256() : m_blocklen(0), m_bitlen(0) {
    m_state[0] = 0x6a09e667; m_state[1] = 0xbb67ae85; m_state[2] = 0x3c6ef372; m_state[3] = 0xa54ff53a;
    m_state[4] = 0x510e527f; m_state[5] = 0x9b05688c; m_state[6] = 0x1f83d9ab; m_state[7] = 0x5be0cd19;
}

void SHA256::update(const uint8_t *data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        m_data[m_blocklen++] = data[i];
        if (m_blocklen == 64) {
            transform();
            m_bitlen += 512;
            m_blocklen = 0;
        }
    }
}

void SHA256::update(const std::string &data) {
    update(reinterpret_cast<const uint8_t*>(data.c_str()), data.length());
}

void SHA256::transform() {
    uint32_t w[64];
    uint32_t a, b, c, d, e, f, g, h;
    uint32_t t1, t2;

    for (int i=0; i<16; ++i) {
        w[i] = (m_data[i*4] << 24) | (m_data[i*4+1] << 16) | (m_data[i*4+2] << 8) | m_data[i*4+3];
    }
    for (int i=16; i<64; ++i) {
        w[i] = rotr(w[i-2], 17) ^ rotr(w[i-2], 19) ^ (w[i-2] >> 10)
               + w[i-7]
               + rotr(w[i-15], 7) ^ rotr(w[i-15], 18) ^ (w[i-15] >> 3)
               + w[i-16];
    }

    a = m_state[0]; b = m_state[1]; c = m_state[2]; d = m_state[3];
    e = m_state[4]; f = m_state[5]; g = m_state[6]; h = m_state[7];
    
    for (int i=0; i<64; ++i) {
        t1 = h + sig1(e) + choose(e,f,g) + k[i] + w[i];
        t2 = sig0(a) + majority(a,b,c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }

    m_state[0] += a; m_state[1] += b; m_state[2] += c; m_state[3] += d;
    m_state[4] += e; m_state[5] += f; m_state[6] += g; m_state[7] += h;
}

void SHA256::pad() {
    uint64_t i = m_blocklen;
    uint8_t end = (m_blocklen < 56) ? (56 - m_blocklen) : (120 - m_blocklen);
    m_data[i++] = 0x80;
    while(i < 64) m_data[i++] = 0x00;
    if (m_blocklen >= 56) {
        transform();
        for(i=0; i<56; ++i) m_data[i] = 0x00;
    }
    m_bitlen += m_blocklen * 8;
    m_data[63] = m_bitlen;
    m_data[62] = m_bitlen >> 8; m_data[61] = m_bitlen >> 16; m_data[60] = m_bitlen >> 24;
    m_data[59] = m_bitlen >> 32; m_data[58] = m_bitlen >> 40; m_data[57] = m_bitlen >> 48; m_data[56] = m_bitlen >> 56;
    transform();
}

void SHA256::revert(uint8_t *hash) {
    for(int i=0; i<4; ++i) {
        for(int j=0; j<8; ++j) {
            hash[i + j*4] = (m_state[j] >> (24 - i*8)) & 0xff;
        }
    }
}

uint8_t* SHA256::digest() {
    static uint8_t hash[32];
    pad();
    revert(hash);
    return hash;
}

std::string SHA256::toString(const uint8_t *digest) {
    std::stringstream s;
    s << std::setfill('0') << std::hex;
    for (uint32_t i=0; i<32; ++i) {
        s << std::setw(2) << static_cast<unsigned>(digest[i]);
    }
    return s.str();
}
// --- End of embedded SHA-256 implementation ---

// Structure to hold user's salt and hashed password
struct UserData {
    std::string salt;
    std::string hashedPassword;
};

class Lse98 {
private:
    // Using a map to simulate a user database
    std::map<std::string, UserData> userDatabase;

    // Generates a random salt.
    std::string generateSalt(size_t length = 16) {
        std::string salt;
        salt.reserve(length);
        std::random_device rd;
        std::mt19937 generator(rd());
        std::uniform_int_distribution<> distribution(0, 255);

        for (size_t i = 0; i < length; ++i) {
            salt += static_cast<char>(distribution(generator));
        }
        return salt;
    }

    // Hashes a password with a given salt.
    std::string hashPassword(const std::string& password, const std::string& salt) {
        SHA256 sha;
        sha.update(salt + password);
        uint8_t* digest = sha.digest();
        return SHA256::toString(digest);
    }
    
    // Helper to convert raw string to hex for printing
    std::string toHex(const std::string& s) {
        std::ostringstream ret;
        for (char c : s) {
            ret << std::hex << std::setfill('0') << std::setw(2) << (int)(unsigned char)c;
        }
        return ret.str();
    }

public:
    // Registers a new user.
    bool registerUser(const std::string& username, const std::string& password) {
        if (username.empty() || password.empty()) {
            std::cout << "Registration failed: Username and password cannot be empty." << std::endl;
            return false;
        }

        if (userDatabase.count(username)) {
            std::cout << "Registration failed: Username '" << username << "' already exists." << std::endl;
            return false;
        }

        std::string salt = generateSalt();
        std::string hashedPassword = hashPassword(password, salt);

        // In a real app, this would be a parameterized INSERT query.
        userDatabase[username] = {salt, hashedPassword};

        std::cout << "User '" << username << "' registered successfully." << std::endl;
        return true;
    }

    // Verifies a user's password.
    bool verifyPassword(const std::string& username, const std::string& password) {
        auto it = userDatabase.find(username);
        if (it == userDatabase.end()) {
            return false; // User not found
        }
        
        const UserData& userData = it->second;
        std::string providedHash = hashPassword(password, userData.salt);
        
        return providedHash == userData.hashedPassword;
    }
    
    void printDatabase() const {
        std::cout << "\n--- Simulated Database Content ---" << std::endl;
        for(const auto& pair : userDatabase) {
            std::cout << "Username: " << pair.first << std::endl;
            std::cout << "  Salt: " << toHex(pair.second.salt) << std::endl;
            std::cout << "  Hashed Password: " << pair.second.hashedPassword << std::endl;
        }
    }
};

int main() {
    std::cout << "--- Running C++ Registration Test Cases ---" << std::endl;
    Lse98 registry;

    // Test Case 1: Successful registration
    registry.registerUser("alice", "password123");

    // Test Case 2: Successful registration
    registry.registerUser("bob", "secure_Pa$$w0rd");

    // Test Case 3: Attempt to register a duplicate username
    registry.registerUser("alice", "another_password");

    // Test Case 4: Attempt to register with an empty password
    registry.registerUser("charlie", "");

    // Test Case 5: Another successful registration
    registry.registerUser("david", "d@v1d_r0ck$");

    std::cout << "\n--- Verifying Users ---" << std::endl;
    std::cout << "Login for 'alice' with correct password: " << (registry.verifyPassword("alice", "password123") ? "Success" : "Failed") << std::endl;
    std::cout << "Login for 'alice' with incorrect password: " << (registry.verifyPassword("alice", "wrongpassword") ? "Success" : "Failed") << std::endl;
    std::cout << "Login for 'bob' with correct password: " << (registry.verifyPassword("bob", "secure_Pa$$w0rd") ? "Success" : "Failed") << std::endl;
    std::cout << "Login for non-existent user 'eve': " << (registry.verifyPassword("eve", "any_password") ? "Success" : "Failed") << std::endl;
    
    registry.printDatabase();

    return 0;
}